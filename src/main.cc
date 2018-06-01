#include <fstream>
#include <iostream>
#include <sstream>

#include "boost/filesystem.hpp"

#include "utl/logging.h"
#include "utl/parser/util.h"
#include "utl/struct/printable.h"

#include "pkg/git_clone.h"

namespace fs = boost::filesystem;
using namespace pkg;

struct dep {
  MAKE_PRINTABLE(dep);

  std::string name() const {
    auto const slash_pos = url_.find_last_of('/');
    auto const dot_pos = url_.find_last_of('.');
    verify(slash_pos != std::string::npos, "no slash in url");
    verify(dot_pos != std::string::npos, "no dot in url");
    verify(slash_pos < dot_pos, "slash and to in wrong order");
    return url_.substr(slash_pos + 1, dot_pos - slash_pos - 1);
  }

  std::string url_, ref_;
};

std::vector<dep> read_deps() {
  std::vector<dep> deps;

  auto const curr_dir = fs::current_path();
  for (auto const& p : {curr_dir / ".pkg", curr_dir.parent_path() / ".pkg"}) {
    if (!fs::is_regular_file(".pkg")) {
      continue;
    }

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

        deps.push_back(dep{base.empty() ? path : base + ":" + path, ref});
      }
    }

    if (!deps.empty()) {
      break;
    }
  }

  return deps;
}

int main(int argc, char** argv) {
  auto const deps_root = fs::path("deps");
  fs::remove_all(deps_root);
  fs::create_directories(deps_root);
  for (auto const& d : read_deps()) {
    auto const path = deps_root / d.name();
    uLOG(utl::info) << "cloning " << d.url_ << " to " << path;
    git_clone(d.url_, path.generic_string(), d.ref_);
  }
}