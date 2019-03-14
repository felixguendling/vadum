#include "pkg/load_deps.h"

#include <fstream>
#include <iostream>

#include "boost/filesystem.hpp"

#include "pkg/dependency_loader.h"
#include "pkg/detect_branch.h"
#include "pkg/git.h"

namespace fs = boost::filesystem;

namespace pkg {

void load_deps(fs::path const& repo, fs::path const& deps_root) {
  fs::remove_all(deps_root);
  fs::create_directories(deps_root);

  dependency_loader l{deps_root};
  l.retrieve(repo, [](boost::filesystem::path const& p, dep* d) {
    git_clone(d->url_, d->commit_, p);
    if (d->branch_.empty()) {
      auto const branches = detect_branch(p);
      if (branches.size() == 1) {
        d->branch_ = branches.front();
        std::cout << "Branch of " << d->commit_ << " in " << d->url_
                  << " is unambiguous. Using: " << d->branch_ << "\n";
      } else {
        std::cout << "Branch of " << d->commit_ << " in " << d->url_
                  << " is ambiguous. It is contained in: ";
        for (auto const& b : branches) {
          std::cout << b << " ";
        }
        std::cout << "\n";
      }
    }
  });

  std::ofstream of{"deps/CMakeLists.txt"};
  of << "project(" + deps_root.string() << ")\n"
     << "cmake_minimum_required(VERSION 3.10)\n\n";
  for (auto const& v : l.sorted()) {
    of << "add_subdirectory(" << v->name() << " EXCLUDE_FROM_ALL)\n";
  }
}

}  // namespace pkg