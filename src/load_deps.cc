#include "pkg/load_deps.h"

#include <fstream>
#include <iostream>
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

void load_deps(fs::path const& repo, fs::path const& deps_root) {
  if (!boost::filesystem::is_directory(deps_root)) {
    boost::filesystem::create_directories(deps_root);
  }

  boost::asio::io_service ios;
  boost::asio::io_service::strand main{ios};

  dependency_loader l{deps_root};
  main.post([&] {
    l.retrieve_async(repo, [&](dep* d, dependency_loader::iteration_fn_t cb) {
      if (fs::is_directory(d->path_)) {
        fmt::print("already cloned: {}\n", d->name());

        auto const commit = get_commit(d->path_);
        if (d->commit_ != commit) {
          fmt::print("[{}] warning:\n  required={}\n  current={}\n", d->name(),
                     d->commit_, commit);
        }
        git_attach(d);
        return cb(d);
      } else {
        fmt::print("cloning: {}\n", d->name());
        std::cout << std::flush;
        ios.post([d_copy = *d, d, &main, cb_mv = std::move(cb)] {
          git_clone(&d_copy);
          return main.post([cb_mv_1 = std::move(cb_mv), d] { cb_mv_1(d); });
        });
      }
    });
  });

  std::vector<std::thread> worker(10);
  std::for_each(begin(worker), end(worker),
                [&](auto& w) { w = std::thread{[&] { ios.run(); }}; });
  std::for_each(begin(worker), end(worker), [&](auto& w) { w.join(); });

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