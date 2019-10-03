#pragma once

#include <map>
#include <set>
#include <string>

#include "utl/struct/comparable.h"

#include "boost/filesystem/path.hpp"

namespace pkg {

constexpr auto const ROOT = ".";
constexpr auto const PKG_FILE = ".pkg";

struct dep {
  dep() = default;

  dep(boost::filesystem::path const& deps_root, std::string url,
      std::string commit, std::string branch);

  static dep root(boost::filesystem::path const& root_repo);

  void write_pkg_file() const;

  bool is_root() const;

  boost::filesystem::path pkg_file() const;

  std::string name() const;

  boost::filesystem::path bare_repo_path() const;

  std::set<dep*> recursive_preds() const;

  friend bool operator<(dep const& a, dep const& b) { return a.url_ < b.url_; }
  friend bool operator==(dep const& a, dep const& b) {
    return a.url_ == b.url_;
  }

  boost::filesystem::path path_;
  std::string url_, commit_, branch_;
  std::set<dep*> preds_;
  std::set<dep*> succs_;

  struct branch_commit {
    MAKE_COMPARABLE()
    std::string branch_, commit_;
  };
  std::map<branch_commit, std::vector<dep*>> referenced_commits_;
};

}  // namespace pkg