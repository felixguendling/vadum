#pragma once

#include "utl/erase.h"

#include "pkg/exec.h"

namespace pkg {

inline void git_clone(std::string const& url, std::string const& commit,
                      boost::filesystem::path const& p) {
  exec(p.parent_path(), "git clone {} {}", url,
       boost::filesystem::absolute(p).string());
  exec(p, "git checkout {}", commit);
  exec(p, "git submodule update --init --recursive");
  exec(p, "git branch -a --contains {}", commit);
}

inline std::string get_commit(boost::filesystem::path const& p) {
  auto out = exec(p, "git rev-parse HEAD").out_;
  utl::erase(out, '\n');
  return out;
}

inline std::string commit(boost::filesystem::path const& p,
                          std::string const& msg) {
  constexpr auto const PKG_FILE = ".pkg";
  exec(p, "git add {}", PKG_FILE);
  exec(p, "git commit -m \"{}\"", msg);
  return get_commit(p);
}

}  // namespace pkg