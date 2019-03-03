#include "pkg/git/git_commit.h"

#include "pkg/git/exec.h"
#include "pkg/git/get_revision.h"

namespace pkg {

constexpr auto const PKG_FILE = ".pkg";

std::string commit(boost::filesystem::path const& p, std::string const& msg) {
  exec(p, "git add {}", PKG_FILE);
  exec(p, "git commit -m \"{}\"", msg);
  return get_revision(p);
}

}  // namespace pkg