#include "pkg/dependency_loader.h"

#include <iostream>
#include <set>
#include <sstream>

#include "boost/filesystem.hpp"

#include "utl/erase.h"
#include "utl/get_or_create.h"
#include "utl/to_vec.h"

#include "pkg/git_clone.h"

namespace fs = boost::filesystem;

namespace pkg {

constexpr auto const PKG_FILE = ".pkg";

struct dependency_loader::dep {
  dep(std::string url, std::string ref)
      : url_{std::move(url)}, ref_{std::move(ref)} {}

  std::string name() const {
    if (url_ == ROOT) {
      return ROOT;
    }

    auto const slash_pos = url_.find_last_of('/');
    auto const dot_pos = url_.find_last_of('.');
    verify(slash_pos != std::string::npos, "no slash in url");
    verify(dot_pos != std::string::npos, "no dot in url");
    verify(slash_pos < dot_pos, "slash and to in wrong order");
    return url_.substr(slash_pos + 1, dot_pos - slash_pos - 1);
  }

  friend bool operator<(dep const& a, dep const& b) { return a.url_ < b.url_; }

  friend bool operator==(dep const& a, dep const& b) {
    return a.url_ == b.url_;
  }

  std::string url_, ref_;
  std::set<dep*> preds_;
  std::set<dep*> succs_;
};

dependency_loader::dependency_loader(fs::path deps_root)
    : deps_root_{std::move(deps_root)} {}

dependency_loader::~dependency_loader() = default;

void dependency_loader::retrieve(fs::path const& p) {
  auto& d = dep_mem_.emplace_back(std::make_unique<dep>(ROOT, ROOT));
  retrieve(deps_[ROOT] = d.get());
}

std::vector<dependency_loader::dep*> dependency_loader::sorted() {
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

std::vector<std::pair<std::string, std::string>> dependency_loader::read_deps(
    dep const* d) {
  auto const p = (d->name() == ROOT) ? (fs::path{ROOT} / PKG_FILE)
                                     : (deps_root_ / d->name() / PKG_FILE);

  std::cout << "deps from " << p << "\n";
  if (!fs::is_regular_file(p)) {
    std::cout << "  " << p << " is not a regular file\n";
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

void dependency_loader::retrieve(dep* pred) {
  for (auto const& url_ref : read_deps(pred)) {
    auto const url = url_ref.first;
    auto const ref = url_ref.second;
    auto succ = utl::get_or_create(deps_, url, [&]() {
      auto next = dep_mem_.emplace_back(std::make_unique<dep>(url, ref)).get();
      git_clone(next->url_, (deps_root_ / next->name()).string(), next->ref_);
      retrieve(next);
      return next;
    });
    verify(succ->ref_ == ref, "non-matching ref");
    succ->preds_.insert(pred);
    pred->succs_.insert(succ);
  }
}

std::string name(dependency_loader::dep const* d) { return d->name(); }

}  // namespace pkg