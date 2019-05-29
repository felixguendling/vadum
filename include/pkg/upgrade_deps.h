#pragma once

#include <vector>

#include "boost/filesystem/path.hpp"

namespace pkg {

struct dep;

void upgrade_deps(boost::filesystem::path const& repo,
                  boost::filesystem::path const& deps_root);

}  // namespace pkg