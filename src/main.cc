#include <cstdio>
#include <algorithm>

#include "vadum/exec.h"
#include "vadum/load_deps.h"
#include "vadum/print_status.h"

namespace fs = boost::filesystem;
using namespace vadum;

extern bool verbose;

bool has_flag(int argc, char** argv, char const* str) {
  return std::any_of(argv, argv + argc,
                     [&](char* arg) { return std::strcmp(arg, str) == 0; });
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf(
        "Usage:\n"
        "  vadum load | -l      [clone dependencies]\n"
        "      -h for https        (default: ssh);\n"
        "      -f for hard reset   (default: checkout)]\n"
        "      -r for recursive    (default: false)]\n"
        "  vadum status | -s    [print status]\n"
        "\n"
        "Common flags - apply everywhere:\n"
        "  -v        verbose [print what's going on]\n");
    return 0;
  }

  auto const mode = std::string_view{argv[1]};
  set_verbose(has_flag(argc - 1, argv + 1, "-v"));
  if (mode == "load" || mode == "-l") {
    load_deps(fs::path{"."}, fs::path("deps"),  //
              has_flag(argc - 1, argv + 1, "-h"),  //
              has_flag(argc - 1, argv + 1, "-f"),  //
              has_flag(argc - 1, argv + 1, "-r"));
  } else if (mode == "status" || mode == "-s") {
    print_status(fs::path{"."}, fs::path("deps"));
  }
}
