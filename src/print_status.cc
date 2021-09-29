#include "vadum/print_status.h"

#include <functional>

#include "fmt/color.h"

#include "vadum/color_output.h"
#include "vadum/dependency_loader.h"
#include "vadum/git.h"
#include "vadum/status.h"

namespace fs = boost::filesystem;

namespace vadum {

void print_status(std::vector<dep*> const& all) {
  enable_color_output();
  auto const dep_status = get_status(all);
  std::function<void(dep*, dep*, int)> print_dep = [&](dep* pred, dep* d,
                                                       int indent) {
    auto const name = d->name();
    auto const& s = dep_status.at(d);

    if (s.commited_change_ || s.recursive_change_) {
      auto const color = s.commited_change_ && s.recursive_change_
                             ? fmt::terminal_color::magenta
                         : s.commited_change_ ? fmt::terminal_color::red
                                              : fmt::terminal_color::blue;
      fmt::print(fg(color), "{:>{}}{}", name, indent * 2 + name.length(),
                 s.uncommited_change_ ? "*" : "");
    } else {
      fmt::print("{:>{}}{}", name, indent * 2 + name.length(),
                 s.uncommited_change_ ? "*" : "");
    }

    if (d->referenced_commits_.size() > 1) {
      fmt::print(" commit={}",
                 git_shorten(d, d->pred_referenced_commits_.at(pred)));
    }

    fmt::print("\n");

    for (auto const& s : d->succs_) {
      print_dep(d, s, indent + 1);
    }
  };

  if (all.empty()) {
    fmt::print("no dependencies found\n");
  } else {
    print_dep(nullptr, all.front(), 0);
  }
}

void print_status(fs::path const& repo, fs::path const& deps_root) {
  dependency_loader l{deps_root};
  l.retrieve(repo);
  print_status(l.get_all());

  fmt::print("\nPackages referenced with multiple versions:\n");
  for (auto const& d : l.get_all()) {
    if (d->referenced_commits_.size() > 1) {
      fmt::print("  {} has {} commits\n", d->name(),
                 d->referenced_commits_.size());
      for (auto const& [commit, preds] : d->referenced_commits_) {
        fmt::print("    commit={} ({}), referenced by ", git_shorten(d, commit),
                   commit_date(d, commit));
        for (auto const& p : preds) {
          fmt::print("{} ", p->name());
        }
        fmt::print("\n");
      }
    }
  }
}

}  // namespace vadum
