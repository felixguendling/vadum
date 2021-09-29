#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "boost/filesystem/path.hpp"

#include "vadum/dep.h"

namespace vadum {

struct dependency_loader {
public:
  using iteration_fn_t =
      std::function<void(dep*, std::string const& /* commit */)>;
  using async_iteration_fn_t = std::function<void(dep*, iteration_fn_t)>;

  explicit dependency_loader(boost::filesystem::path deps_root);
  ~dependency_loader();

  void retrieve(
      boost::filesystem::path const&,
      iteration_fn_t const& = [](dep*, std::string const&) {},
      bool recursive = false);

  dep* root();
  std::vector<dep*> sorted();
  std::vector<dep*> get_all() const;
  std::optional<dep*> resolve(std::string const& url) const;

private:
  void retrieve(dep* pred, iteration_fn_t const&, bool recursive);

  boost::filesystem::path deps_root_;
  std::map<std::string, dep*> deps_;
  std::vector<std::unique_ptr<dep>> dep_mem_;
};

}  // namespace vadum
