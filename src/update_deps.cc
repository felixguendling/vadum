#include "pkg/update_deps.h"

#include <algorithm>

#include "fmt/format.h"

#include "pkg/dependency_loader.h"
#include "pkg/git.h"
#include "pkg/status.h"

namespace fs = boost::filesystem;

namespace pkg {

void update_deps(fs::path const& repo, fs::path const& deps_root) {
  dependency_loader l{deps_root};
  l.retrieve(repo);

  auto const dep_status = get_status(l.get_all());
  if (std::none_of(begin(dep_status), end(dep_status),
                   [](auto&& s) { return s.second.commited_change_; })) {
    fmt::print("nothing to do\n");
    return;
  }

  for (auto const& d : l.sorted()) {
    auto const& s = dep_status.at(d);
    if (s.unchanged()) {
      continue;
    }

    if (s.commited_change_) {
      d->commit_ = s.new_commit_;
    }

    if (s.recursive_change_) {
      d->write_pkg_file();
    }

    d->commit_ = commit(d->path_, "update dependencies");
  }
}

}  // namespace pkg