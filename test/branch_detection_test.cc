#include "doctest.h"

#include "vadum/detect_branch.h"

using vadum::detect_branch;

TEST_CASE("detached single branch") {
  std::string o1 = R"(
* (HEAD detached at cd390d6)
  remotes/origin/boost-1_60
)";
  CHECK(std::vector<std::string>{"boost-1_60"} == detect_branch(o1));
}

TEST_CASE("head single branch") {
  std::string o2 = R"(
* (HEAD detached at 255e7a2)
  net-old
  remotes/origin/HEAD -> origin/net-old
  remotes/origin/net-old
)";
  CHECK(std::vector<std::string>{"net-old"} == detect_branch(o2));
}

TEST_CASE("multiple branches") {
  std::string o3 = R"(
* (HEAD detached at 8efbc8a)
  master
  remotes/origin/HEAD -> origin/master
  remotes/origin/master
  remotes/origin/tiles
)";

  CHECK(std::vector<std::string>{"master", "tiles"} == detect_branch(o3));
}
