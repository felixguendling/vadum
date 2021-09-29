#include <set>
#include <string>

#include "boost/filesystem.hpp"

#include "vadum/dep.h"

namespace vadum {

inline std::ostream& operator<<(std::ostream& out, std::set<dep> const& s) {
  out << "[\n";
  for (auto const& d : s) {
    out << "  {" << d.url_ << " " << d.commit_ << "}\n";
  }
  out << "]";
  return out;
}

std::set<dep> read_deps(boost::filesystem::path const& deps_root, dep const* d,
                        bool recursive = false);
std::set<dep> read_deps(boost::filesystem::path const& deps_root,
                        std::string const& file_content);

}  // namespace vadum
