#pragma once

#include <map>
#include <set>
#include <string>

#include "utl/struct/comparable.h"

#include "boost/filesystem/path.hpp"

namespace vadum {

constexpr auto const ROOT = ".";
constexpr auto const VADUM_FILE = "vadum.ini";

struct branch_commit {
  MAKE_COMPARABLE()
  std::string branch_, commit_;
};

struct dep {
  dep() = default;

  dep(boost::filesystem::path const& deps_root, std::string url,
      std::string commit);

  static dep root(boost::filesystem::path const& root_repo);

  void write_vadum_file() const;

  bool is_root() const;

  boost::filesystem::path vadum_file() const;

  std::string name() const;

  std::set<dep*> recursive_preds() const;

  friend bool operator<(dep const& a, dep const& b) { return a.url_ < b.url_; }
  friend bool operator==(dep const& a, dep const& b) {
    return a.url_ == b.url_;
  }

  boost::filesystem::path path_;
  std::string url_, commit_;
  std::set<dep*> preds_;
  std::set<dep*> succs_;

  std::map<std::string, std::set<dep*>> referenced_commits_;
  std::map<dep*, std::string> pred_referenced_commits_;
};

}  // namespace vadum
