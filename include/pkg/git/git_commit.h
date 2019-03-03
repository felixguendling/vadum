#pragma once

#include <string>

#include "boost/filesystem/path.hpp"

namespace pkg {

std::string commit(boost::filesystem::path const& path, std::string const& msg);

}  // namespace pkg