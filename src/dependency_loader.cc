#include "pkg/dependency_loader.h"

#include <iostream>
#include <set>
#include <sstream>

#include "boost/filesystem.hpp"

#include "fmt/format.h"

#include "utl/erase.h"
#include "utl/get_or_create.h"
#include "utl/to_vec.h"

#include "pkg/git.h"
#include "pkg/read_deps.h"

namespace fs = boost::filesystem;

namespace pkg {

dependency_loader::dependency_loader(fs::path deps_root)
    : deps_root_{std::move(deps_root)} {}

dependency_loader::~dependency_loader() = default;

void dependency_loader::retrieve(
    fs::path const& p, dependency_loader::iteration_fn_t const& iterate) {
  auto& d = dep_mem_.emplace_back(std::make_unique<dep>(*dep::root()));
  retrieve(deps_[ROOT] = d.get(), iterate);
}

std::vector<dep*> dependency_loader::sorted() {
  auto sorted = std::vector<dep*>{};
  auto all = utl::to_vec(dep_mem_, [](auto&& d) { return d.get(); });
  while (!all.empty()) {
    auto const next = std::find_if(begin(all), end(all),
                                   [](auto&& d) { return d->succs_.empty(); });
    verify(next != end(all), "cycle detected");

    auto d = (*next);
    if (d->name() != ROOT) {
      sorted.emplace_back(d);
      for (auto const& pred : d->preds_) {
        utl::erase(pred->succs_, d);
      }
    }
    all.erase(next);
  }
  return sorted;
}

void dependency_loader::retrieve(
    dep* pred, dependency_loader::iteration_fn_t const& iterate) {
  auto const deps = read_deps(deps_root_, pred);
  for (auto const d : deps) {
    std::cout << "[" << d.name() << "]\n"  //
              << "  url=" << d.url_ << "\n"  //
              << "  branch=" << d.branch_ << "\n"  //
              << "  commit=" << d.commit_ << "\n";
  }
  std::cout << "\n";

  for (auto const& d : deps) {

    auto succ = utl::get_or_create(deps_, d.url_, [&]() {
      auto next = dep_mem_.emplace_back(std::make_unique<dep>(d)).get();
      iterate(next->url_, (deps_root_ / next->name()).string(), next->commit_);
      retrieve(next, iterate);
      return next;
    });

    if (succ->commit_ != d.commit_) {
      std::cerr << "non-matching ref: " << d.url_ << "\n";
      std::cerr << d.commit_ << " from: " << pred->url_ << "\n";
      for (auto const* succ_pred : succ->preds_) {
        std::cerr << succ->commit_ << " from: " << succ_pred->url_ << "\n";
      }
      throw std::runtime_error{"non-matching ref"};
    }

    succ->preds_.insert(pred);
    pred->succs_.insert(succ);
  }
}

}  // namespace pkg