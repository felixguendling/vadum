#pragma once

#include <vector>

#include "boost/filesystem/path.hpp"

namespace pkg {

struct dep;

void update_deps(std::vector<dep*> const& sorted, bool const push);

void update_deps(boost::filesystem::path const& repo,
                 boost::filesystem::path const& deps_root);

}  // namespace pkg