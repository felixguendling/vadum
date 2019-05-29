#include "pkg/git.h"

#include <algorithm>

#include "boost/filesystem.hpp"

#include "utl/erase.h"
#include "utl/to_vec.h"

#include "pkg/exec.h"

namespace pkg {

void git_clone(dep const* d) {
  if (!boost::filesystem::is_directory(d->path_.parent_path())) {
    boost::filesystem::create_directories(d->path_.parent_path());
  }

  exec(d->path_.parent_path(), "git clone {} {}", d->url_,
       boost::filesystem::absolute(d->path_).string());
  exec(d->path_, "git checkout {}", d->commit_);
  exec(d->path_, "git submodule update --init --recursive");
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