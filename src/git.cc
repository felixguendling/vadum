#include "pkg/git.h"

#include "boost/filesystem.hpp"

#include "utl/erase.h"

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

}  // namespace pkg