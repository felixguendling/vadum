#pragma once

#include <string>

#include "boost/filesystem/path.hpp"

namespace pkg {

void git_clone(std::string const& url, std::string const& ref,
               boost::filesystem::path const& path);

}  // namespace pkg