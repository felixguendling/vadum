#pragma once

#include <string>

namespace pkg {

void git_clone(std::string const& url, std::string const& ref,
               std::string const& path);

}  // namespace pkg