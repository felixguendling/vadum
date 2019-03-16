#include "pkg/load_deps.h"

#include <fstream>
#include <ostream>

#include "boost/filesystem.hpp"

#include "fmt/format.h"

#include "pkg/dependency_loader.h"
#include "pkg/detect_branch.h"
#include "pkg/git.h"

namespace fs = boost::filesystem;

namespace pkg {

void load_deps(fs::path const& repo, fs::path const& deps_root) {
  dependency_loader l{deps_root};
  l.retrieve(repo, [](dep* d) {
    if (fs::is_directory(d->path_)) {
      fmt::print("already cloned: {}\n", d->name());
    } else {
      fmt::print("cloning: {}\n", d->name());
      git_clone(d);
    }

    if (!d->branch_.empty() &&
        get_commit(d->path_, "remotes/origin/" + d->branch_) == d->commit_) {
      exec(d->path_, "git checkout {}", d->branch_);
    }
  });

  std::ofstream of{"deps/CMakeLists.txt"};
  of << "project(" + deps_root.string() << ")\n"
     << "cmake_minimum_required(VERSION 3.10)\n\n";
  for (auto const& v : l.sorted()) {
    of << "add_subdirectory(" << v->name() << " EXCLUDE_FROM_ALL)\n";
  }
}

}  // namespace pkg