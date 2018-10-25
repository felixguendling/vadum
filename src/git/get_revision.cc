#include "pkg/git/get_revision.h"

#include "utl/raii.h"

#include "git2.h"

#include "pkg/git/git_do.h"

namespace pkg {

std::string get_revision(std::string const& p) {
  git_libgit2_init();
  UTL_FINALLY([&]() { git_libgit2_shutdown(); });

  git_repository* repo = nullptr;
  git2_do(git_repository_open_ext(&repo, p.c_str(), 0, nullptr));
  UTL_FINALLY([&]() { git_repository_free(repo); });

  git_object* obj = nullptr;
  git2_do(git_revparse_single(&obj, repo, "HEAD"));
  UTL_FINALLY([&]() { git_object_free(obj); });

  auto const id = git_object_id(obj);
  char hash[41] = {0};
  git_oid_fmt(hash, id);

  return hash;
}

}  // namespace pkg