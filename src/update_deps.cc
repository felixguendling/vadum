#include "pkg/update_deps.h"

#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>

#include "pkg/dependency_loader.h"
#include "pkg/git.h"
#include "pkg/read_deps.h"
#include "pkg/write_deps.h"

namespace fs = boost::filesystem;

namespace pkg {

std::string update_revs(fs::path const& deps_root, dep* d,
                        std::map<std::string, std::string> const& new_revs,
                        bool do_commit = true) {
  {
    auto deps = read_deps(deps_root, d);
    for (auto& d : deps) {
      if (auto const it = new_revs.find(name_from_url(d.url_));
          it != end(new_revs)) {
        d.commit_ = it->second;
      }
    }

    std::ofstream of{(deps_root / d->name() / PKG_FILE).string().c_str()};
    write_deps(of, deps);
    of.close();
  }

  std::cout << "  commiting changes in " << (deps_root / d->name()) << "\n";

  std::string new_rev = "";
  if (do_commit) {
    auto const repo_path = (deps_root / d->name()).string();
    new_rev = commit(repo_path, ".pkg: update dependencies .pkg file");
    push(repo_path); // XXX directly and unconditionally push here ???
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
      std::cout << "update refs of " << s->name() << "\n";
      new_revs[s->name()] = update_revs(deps_root, s, new_revs);
    }
  }

  update_revs(repo, dep::root(), new_revs, false);
}

}  // namespace pkg