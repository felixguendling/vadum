#pragma once

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "boost/filesystem.hpp"

#include "utl/parser/util.h"

namespace pkg {

constexpr auto const PKG_FILE = ".pkg";
constexpr auto const ROOT = ".";

struct dep;

std::vector<std::pair<std::string, std::string>> read_deps(
    boost::filesystem::path const& deps_root, dep const*);

std::string name_from_url(std::string const&);

struct dep {
  dep(std::string url, std::string ref)
      : url_{std::move(url)}, ref_{std::move(ref)} {}

  static dep* root() {
    static dep d{ROOT, ROOT};
    return &d;
  }

  std::string name() const { return name_from_url(url_); }

  friend bool operator<(dep const& a, dep const& b) { return a.url_ < b.url_; }

  friend bool operator==(dep const& a, dep const& b) {
    return a.url_ == b.url_;
  }

  std::string url_, ref_;
  std::set<dep*> preds_;
  std::set<dep*> succs_;
};

struct dependency_loader {
public:
  using iteration_fn_t = std::function<void(std::string,  // URL
                                            std::string,  // ref
                                            std::string  // path
                                            )>;

  explicit dependency_loader(boost::filesystem::path deps_root);
  ~dependency_loader();

  void retrieve(boost::filesystem::path const&,
                iteration_fn_t const& = [](std::string const&,
                                           std::string const&,
                                           std::string const&) {});
  std::vector<dep*> sorted();

private:
  void retrieve(dep* pred, iteration_fn_t const&);

  boost::filesystem::path deps_root_;
  std::map<std::string, dep*> deps_;
  std::vector<std::unique_ptr<dep>> dep_mem_;
};

}  // namespace pkg