#pragma once

#include <set>
#include <string>

#include "utl/struct/comparable.h"

#include "boost/filesystem/path.hpp"

#include "pkg/dep.h"

namespace pkg {

struct commit_info {
  MAKE_COMPARABLE()
  std::string info_;
  dep::branch_commit bc_;
};

void git_clone(dep const*);
void git_clone_clean(dep const*);

void git_attach(dep const*);

std::string get_commit(boost::filesystem::path const& p,
                       std::string const& target = "HEAD");

std::string commit(boost::filesystem::path const& p, std::string const& msg);

void push(boost::filesystem::path const& p);

std::vector<commit_info> get_commit_infos(
    boost::filesystem::path const& p,
    std::set<dep::branch_commit> const& commits);

bool is_fast_forward(boost::filesystem::path const& p,
                     std::string const& commit1, std::string const& commit2);

}  // namespace pkg
