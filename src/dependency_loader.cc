#include "vadum/dependency_loader.h"

#include <set>

#include "utl/get_or_create.h"
#include "utl/to_vec.h"
#include "utl/verify.h"

#include "vadum/git.h"
#include "vadum/read_deps.h"

namespace fs = boost::filesystem;

namespace vadum {

dependency_loader::dependency_loader(fs::path deps_root)
    : deps_root_{std::move(deps_root)} {}

dependency_loader::~dependency_loader() = default;

dep* dependency_loader::root() { return deps_.at(ROOT); }

std::optional<dep*> dependency_loader::resolve(std::string const& url) const {
  if (auto it = deps_.find(url); it == end(deps_)) {
    return std::nullopt;
  } else {
    return std::make_optional(it->second);
  }
}

std::vector<dep*> dependency_loader::sorted() {
  auto written = std::set<dep*>{};
  auto sorted = std::vector<dep*>{};
  auto all = get_all();
  while (!all.empty()) {
    // Find the first dep without unwritten successors.
    // There has to be at least one in a cycle-free dependency graph.
    auto const next = std::find_if(begin(all), end(all), [&](auto&& d) {
      return std::all_of(begin(d->succs_), end(d->succs_), [&](auto&& s) {
        return written.find(s) != end(written);
      });
    });
    utl::verify(next != end(all), "cycle detected");

    auto d = (*next);
    written.emplace(d);
    sorted.emplace_back(d);
    all.erase(next);
  }
  return sorted;
}

std::vector<dep*> dependency_loader::get_all() const {
  return utl::to_vec(dep_mem_, [](auto&& d) { return d.get(); });
}

void dependency_loader::retrieve(
    fs::path const& p, dependency_loader::iteration_fn_t const& iterate,
    bool const recursive) {
  deps_.clear();
  dep_mem_.clear();
  auto& d = dep_mem_.emplace_back(std::make_unique<dep>(dep::root(p)));
  retrieve(deps_[ROOT] = d.get(), iterate, recursive);
}

void dependency_loader::retrieve(
    dep* pred, dependency_loader::iteration_fn_t const& iterate,
    bool const recursive) {
  for (auto const& d : read_deps(deps_root_, pred, recursive)) {
    auto succ = utl::get_or_create(deps_, d.url_, [&]() {
      return dep_mem_.emplace_back(std::make_unique<dep>(d)).get();
    });

    succ->referenced_commits_[d.commit_].emplace(pred);
    succ->pred_referenced_commits_[pred] = d.commit_;
    succ->preds_.insert(pred);
    pred->succs_.insert(succ);

    iterate(succ, d.commit_);
    retrieve(succ, iterate, recursive);
  }
}

}  // namespace vadum
