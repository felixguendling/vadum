#include <fstream>
#include <iostream>
#include <queue>
#include <set>

#include "boost/filesystem.hpp"

#include "pkg/dependency_loader.h"
#include "pkg/get_revision.h"
#include "pkg/git_clone.h"

// ========================================================================
// git_commit.h
// ------------------------------------------------------------------------
#include "git2.h"
#include "git2/status.h"

#include "utl/raii.h"

// ========================================================================
// git_commit.cc
// ------------------------------------------------------------------------
#include "git2/signature.h"

#define git2_do(...)                                                    \
  if (__VA_ARGS__ != 0) {                                               \
    auto const e = giterr_last();                                       \
    throw std::runtime_error(e ? e->message : "unknown libgit2 error"); \
  }

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
  git2_do(git_index_add_bypath(idx, pkg::PKG_FILE));
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
  char* paths[] = {const_cast<char*>(pkg::PKG_FILE)};
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

// ========================================================================
// update_deps.cc
// ------------------------------------------------------------------------
namespace fs = boost::filesystem;
using namespace pkg;

void clear_and_load_deps(fs::path const& repo, fs::path const& deps_root) {
  fs::remove_all(deps_root);
  fs::create_directories(deps_root);

  dependency_loader l{deps_root};
  l.retrieve(repo, [](std::string const& url, std::string const& path,
                      std::string const& ref) {
    std::cout << "git clone " << url << "::" << ref << "\n";
    git_clone(url, ref, path);
  });

  std::ofstream of{"deps/CMakeLists.txt"};
  of << "project(" + deps_root.string() << ")\n"
     << "cmake_minimum_required(VERSION 3.11)\n\n";
  for (auto const& v : l.sorted()) {
    of << "add_subdirectory(" << v->name() << " EXCLUDE_FROM_ALL)\n";
  }
}

std::string update_revs(fs::path const& deps_root, dep* d,
                        std::map<std::string, std::string> const& new_revs) {
  {
    auto const deps = read_deps(deps_root, d);
    std::ofstream of{(deps_root / d->name() / PKG_FILE).string().c_str()};
    for (auto const& [succ_url, succ_ref] : deps) {
      of << succ_url << " ";
      if (auto const it = new_revs.find(name_from_url(succ_url));
          it != end(new_revs)) {
        of << it->second;
      } else {
        of << succ_ref;
      }
      of << "\n";
    }
    of.close();
  }

  std::cout << "  commiting changes in " << (deps_root / d->name()) << "\n";
  return commit((deps_root / d->name()).string(),
                "update dependencies .pkg file");
}

void update_deps(fs::path const& repo, fs::path const& deps_root) {
  dependency_loader l{deps_root};
  l.retrieve(repo);
  auto const sorted = l.sorted();

  std::map<std::string, std::string> new_revs;
  std::queue<dep*> q;
  std::set<dep*> initial;
  for (auto const& dep : sorted) {
    if (auto const new_rev = get_revision((deps_root / dep->name()).string());
        new_rev != dep->ref_) {
      new_revs[dep->name()] = new_rev;
      q.emplace(dep);
      initial.emplace(dep);
    }
  }

  std::set<dep*> visited;
  std::set<dep*> referenced;
  while (!q.empty()) {
    auto const next = q.front();
    q.pop();
    visited.emplace(next);
    for (auto const& pred : next->preds_) {
      q.emplace(pred);
      referenced.emplace(pred);
    }
  }

  for (auto const& s : sorted) {
    if (visited.find(s) != end(visited) &&
        (initial.find(s) == end(initial) ||
         referenced.find(s) != end(referenced))) {
      std::cout << "update refs of " << s->name() << "\n";
      new_revs[s->name()] = update_revs(deps_root, s, new_revs);
    }
  }

  // update_revs(repo, dep::root(), new_revs);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("usage: %s [-u for update] [-l for load]\n", argv[0]);
    return 0;
  } else if (std::strcmp(argv[1], "-u") == 0) {
    update_deps(fs::path{"."}, fs::path("deps"));
  } else if (std::strcmp(argv[1], "-l") == 0) {
    clear_and_load_deps(fs::path{"."}, fs::path("deps"));
  }
}