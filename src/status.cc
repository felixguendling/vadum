#include "pkg/status.h"

#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "fmt/color.h"

#include "pkg/dependency_loader.h"
#include "pkg/exec.h"
#include "pkg/git.h"

namespace fs = boost::filesystem;

namespace pkg {

struct git_status {
  explicit git_status(dep const* d) {
    std::stringstream ss;
    ss.str(exec(d->path_, "git status --short").out_);

    std::string line;
    while (std::getline(ss, line)) {
      changed_.emplace_back(line);
    }
  }
  bool clean() const { return changed_.empty(); }
  std::vector<std::string> changed_;
};

void print_status(fs::path const& repo, fs::path const& deps_root) {
  dependency_loader l{deps_root};
  l.retrieve(repo);

  std::set<dep*> marked;
  for (auto const& d : l.get_all()) {
    if (auto const s = git_status(d);
        !s.clean() || get_commit(d->path_) != d->commit_) {
      auto const rec_preds = d->recursive_preds();
      marked.insert(d);
      marked.insert(begin(rec_preds), end(rec_preds));
    }
  }

  std::function<void(dep*, int)> print_dep = [&](dep* d, int indent) {
    auto const name = d->name();
    if (marked.find(d) == end(marked)) {
      fmt::print("{:>{}}\n", name, indent * 2 + name.length());
    } else {
      fmt::print(fg(fmt::terminal_color::red), "{:>{}}\n", name,
                 indent * 2 + name.length());
    }

    for (auto const& s : d->succs_) {
      print_dep(s, indent + 1);
    }
  };

  print_dep(l.get_all().front(), 0);
}

}  // namespace pkg