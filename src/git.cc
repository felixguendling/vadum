#include "pkg/git.h"

#include <algorithm>
#include <iostream>

#include "boost/filesystem.hpp"

#include "utl/erase.h"
#include "utl/to_vec.h"

#include "pkg/exec.h"

namespace pkg {

std::string git_shorten(dep const* d, std::string const& commit) {
  auto out = exec(d->path_, "git rev-parse --short {}", commit).out_;
  utl::erase(out, '\n');
  return out;
}

std::string get_commit(executor& e, boost::filesystem::path const& p,
                       std::string const& target) {
  auto out = exec(p, "git rev-parse {}", target).out_;
  utl::erase(out, '\n');
  return out;
}

void git_attach(executor& e, dep const* d) {
  e.exec(d->path_, "git fetch origin '*:*'");
  auto const branch_head_commit =
      get_commit(e, d->path_, "origin/" + d->branch_);
  if (branch_head_commit == d->commit_) {
    e.exec(d->path_, "git checkout {}", d->branch_);
  } else {
    e.exec(d->path_, "git checkout {}", d->commit_);
  }
}

void git_clone(executor& e, dep const* d) {
  if (!boost::filesystem::is_directory(d->path_.parent_path())) {
    boost::filesystem::create_directories(d->path_.parent_path());
  }

  e.exec(d->path_.parent_path(), "git clone {} {}", d->url_,
         boost::filesystem::absolute(d->path_).string());
  e.exec(d->path_, "git checkout {}", d->commit_);
  e.exec(d->path_, "git submodule update --init --recursive");
}

void git_attach(dep const* d) {
  auto e = executor{};
  git_attach(e, d);
}

std::string get_commit(boost::filesystem::path const& p,
                       std::string const& target) {
  auto e = executor{};
  return get_commit(e, p, target);
}

std::string commit(boost::filesystem::path const& p, std::string const& msg) {
  constexpr auto const PKG_FILE = ".pkg";
  exec(p, "git add {}", PKG_FILE);
  exec(p, "git commit -m \"{}\"", msg);
  return get_commit(p);
}

void push(boost::filesystem::path const& p) { exec(p, "git push"); }

std::vector<commit_info> get_commit_infos(
    boost::filesystem::path const& p,
    std::set<dep::branch_commit> const& commits) {
  auto infos = utl::to_vec(commits, [&](auto&& bc) -> commit_info {
    auto info = exec(p, "git show -s --format=%ci {}", bc.commit_).out_;
    info.resize(info.size() - 1);
    return {info, bc};
  });
  std::sort(begin(infos), end(infos), std::greater<commit_info>());
  return infos;
}

bool is_fast_forward(boost::filesystem::path const& p,
                     std::string const& commit1, std::string const& commit2) {
  return exec(p, "git merge-base --is-ancestor {} {}", commit1, commit2)
             .exit_code_ == 0;
}

}  // namespace pkg