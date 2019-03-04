#include "pkg/dependency_loader.h"

#include <iostream>
#include <set>
#include <sstream>

#include "boost/filesystem.hpp"

#include "utl/erase.h"
#include "utl/get_or_create.h"
#include "utl/to_vec.h"

#include "pkg/git.h"

namespace fs = boost::filesystem;

namespace pkg {

std::vector<std::pair<std::string, std::string>> read_deps(
    fs::path const& deps_root, dep const* d) {
  auto const p = (d->name() == ROOT) ? (fs::path{ROOT} / PKG_FILE)
                                     : (deps_root / d->name() / PKG_FILE);

  std::cout << "deps from " << p << "\n";
  if (!fs::is_regular_file(p)) {
    std::cout << "  " << p << " not found\n";
    return {};
  }

  std::ifstream f{p.string().c_str()};
  f.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  std::vector<std::pair<std::string, std::string>> deps;
  std::string line, base, path, ref;
  while (!f.eof() && f.peek() != EOF && std::getline(f, line)) {
    if (line.empty()) {
      continue;
    }

    if (line[0] == '*') {
      base = line.substr(1);
    } else {
      std::stringstream ss{line};
      ss >> path >> ref;
      deps.emplace_back(base.empty() ? path : base + ":" + path, ref);
    }
  }

  for (auto const& [url, ref] : deps) {
    std::cout << "  " << url << " " << ref << "\n";
  }

  return deps;
}

std::string name_from_url(std::string const& url) {
  if (url == ROOT) {
    return ROOT;
  }

  auto const slash_pos = url.find_last_of('/');
  auto const dot_pos = url.find_last_of('.');
  verify(slash_pos != std::string::npos, "no slash in url");
  verify(dot_pos != std::string::npos, "no dot in url");
  verify(slash_pos < dot_pos, "slash and dot in wrong order");
  return url.substr(slash_pos + 1, dot_pos - slash_pos - 1);
}

dependency_loader::dependency_loader(fs::path deps_root)
    : deps_root_{std::move(deps_root)} {}

dependency_loader::~dependency_loader() = default;

void dependency_loader::retrieve(
    fs::path const& p, dependency_loader::iteration_fn_t const& iterate) {
  auto& d = dep_mem_.emplace_back(std::make_unique<dep>(ROOT, ROOT));
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
  for (auto const& url_ref : read_deps(deps_root_, pred)) {
    auto const url = url_ref.first;
    auto const ref = url_ref.second;
    auto succ = utl::get_or_create(deps_, url, [&]() {
      auto next = dep_mem_.emplace_back(std::make_unique<dep>(url, ref)).get();
      iterate(next->url_, (deps_root_ / next->name()).string(), next->ref_);
      retrieve(next, iterate);
      return next;
    });
    if (succ->ref_ != ref) {
      std::cerr << "non-matching ref: " << url << "\n";
      std::cerr << ref << " from: " << pred->url_ << "\n";
      for (auto const* succ_pred : succ->preds_) {
        std::cerr << succ->ref_ << " from: " << succ_pred->url_ << "\n";
      }
      verify(false, "non-matching ref");
    }
    succ->preds_.insert(pred);
    pred->succs_.insert(succ);
  }
}

}  // namespace pkg