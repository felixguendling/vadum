#pragma once

#include "boost/filesystem/path.hpp"

namespace pkg {

void update_deps(boost::filesystem::path const& repo,
                 boost::filesystem::path const& deps_root);

}  // namespace pkg