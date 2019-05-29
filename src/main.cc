#include <cstdio>
#include <fstream>
#include <iostream>

#include "pkg/dependency_loader.h"
#include "pkg/detect_branch.h"
#include "pkg/load_deps.h"
#include "pkg/print_status.h"
#include "pkg/resolve_conflicts.h"
#include "pkg/update_deps.h"
#include "pkg/upgrade_deps.h"

namespace fs = boost::filesystem;
using namespace pkg;

int main(int argc, char** argv) {
  if (argc != 2) {
    printf(
        "usage: %s\n"
        "  -l    load    [clone dependencies]\n"
        "  -u    update  [recursive on new commit]\n"
        "  -s    status  [print status]\n"
        "  -r    resolve [resolve conflicts]\n"
        "  -g    upgrade [switch to branch head]\n",
        argv[0]);
    return 0;
  } else if (std::strcmp(argv[1], "-u") == 0) {
    update_deps(fs::path{"."}, fs::path("deps"));
  } else if (std::strcmp(argv[1], "-l") == 0) {
    load_deps(fs::path{"."}, fs::path("deps"));
  } else if (std::strcmp(argv[1], "-s") == 0) {
    print_status(fs::path{"."}, fs::path("deps"));
  } else if (std::strcmp(argv[1], "-r") == 0) {
    resolve_conflicts(fs::path{"."}, fs::path("deps"));
  } else if (std::strcmp(argv[1], "-g") == 0) {
    upgrade_deps(fs::path{"."}, fs::path("deps"));
  }
}