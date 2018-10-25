#include "pkg/clear_and_load_deps.h"

#include <fstream>
#include <iostream>

#include "boost/filesystem.hpp"

#include "pkg/dependency_loader.h"
#include "pkg/git/git_clone.h"

namespace fs = boost::filesystem;

namespace pkg {

void clear_and_load_deps(fs::path const& repo, fs::path const& deps_root) {
  fs::remove_all(deps_root);
  fs::create_directories(deps_root);

  dependency_loader l{deps_root};
  l.retrieve(repo, [](std::string const& url, std::string const& path,
                      std::string const& ref) {
    std::cout << "git clone " << url << "::" << ref << "\n";
    git_clone(url, ref, path);
  });

  std::ofstream of{"deps/CMakeLists.txt"};
  of << "project(" + deps_root.string() << ")\n"
     << "cmake_minimum_required(VERSION 3.11)\n\n";
  for (auto const& v : l.sorted()) {
    of << "add_subdirectory(" << v->name() << " EXCLUDE_FROM_ALL)\n";
  }
}

}  // namespace pkg