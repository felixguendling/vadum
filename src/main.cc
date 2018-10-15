#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#include "boost/filesystem.hpp"

#include "utl/logging.h"
#include "utl/parser/util.h"

#include "pkg/git_clone.h"

namespace fs = boost::filesystem;
using namespace pkg;

struct dep {
  std::string name() const {
    auto const slash_pos = url_.find_last_of('/');
    auto const dot_pos = url_.find_last_of('.');
    verify(slash_pos != std::string::npos, "no slash in url");
    verify(dot_pos != std::string::npos, "no dot in url");
    verify(slash_pos < dot_pos, "slash and to in wrong order");
    return url_.substr(slash_pos + 1, dot_pos - slash_pos - 1);
  }

  friend bool operator<(dep const& a, dep const& b) { return a.url_ < b.url_; }

  friend bool operator==(dep const& a, dep const& b) {
    return a.url_ == b.url_;
  }

  std::string url_, ref_;
  std::string required_by_;
};

std::vector<dep> read_deps(fs::path const& p) {
  if (!fs::is_regular_file(p)) {
    return {};
  }

  std::vector<dep> deps;

  std::ifstream f{p.string().c_str()};
  f.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  std::string line, base, path, ref;
  while (!f.eof() && f.peek() != EOF && std::getline(f, line)) {
    if (line.empty()) {
      continue;
    }
    if (line[0] == '*') {
      base = line.substr(1);
    } else {
      std::stringstream ss{line};
      ss >> path >> ref;
      deps.push_back(
          dep{base.empty() ? path : base + ":" + path, ref, p.string()});
    }
  }

  return deps;
}

std::vector<dep> read_deps() {
  auto const curr_dir = fs::current_path();
  for (auto const& p : {curr_dir / ".pkg", curr_dir.parent_path() / ".pkg"}) {
    if (auto const deps = read_deps(p); !deps.empty()) {
      return deps;
    }
  }
  std::cout << "no dependencies found\n";
  return {};
}

void clone_dep_recursive(std::set<dep>& visited, fs::path const& deps_root,
                         dep const& d) {
  auto const path = deps_root / d.name();
  uLOG(utl::info) << "cloning " << d.url_ << " [" << d.ref_ << "]"
                  << " to " << path;
  git_clone(d.url_, path.generic_string(), d.ref_);
  for (auto const& d : read_deps(path / ".pkg")) {
    if (auto const it = visited.find(d); it == end(visited)) {
      visited.insert(d);
      clone_dep_recursive(visited, deps_root, d);
    } else if (it->ref_ != d.ref_) {
      std::cout << "dependency conflict:\n"
                << "  " << it->required_by_ << " requires\n    " << it->ref_
                << "\n"
                << "  " << d.required_by_ << " requires\n    " << d.ref_
                << "\n";
      throw std::runtime_error("non-matching references");
    }
  }
}

int main(int argc, char** argv) {
  auto const deps_root = fs::path("deps");
  fs::remove_all(deps_root);
  fs::create_directories(deps_root);
  std::set<dep> visited;
  for (auto const& d : read_deps()) {
    if (visited.insert(d).second) {
      clone_dep_recursive(visited, deps_root, d);
    }
  }

  std::ofstream of{"deps/CMakeLists.txt"};
  of << "project(" + deps_root.string() << ")\n"
     << "cmake_minimum_required(VERSION 3.11)\n\n";
  for (auto const& v : visited) {
    of << "add_subdirectory(" << v.name() << " EXCLUDE_FROM_ALL)\n";
  }
}