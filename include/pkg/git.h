#pragma once

#include <string>

#include "boost/filesystem/path.hpp"

#include "pkg/dep.h"

namespace pkg {

void git_clone(dep const* d);

std::string get_commit(boost::filesystem::path const& p,
                       std::string const& target = "HEAD");

std::string commit(boost::filesystem::path const& p, std::string const& msg);

void push(boost::filesystem::path const& p);

}  // namespace pkg
