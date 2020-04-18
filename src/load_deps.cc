#include "pkg/load_deps.h"

#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/asio/strand.hpp"
#include "boost/filesystem.hpp"

#include "fmt/format.h"

#include "pkg/dependency_loader.h"
#include "pkg/detect_branch.h"
#include "pkg/exec.h"
#include "pkg/git.h"

namespace fs = boost::filesystem;

namespace pkg {

void load_deps(fs::path const& repo, fs::path const& deps_root,
               bool const clone_https, bool const force) {
  if (!boost::filesystem::is_directory(deps_root)) {
    boost::filesystem::create_directories(deps_root);
  }

  dependency_loader l{deps_root};
  l.retrieve(repo, [&](dep* d, branch_commit const& bc) {
    if (fs::is_directory(d->path_)) {
      auto const current_commit = get_commit(d->path_);
      if (bc.commit_ == current_commit) {
        return;
      }

      executor e;
      try {
        fmt::print("{} already cloned: current={}, previous={}, required={}",
                   d->name(), git_shorten(d, current_commit),
                   git_shorten(d, d->commit_), git_shorten(d, bc.commit_));
        std::cout << std::flush;

        if (!commit_exists(d, d->commit_) || !commit_exists(d, bc.commit_)) {
          fmt::print(" ... fetch.");
          std::cout << std::flush;
          e.exec(d->path_, "git fetch origin");
        }

        if (commit_time(d, d->commit_) < commit_time(d, bc.commit_)) {
          d->commit_ = bc.commit_;
          d->branch_ = bc.branch_;
        } else {
          fmt::print(" ... required > previous.");
          std::cout << std::flush;
        }

        if (current_commit != d->commit_) {
          fmt::print(" ... checkout {}.", git_shorten(d, d->commit_));
          std::cout << std::flush;
          git_attach(e, d, force);
        }

        std::cout << std::endl;
      } catch (std::exception const& ex) {
        fmt::print("Checkout failed for {}: {}\n", d->name(), ex.what());
        e.print_trace();
      }
    } else {
      fmt::print("cloning: {}\n", d->name());
      std::cout << std::flush;

      std::function<void(int)> clone = [&](int i) {
        executor e;
        try {
          git_clone(e, d, clone_https);
        } catch (std::exception const& ex) {
          fmt::print("Repo checkout failed for {}: {}\n", d->name(), ex.what());
          e.print_trace();

          std::cout << "Retry clone\n";
          if (i != 0) {
            clone(i - 1);
          }
        }
      };

      clone(2);
    }
  });

  std::ofstream of{"deps/CMakeLists.txt"};
  of << "project(" + deps_root.string() << ")\n"
     << "cmake_minimum_required(VERSION 3.10)\n\n";
  for (auto const& v : l.sorted()) {
    if (v->url_ == ROOT) {
      continue;
    }
    of << "add_subdirectory(" << v->name() << " EXCLUDE_FROM_ALL)\n";
  }
}

}  // namespace pkg