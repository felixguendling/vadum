#pragma once

#include <string>

#include "boost/filesystem/path.hpp"

namespace pkg {

std::string get_revision(boost::filesystem::path const& path);

}  // namespace pkg