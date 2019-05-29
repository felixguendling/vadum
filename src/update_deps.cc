#include "pkg/update_deps.h"

#include <algorithm>

#include "fmt/format.h"

#include "pkg/dependency_loader.h"
#include "pkg/git.h"
#include "pkg/status.h"

namespace fs = boost::filesystem;

namespace pkg {

void update_deps(std::vector<dep*> const& sorted, bool const push_changes) {
  auto const dep_status = get_status(sorted);
  if (std::none_of(begin(dep_status), end(dep_status),
                   [](auto&& s) { return s.second.commited_change_; })) {
    fmt::print("nothing to do\n");
    return;
  }

  for (auto const& d : sorted) {
    auto const& s = dep_status.at(d);
    if (s.unchanged()) {
      continue;
    }

    if (s.commited_change_) {
      fmt::print("{}: new commit {}\n", d->name(), s.new_commit_);
      d->commit_ = s.new_commit_;
    }

    if (s.recursive_change_) {
      fmt::print("{}: updating dependencies\n", d->name());
      d->write_pkg_file();
      d->commit_ = commit(d->path_, "update dependencies");
      if (push_changes) {
        push(d->path_);
      }
    }
  }
}

void update_deps(fs::path const& repo, fs::path const& deps_root) {
  dependency_loader l{deps_root};
  l.retrieve(repo);
  update_deps(l.sorted(), false);
}

}  // namespace pkg