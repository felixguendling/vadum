#include <cstdio>
#include <fstream>
#include <iostream>

#include "pkg/dependency_loader.h"
#include "pkg/detect_branch.h"
#include "pkg/load_deps.h"
#include "pkg/status.h"
#include "pkg/update_deps.h"

namespace fs = boost::filesystem;
using namespace pkg;

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("usage: %s [-u for update] [-l for load] [-s for status]\n",
           argv[0]);
    return 0;
  } else if (std::strcmp(argv[1], "-u") == 0) {
    update_deps(fs::path{"."}, fs::path("deps"));
  } else if (std::strcmp(argv[1], "-l") == 0) {
    load_deps(fs::path{"."}, fs::path("deps"));
  } else if (std::strcmp(argv[1], "-s") == 0) {
    print_status(fs::path{"."}, fs::path("deps"));
  }
}