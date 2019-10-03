#include "pkg/git.h"

#include <algorithm>
#include <iostream>

#include "boost/filesystem.hpp"

#include "utl/erase.h"
#include "utl/to_vec.h"

#include "pkg/exec.h"

namespace pkg {

void git_clone(dep const* d) {
  auto const bare_repo_path = d->bare_repo_path();
  if (!boost::filesystem::is_directory(bare_repo_path)) {
    exec(d->path_.parent_path(), "git clone --bare {} {}", d->url_,
         bare_repo_path.string());
  }
  exec(bare_repo_path, "git worktree prune");
  exec(bare_repo_path, "git worktree add --checkout {} {}",
       boost::filesystem::absolute(d->path_).string(), d->branch_);
  exec(d->path_, "git submodule update --init --recursive");
  git_attach(d);
  std::cout << exec(d->path_, "dir").out_ << "\n";
}

void git_attach(dep const* d) {
  auto const branch_head_commit = get_commit(d->path_, d->branch_);
  if (!d->branch_.empty() && branch_head_commit != d->commit_) {
    exec(d->path_, "git checkout {}", d->commit_);
  }
}

std::string get_commit(boost::filesystem::path const& p,
                       std::string const& target) {
  auto out = exec(p, "git rev-parse {}", target).out_;
  utl::erase(out, '\n');
  return out;
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
             .return_code_ == 0;
}

}  // namespace pkg