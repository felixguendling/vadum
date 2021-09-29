#include <set>
#include <string>

#include "boost/filesystem.hpp"

#include "vadum/dep.h"

namespace vadum {

std::set<dep> read_deps(boost::filesystem::path const& deps_root, dep const* d,
                        bool recursive = false);
std::set<dep> read_deps(boost::filesystem::path const& deps_root,
                        std::string const& file_content);

}  // namespace vadum
