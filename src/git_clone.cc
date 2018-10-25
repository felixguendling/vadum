#include "pkg/git_clone.h"

#include "git2.h"
#include "git2/clone.h"

#include "utl/parser/util.h"
#include "utl/raii.h"

namespace pkg {

int agent(git_cred** out, const char*, const char*, unsigned int,
          void* payload_ptr) {
  auto& payload = *reinterpret_cast<bool*>(payload_ptr);
  if (payload == true) {
    return -1;
  }
  payload = true;
  return git_cred_ssh_key_from_agent(out, "git");
}

void git_clone(std::string const& url, std::string const& ref,
               std::string const& path) {
  git_libgit2_init();
  UTL_FINALLY([&]() { git_libgit2_shutdown(); });

  git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
  checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

  auto payload = false;
  git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
  clone_opts.checkout_opts = checkout_opts;
  clone_opts.fetch_opts.callbacks.credentials = agent;
  clone_opts.fetch_opts.callbacks.payload = &payload;

  git_repository* cloned_repo = nullptr;
  if (auto const err =
          git_clone(&cloned_repo, url.c_str(), path.c_str(), &clone_opts);
      err != 0) {
    auto const e = giterr_last();
    throw std::runtime_error(e ? e->message : "unkown git error");
  }
  UTL_FINALLY([&]() { git_repository_free(cloned_repo); });

  git_object* target = nullptr;
  verify(git_revparse_single(&target, cloned_repo, ref.c_str()) == 0,
         "could not resolve ref");
  verify(git_reset(cloned_repo, target, GIT_RESET_HARD, nullptr) == 0,
         "could not reset to ref");
}

}  // namespace pkg