#include "pkg/print_status.h"

#include <functional>

#include "fmt/color.h"

#include "pkg/dependency_loader.h"
#include "pkg/status.h"

namespace fs = boost::filesystem;

namespace pkg {

void print_status(fs::path const& repo, fs::path const& deps_root) {
  dependency_loader l{deps_root};
  l.retrieve(repo);

  auto const dep_status = get_status(l.get_all());
  std::function<void(dep*, int)> print_dep = [&](dep* d, int indent) {
    auto const name = d->name();
    if (dep_status.at(d).unchanged()) {
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