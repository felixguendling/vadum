#pragma once

#include <map>
#include <vector>

#include "boost/filesystem/path.hpp"

#include "vadum/dep.h"

namespace vadum {

struct status {
  bool unchanged() const {
    return !commited_change_ && !uncommited_change_ && !recursive_change_;
  }
  bool commited_change_{false}, uncommited_change_{false},
      recursive_change_{false};
  std::string new_commit_;
};

std::map<dep*, status> get_status(std::vector<dep*> const&);

}  // namespace vadum
