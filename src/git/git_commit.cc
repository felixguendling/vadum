#include "pkg/git/git_commit.h"

#include "git2.h"
#include "git2/signature.h"

#include "utl/raii.h"

#include "pkg/git/git_do.h"

namespace pkg {

constexpr auto const PKG_FILE = ".pkg";

std::string commit(std::string const& p, std::string const& msg) {
  git_libgit2_init();
  UTL_FINALLY([&]() { git_libgit2_shutdown(); });

  // Open repositoriy.
  git_repository* repo = nullptr;
  git2_do(git_repository_open_ext(&repo, p.c_str(), 0, nullptr));
  UTL_FINALLY([&]() { git_repository_free(repo); });

  // --- STAGE FILE TO INDEX ---
  // Get repo index.
  git_index* idx = nullptr;
  git2_do(git_repository_index(&idx, repo));

  // Add .pkg file.
  git2_do(git_index_add_bypath(idx, PKG_FILE));
  UTL_FINALLY([&]() { git_index_free(idx); });

  // Write tree with changed file.
  git_oid tree_id = {0};
  git2_do(git_index_write_tree(&tree_id, idx));

  // --- GIT COMMIT ---
  // Get tree pointer.
  git_tree* tree = nullptr;
  git2_do(git_tree_lookup(&tree, repo, &tree_id));
  UTL_FINALLY([&]() { git_tree_free(tree); });

  // Get parent reference to current HEAD.
  git_oid parent_oid = {0};
  git2_do(git_reference_name_to_id(&parent_oid, repo, "HEAD"));

  // Get parent commit.
  git_commit* parent = nullptr;
  git2_do(git_commit_lookup(&parent, repo, &parent_oid));
  UTL_FINALLY([&]() { git_commit_free(parent); });

  // Author information (default author from configuration).
  git_signature* signature = nullptr;
  git2_do(git_signature_default(&signature, repo));
  UTL_FINALLY([&]() { git_signature_free(signature); });

  // Commit changed file.
  git_oid commit_id = {0};
  git2_do(git_commit_create_v(&commit_id, repo, "HEAD", signature, signature,
                              nullptr, msg.c_str(), tree, 1, parent));

  // --- RESET TO HEAD ---
  // Get HEAD for reset.
  git_reference* head;
  git2_do(git_repository_head(&head, repo));
  UTL_FINALLY([&]() { git_reference_free(head); });

  // List .pkg file to reset.
  char* paths[] = {const_cast<char*>(PKG_FILE)};
  git_strarray path_specs;
  path_specs.strings = paths;
  path_specs.count = 1;

  // Get HEAD commit.
  git_object* head_commit;
  git2_do(git_reference_peel(&head_commit, head, GIT_OBJ_COMMIT));
  UTL_FINALLY([&]() { git_object_free(head_commit); });

  // Reset to HEAD commit.
  git2_do(git_reset_default(repo, head_commit, &path_specs));

  // --- RETURN COMMIT ID ---
  char hash[41] = {0};
  git_oid_fmt(hash, &commit_id);
  return hash;
}

}  // namespace pkg