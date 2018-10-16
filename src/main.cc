#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#include "boost/filesystem.hpp"

#include "utl/erase.h"
#include "utl/get_or_create.h"
#include "utl/logging.h"
#include "utl/parser/util.h"
#include "utl/to_vec.h"

#include "pkg/git_clone.h"

namespace fs = boost::filesystem;
using namespace pkg;

constexpr auto const PKG_FILE = ".pkg";
constexpr auto const ROOT = ".";

struct dependency_loader {
  struct dep {
    dep(std::string url, std::string ref)
        : url_{std::move(url)}, ref_{std::move(ref)} {}

    std::string name() const {
      if (url_ == ROOT) {
        return ROOT;
      }

      auto const slash_pos = url_.find_last_of('/');
      auto const dot_pos = url_.find_last_of('.');
      verify(slash_pos != std::string::npos, "no slash in url");
      verify(dot_pos != std::string::npos, "no dot in url");
      verify(slash_pos < dot_pos, "slash and to in wrong order");
      return url_.substr(slash_pos + 1, dot_pos - slash_pos - 1);
    }

    friend bool operator<(dep const& a, dep const& b) {
      return a.url_ < b.url_;
    }

    friend bool operator==(dep const& a, dep const& b) {
      return a.url_ == b.url_;
    }

    std::string url_, ref_;
    std::set<dep*> preds_;
    std::set<dep*> succs_;
  };

  explicit dependency_loader(fs::path deps_root)
      : deps_root_{std::move(deps_root)} {}

  std::vector<std::pair<std::string, std::string>> read_deps(dep const* d) {
    auto const p = (d->name() == ROOT) ? (fs::path{ROOT} / PKG_FILE)
                                       : (deps_root_ / d->name() / PKG_FILE);

    std::cout << "deps from " << p << "\n";
    if (!fs::is_regular_file(p)) {
      std::cout << "  " << p << " is not a regular file\n";
      return {};
    }

    std::ifstream f{p.string().c_str()};
    f.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::vector<std::pair<std::string, std::string>> deps;
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
        deps.emplace_back(base.empty() ? path : base + ":" + path, ref);
      }
    }

    for (auto const& [url, ref] : deps) {
      std::cout << "  " << url << " " << ref << "\n";
    }

    return deps;
  }

  void retrieve(dep* pred) {
    for (auto const& url_ref : read_deps(pred)) {
      auto const url = url_ref.first;
      auto const ref = url_ref.second;
      auto succ = utl::get_or_create(deps_, url, [&]() {
        auto next =
            dep_mem_.emplace_back(std::make_unique<dep>(url, ref)).get();
        git_clone(next->url_, (deps_root_ / next->name()).string(), next->ref_);
        retrieve(next);
        return next;
      });
      verify(succ->ref_ == ref, "non-matching ref");
      succ->preds_.insert(pred);
      pred->succs_.insert(succ);
    }
  }

  void retrieve(fs::path const& p) {
    auto& d = dep_mem_.emplace_back(std::make_unique<dep>(ROOT, ROOT));
    retrieve(deps_[ROOT] = d.get());
  }

  std::vector<dep*> sorted() {
    auto sorted = std::vector<dep*>{};
    auto all = utl::to_vec(dep_mem_, [](auto&& d) { return d.get(); });
    while (true) {
      auto const dependency_free = std::find_if(
          begin(all), end(all), [](auto&& d) { return d->succs_.empty(); });
      if (dependency_free == end(all)) {
        return sorted;
      }

      std::cout << (*dependency_free)->name() << " has no dependencies\n";

      if ((*dependency_free)->name() != ROOT) {
        sorted.emplace_back(*dependency_free);
      }
      for (auto const& pred : (*dependency_free)->preds_) {
        std::cout << "  removing " << (*dependency_free)->name() << " from "
                  << pred->name() << "\n";
        utl::erase(pred->succs_, (*dependency_free));
      }
      all.erase(dependency_free);
    }
  }

  fs::path deps_root_;
  std::map<std::string, dep*> deps_;
  std::vector<std::unique_ptr<dep>> dep_mem_;
};

int main(int argc, char** argv) {
  auto const deps_root = fs::path("deps");
  fs::remove_all(deps_root);
  fs::create_directories(deps_root);

  dependency_loader l{deps_root};
  l.retrieve(fs::path{"."});

  std::ofstream of{"deps/CMakeLists.txt"};
  of << "project(" + deps_root.string() << ")\n"
     << "cmake_minimum_required(VERSION 3.11)\n\n";
  for (auto const& v : l.sorted()) {
    of << "add_subdirectory(" << v->name() << " EXCLUDE_FROM_ALL)\n";
  }
}