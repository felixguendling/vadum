#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "boost/filesystem.hpp"

#include "utl/parser/util.h"

namespace pkg {

constexpr auto const ROOT = ".";

struct dependency_loader {
  struct dep;

  explicit dependency_loader(boost::filesystem::path deps_root);
  ~dependency_loader();

  void retrieve(boost::filesystem::path const& p);
  std::vector<dep*> sorted();

  std::vector<std::pair<std::string, std::string>> read_deps(dep const* d);
  void retrieve(dep* pred);

  boost::filesystem::path deps_root_;
  std::map<std::string, dep*> deps_;
  std::vector<std::unique_ptr<dep>> dep_mem_;
};

std::string name(dependency_loader::dep const*);

}  // namespace pkg