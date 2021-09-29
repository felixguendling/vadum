#pragma once

#include "boost/filesystem/path.hpp"

namespace vadum {

void load_deps(boost::filesystem::path const& repo,
               boost::filesystem::path const& deps_root, bool clone_https,
               bool force, bool recursive);

}  // namespace vadum
