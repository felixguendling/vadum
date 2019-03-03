#include "pkg/git/git_clone.h"

#include "pkg/git/exec.h"

namespace pkg {

void git_clone(std::string const& url, std::string const& ref,
               boost::filesystem::path const& p) {
  exec(p.parent_path(), "git clone {} {}", url,
       boost::filesystem::absolute(p).string());
  exec(p, "git checkout {}", ref);
}

}  // namespace pkg