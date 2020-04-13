#include "pkg/upgrade_deps.h"

#include "fmt/format.h"

#include "pkg/dependency_loader.h"
#include "pkg/exec.h"
#include "pkg/git.h"

namespace fs = boost::filesystem;

namespace pkg {

void upgrade_deps(fs::path const& repo, fs::path const& deps_root) {
  dependency_loader l{deps_root};
  l.retrieve(repo, [&](dep* d, branch_commit const&) {
    if (d->branch_.empty()) {
      return;
    }

    auto const branch_head_commit =
        get_commit(d->path_, "remotes/origin/" + d->branch_);
    if (branch_head_commit != d->commit_) {
      fmt::print("{}: updating from {} to {}\n", d->name(), d->commit_,
                 d->branch_);
      exec(d->path_, "git fetch");
      exec(d->path_, "git checkout {}", d->branch_);
    }
  });
}

}  // namespace pkg