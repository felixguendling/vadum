#include "pkg/update_deps.h"

#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>

#include "pkg/dependency_loader.h"
#include "pkg/git/get_revision.h"
#include "pkg/git/git_commit.h"

namespace fs = boost::filesystem;

namespace pkg {

std::string update_revs(fs::path const& deps_root, dep* d,
                        std::map<std::string, std::string> const& new_revs) {
  {
    auto const deps = read_deps(deps_root, d);
    std::ofstream of{(deps_root / d->name() / PKG_FILE).string().c_str()};
    for (auto const& [succ_url, succ_ref] : deps) {
      of << succ_url << " ";
      if (auto const it = new_revs.find(name_from_url(succ_url));
          it != end(new_revs)) {
        of << it->second;
      } else {
        of << succ_ref;
      }
      of << "\n";
    }
    of.close();
  }

  std::cout << "  commiting changes in " << (deps_root / d->name()) << "\n";
  return commit((deps_root / d->name()).string(),
                "update dependencies .pkg file");
}

void update_deps(fs::path const& repo, fs::path const& deps_root) {
  dependency_loader l{deps_root};
  l.retrieve(repo);
  auto const sorted = l.sorted();

  std::map<std::string, std::string> new_revs;
  std::queue<dep*> q;
  std::set<dep*> initial;
  for (auto const& dep : sorted) {
    if (auto const new_rev = get_revision((deps_root / dep->name()).string());
        new_rev != dep->ref_) {
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

  // update_revs(repo, dep::root(), new_revs);
}

}  // namespace pkg