#include "vadum/detect_branch.h"

#include <sstream>

#include "fmt/format.h"
#include "fmt/ranges.h"

#include "vadum/dependency_loader.h"
#include "vadum/exec.h"
#include "vadum/git.h"

namespace fs = boost::filesystem;

namespace vadum {

std::vector<std::string> detect_branch(fs::path const& p) {
  return detect_branch(
      exec(p, "git branch -a --contains {}", get_commit(p)).out_);
}

std::vector<std::string> detect_branch(
    std::string const& git_branch_contains_output) {
  std::stringstream ss;
  ss.str(git_branch_contains_output);

  std::vector<std::string> branches;
  std::string line;
  while (std::getline(ss, line)) {
    if (line.empty() || line.find("HEAD") != std::string::npos) {
      continue;
    }

    auto const start_tag = "remotes/origin/";
    auto const start_tag_pos = line.find(start_tag);
    if (start_tag_pos == std::string::npos) {
      continue;
    }

    auto const start_tag_len = std::strlen(start_tag);
    branches.emplace_back(line.substr(start_tag_pos + start_tag_len));
  }
  return branches;
}

bool detect_branch(dep* d) {
  if (!d->branch_.empty()) {
    return false;
  }

  if (auto const branches = detect_branch(d->path_); branches.size() == 1) {
    d->branch_ = branches.front();
    fmt::print("Branch of {} in {} is unambiguous. Using: {}\n", d->commit_,
               d->url_, d->branch_);
    return true;
  } else {
    fmt::print("Branch of {} in {} is ambiguous. Options: {}\n", d->commit_,
               d->url_, branches);
    return false;
  }
}

}  // namespace vadum
