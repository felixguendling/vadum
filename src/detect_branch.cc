#include "pkg/detect_branch.h"

#include <sstream>

#include "pkg/exec.h"
#include "pkg/git.h"

namespace pkg {

std::vector<std::string> detect_branch(boost::filesystem::path const& p) {
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

}  // namespace pkg
