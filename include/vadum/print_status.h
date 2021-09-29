#pragma once

#include "boost/filesystem/path.hpp"

namespace vadum {

void print_status(boost::filesystem::path const& repo,
                  boost::filesystem::path const& deps_root);

}  // namespace vadum
