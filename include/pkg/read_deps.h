#include <string>

#include "boost/filesystem.hpp"

#include "pkg/dep.h"

namespace pkg {

std::vector<dep> read_deps(boost::filesystem::path const& deps_root,
                           dep const* d);
std::vector<dep> read_deps(std::string const& file_content);

}  // namespace pkg
