#pragma once

#include <string>

namespace pkg {

void git_clone(std::string const& url, std::string const& path,
               std::string const& ref);

}  // namespace pkg