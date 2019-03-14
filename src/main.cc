#include <cstdio>

#include "pkg/load_deps.h"
#include "pkg/update_deps.h"

namespace fs = boost::filesystem;
using namespace pkg;

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("usage: %s [-u for update] [-l for load]\n", argv[0]);
    return 0;
  } else if (std::strcmp(argv[1], "-u") == 0) {
    update_deps(fs::path{"."}, fs::path("deps"));
  } else if (std::strcmp(argv[1], "-l") == 0) {
    load_deps(fs::path{"."}, fs::path("deps"));
  }
}