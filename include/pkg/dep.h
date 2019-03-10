#pragma once

#include <ostream>
#include <set>
#include <string>

#include "utl/struct/printable.h"

#include "pkg/name_from_url.h"

namespace pkg {

constexpr auto const ROOT = ".";
constexpr auto const PKG_FILE = ".pkg";

struct dep {
  dep(std::string url, std::string commit, std::string branch)
      : url_{std::move(url)},
        commit_{std::move(commit)},
        branch_{std::move(branch)} {}

  static dep* root() {
    static dep d{ROOT, ROOT, ROOT};
    return &d;
  }

  std::string name() const { return name_from_url(url_); }

  friend bool operator<(dep const& a, dep const& b) { return a.url_ < b.url_; }

  friend bool operator==(dep const& a, dep const& b) {
    return a.url_ == b.url_;
  }

  std::string url_, commit_, branch_;
  std::set<dep*> preds_;
  std::set<dep*> succs_;
};

}  // namespace pkg