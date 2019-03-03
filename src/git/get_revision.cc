#include "pkg/git/get_revision.h"

#include "pkg/git/exec.h"

namespace pkg {

std::string get_revision(boost::filesystem::path const& p) {
  return exec(p, "git rev-parse HEAD").out_;
}

}  // namespace pkg