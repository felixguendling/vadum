#include "pkg/print_status.h"

#include <functional>

#include "fmt/color.h"

#include "pkg/color_output.h"
#include "pkg/dependency_loader.h"
#include "pkg/status.h"

namespace fs = boost::filesystem;

namespace pkg {

void print_status(std::vector<dep*> const& all) {
  enable_color_output();
  auto const dep_status = get_status(all);
  std::function<void(dep*, int)> print_dep = [&](dep* d, int indent) {
    auto const name = d->name();
    auto const& s = dep_status.at(d);

    if (s.commited_change_ || s.recursive_change_) {
      auto const color = s.commited_change_ && s.recursive_change_
                             ? fmt::terminal_color::magenta
                             : s.commited_change_ ? fmt::terminal_color::red
                                                  : fmt::terminal_color::blue;
      fmt::print(fg(color), "{:>{}}{}\n", name, indent * 2 + name.length(),
                 s.uncommited_change_ ? '*' : ' ');
    } else {
      fmt::print("{:>{}}{}\n", name, indent * 2 + name.length(),
                 s.uncommited_change_ ? '*' : ' ');
    }

    for (auto const& s : d->succs_) {
      print_dep(s, indent + 1);
    }
  };

  if (all.empty()) {
    fmt::print("no dependencies found\n");
  } else {
    print_dep(all.front(), 0);
  }
}

void print_status(fs::path const& repo, fs::path const& deps_root) {
  dependency_loader l{deps_root};
  l.retrieve(repo);
  print_status(l.get_all());
}

}  // namespace pkg