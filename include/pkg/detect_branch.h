#pragma once

#include <string>
#include <vector>

#include "boost/filesystem/path.hpp"

namespace pkg {

struct dep;

std::vector<std::string> detect_branch(boost::filesystem::path const&);

std::vector<std::string> detect_branch(
    std::string const& git_branch_contains_output);

bool detect_branch(dep*);

}  // namespace pkg