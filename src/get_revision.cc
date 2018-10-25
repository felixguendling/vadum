#include "pkg/get_revision.h"

#include "utl/raii.h"

#include "git2.h"

namespace pkg {

std::string get_revision(std::string const& p) {
  git_libgit2_init();
  UTL_FINALLY([&]() { git_libgit2_shutdown(); });

  git_repository* repo = nullptr;
  if (auto const error = git_repository_open_ext(&repo, p.c_str(), 0, nullptr);
      error != 0) {
    auto const e = giterr_last();
    throw std::runtime_error(e ? e->message : "unable to open repo");
  }
  UTL_FINALLY([&]() { git_repository_free(repo); });

  git_object* obj = nullptr;
  if (auto const error = git_revparse_single(&obj, repo, "HEAD"); error != 0) {
    auto const e = giterr_last();
    throw std::runtime_error(e ? e->message : "unable to open repo");
  }
  UTL_FINALLY([&]() { git_object_free(obj); });

  auto const id = git_object_id(obj);
  char hash[41] = {0};
  git_oid_fmt(hash, id);

  return hash;
}

}  // namespace pkg