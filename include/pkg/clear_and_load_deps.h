#pragma once

#include "boost/filesystem/path.hpp"

namespace pkg {

void clear_and_load_deps(boost::filesystem::path const& repo,
                         boost::filesystem::path const& deps_root);

}  // namespace pkg