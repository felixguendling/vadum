#include "doctest.h"

#include <iostream>

#include "vadum/read_deps.h"

#include "test_dir.h"

namespace fs = boost::filesystem;
using vadum::dep;
using vadum::read_deps;
using vadum::ROOT;

auto const vadum_file_2 = std::string{R"(
[pugixml]
  url=git@git.net/pugi.git
  commit=8153c7e9a52b9a64397737625d845fa7aec9e117
[net]
  url=git@git.net/net.git
  commit=494d1527d81ba286f18f1d0e4a7fefb580e49af4
[lmdb]
  url=git@git.net/lmdb.git
  commit=df5c07807e2a40f6e451497d15095ee405ad3693
[flatbuffers]
  url=git@git.net/fbs.git
  commit=c3d74c78904a08f89d61d2c2378d361999a53566
[conf]
  url=git@git.net/conf.git
  commit=78699a83f7c3979ce49a0bcd7d9179e39d87a8b3
[miniz]
  url=git@git.net/miniz.git
  commit=f5d1cf73092133260fd705b1ff9128cae59a2e27
[geo]
  url=git@git.net/geo.git
  commit=3873d97fdcc44c599a38faf86ad2f903cf5f3896
[hash]
  url=git@git.net/hash.git
  commit=4effd2daabd21054e6818ae1a68dd801bc44e67f
[ctx]
  url=git@git.net/ctx.git
  commit=cd390d60d545752e241ce982ba09ea78ab157ba2
[utl]
  url=git@git.net/utl.git
  commit=36abee117eb8a4fbda51a2000ccded3f7fd24ebe
[osrm-backend]
  url=git@git.net/osrm.git
  commit=bf0acdf920eb06419c233b51e4f333eaf3f0f127
[tar]
  url=git@git.net/tar.git
  commit=7f2916f556a9972059611a0d471aaa465300c20c
[motis]
  url=motis-project/motis
  commit=0.5
)"};

TEST_CASE("load_deps") {
  auto const check_deps = std::set<dep>{
      {dep{"./deps", "git@git.net/pugi.git",
           "8153c7e9a52b9a64397737625d845fa7aec9e117"},
       dep{"./deps", "git@git.net/net.git",
           "494d1527d81ba286f18f1d0e4a7fefb580e49af4"},
       dep{"./deps", "git@git.net/lmdb.git",
           "df5c07807e2a40f6e451497d15095ee405ad3693"},
       dep{"./deps", "git@git.net/fbs.git",
           "c3d74c78904a08f89d61d2c2378d361999a53566"},
       dep{"./deps", "git@git.net/conf.git",
           "78699a83f7c3979ce49a0bcd7d9179e39d87a8b3"},
       dep{"./deps", "git@git.net/miniz.git",
           "f5d1cf73092133260fd705b1ff9128cae59a2e27"},
       dep{"./deps", "git@git.net/geo.git",
           "3873d97fdcc44c599a38faf86ad2f903cf5f3896"},
       dep{"./deps", "git@git.net/hash.git",
           "4effd2daabd21054e6818ae1a68dd801bc44e67f"},
       dep{"./deps", "git@git.net/ctx.git",
           "cd390d60d545752e241ce982ba09ea78ab157ba2"},
       dep{"./deps", "git@git.net/utl.git",
           "36abee117eb8a4fbda51a2000ccded3f7fd24ebe"},
       dep{"./deps", "git@git.net/osrm.git",
           "bf0acdf920eb06419c233b51e4f333eaf3f0f127"},
       dep{"./deps", "git@git.net/tar.git",
           "7f2916f556a9972059611a0d471aaa465300c20c"},
       dep{"./deps", "git@github.com:motis-project/motis.git", "0.5"}}};
  auto const read = read_deps("./deps", vadum_file_2);
  CHECK(check_deps == read);
}

TEST_CASE("load_deps_rec") {
  boost::filesystem::current_path(fs::path{TEST_EXECUTION_DIR} / "test" /
                                  "test_repo");

  auto const check_deps =
      std::set<dep>{{dep{"./deps", "git@git.net/pugi.git",
                         "8153c7e9a52b9a64397737625d845fa7aec9e117"},
                     dep{"./deps", "git@git.net/net.git",
                         "494d1527d81ba286f18f1d0e4a7fefb580e49af4"}}};

  auto const root = dep::root(".");
  auto const read = read_deps("deps", &root, true);
  CHECK(check_deps == read);
}
