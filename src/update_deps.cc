#include "pkg/update_deps.h"

#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>

#include "fmt/ostream.h"

#include "pkg/dependency_loader.h"
#include "pkg/git.h"
#include "pkg/name_from_url.h"
#include "pkg/read_deps.h"

namespace fs = boost::filesystem;

namespace pkg {

std::string update_revs(fs::path const& deps_root, dep* d,
                        std::map<std::string, std::string> const& new_revs,
                        bool do_commit = true) {
  {
    for (auto& s : d->succs_) {
      if (auto const it = new_revs.find(name_from_url(s->url_));
          it != end(new_revs)) {
        s->commit_ = it->second;
      }
    }
    d->write_pkg_file();
  }

  std::string new_rev = "";
  if (do_commit) {
    fmt::print(std::cout, "  commiting changes: {}\n", deps_root / d->name());
    auto const repo_path = (deps_root / d->name()).string();
    new_rev = commit(repo_path, ".pkg: update dependencies .pkg file");
    push(repo_path);  // XXX directly and unconditionally push here ???
  }
  return new_rev;
}

void update_deps(fs::path const& repo, fs::path const& deps_root) {
  dependency_loader l{deps_root};
  l.retrieve(repo);
  auto const sorted = l.sorted();

  std::map<std::string, std::string> new_revs;
  std::queue<dep*> q;
  std::set<dep*> initial;
  for (auto const& dep : sorted) {
    if (auto const new_rev = get_commit((deps_root / dep->name()).string());
        new_rev != dep->commit_) {

      fmt::print(std::cout, "  commit changed: {}@{}\n", dep->name(), new_rev);
      new_revs[dep->name()] = new_rev;
      q.emplace(dep);
      initial.emplace(dep);
    }
  }

  std::set<dep*> visited;
  std::set<dep*> referenced;
  while (!q.empty()) {
    auto const next = q.front();
    q.pop();
    visited.emplace(next);
    for (auto const& pred : next->preds_) {
      q.emplace(pred);
      referenced.emplace(pred);
    }
  }

  for (auto const& s : sorted) {
    if (visited.find(s) != end(visited) &&
        (initial.find(s) == end(initial) ||
         referenced.find(s) != end(referenced))) {
      fmt::print(std::cout, "update refs of {}\n", s->name());
      new_revs[s->name()] = update_revs(deps_root, s, new_revs);
    }
  }

  fmt::print(std::cout, "update refs of {}\n", l.root()->name());
  update_revs(repo, l.root(), new_revs, false);
}

}  // namespace pkg