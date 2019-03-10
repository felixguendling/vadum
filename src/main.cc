#include <boost/foreach.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <exception>
#include <iostream>
#include <set>
#include <string>

namespace pt = boost::property_tree;

int main(int argc, char** argv) {
  if (argc < 2) {
    return 1;
  }

  auto const filename = argv[1];

  pt::ptree tree;
  pt::read_ini(filename, tree);
  for (auto const& m : tree) {
    std::cout << m.first << " " << m.second << "\n";
  }
}

// #include <fstream>
// #include <iostream>

// #include "pkg/clear_and_load_deps.h"
// #include "pkg/update_deps.h"

// namespace fs = boost::filesystem;
// using namespace pkg;

// int main(int argc, char** argv) {
//   if (argc != 2) {
//     printf("usage: %s [-u for update] [-l for load]\n", argv[0]);
//     return 0;
//   } else if (std::strcmp(argv[1], "-u") == 0) {
//     update_deps(fs::path{"."}, fs::path("deps"));
//   } else if (std::strcmp(argv[1], "-l") == 0) {
//     clear_and_load_deps(fs::path{"."}, fs::path("deps"));
//   }
// }