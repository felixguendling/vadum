#include "pkg/git.h"

#include <algorithm>
#include <iostream>

#include "boost/filesystem.hpp"

#include "utl/erase.h"
#include "utl/to_vec.h"

#include "pkg/exec.h"

namespace pkg {

std::string get_commit(executor& e, boost::filesystem::path const& p,
                       std::string const& target) {
  auto out = exec(p, "git rev-parse {}", target).out_;
  utl::erase(out, '\n');
  return out;
}

void git_attach(executor& e, dep const* d) {
  auto const branch_head_commit = get_commit(e, d->path_, d->branch_);
  if (!d->branch_.empty() && branch_head_commit != d->commit_) {
    e.exec(d->path_, "git checkout {}", d->commit_);
  }
}

void git_clone(executor& e, dep const* d) {
  auto const bare_repo_path = d->bare_repo_path();
  if (boost::filesystem::is_directory(bare_repo_path)) {
    e.exec(bare_repo_path, "git fetch origin '*:*'");
  } else {
    e.exec(d->path_.parent_path(), "git clone --bare {} {}", d->url_,
           bare_repo_path.string());
  }
  e.exec(bare_repo_path, "git worktree prune");
  e.exec(bare_repo_path, "git worktree add -f -f --checkout {} {}",
         boost::filesystem::absolute(d->path_).string(), d->branch_);
  e.exec(d->path_, "git submodule update --init --recursive");
  git_attach(e, d);
}

void git_clone_clean(executor& e, dep const* d) {
  boost::filesystem::remove_all(d->path_);
  boost::filesystem::remove_all(d->bare_repo_path());
  git_clone(e, d);
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