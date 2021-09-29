#include "vadum/status.h"

#include <sstream>
#include <string>

#include "vadum/exec.h"
#include "vadum/git.h"

namespace fs = boost::filesystem;

namespace vadum {

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

std::map<dep*, status> get_status(std::vector<dep*> const& deps) {
  std::map<dep*, status> dep_status;
  for (auto const& d : deps) {
    auto const s = git_status(d);
    auto const current_commit = get_commit(d->path_);
    auto const commited = current_commit != d->commit_;
    auto& dep_stat = dep_status[d];
    if (!s.clean() || commited) {
      dep_stat.commited_change_ = commited;
      dep_stat.uncommited_change_ = !s.clean();
      if (commited) {
        dep_stat.new_commit_ = current_commit;
        for (auto const& pred : d->recursive_preds()) {
          dep_status[pred].recursive_change_ = true;
        }
      }
    }
  }
  return dep_status;
}

}  // namespace vadum
