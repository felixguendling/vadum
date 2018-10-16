#include <fstream>
#include <iostream>

#include "boost/filesystem.hpp"

#include "pkg/dependency_loader.h"

namespace fs = boost::filesystem;
using namespace pkg;

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
    of << "add_subdirectory(" << name(v) << " EXCLUDE_FROM_ALL)\n";
  }
}