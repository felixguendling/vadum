#include "pkg/read_deps.h"

#include <fstream>
#include <istream>
#include <optional>
#include <sstream>

#include "boost/property_tree/ini_parser.hpp"
#include "boost/property_tree/ptree.hpp"

#include "utl/to_vec.h"

namespace fs = boost::filesystem;

namespace pkg {

std::vector<dep> read_deps_old(std::string const& file_content) {
  std::stringstream ss;
  ss.str(file_content);

  std::vector<dep> deps;
  std::string line, url, commit, branch;
  while (true) {
    std::string line;
    if (!std::getline(ss, line)) {
      break;
    }
    if (line.empty()) {
      continue;
    }

    std::stringstream line_ss;
    line_ss.str(line);
    line_ss >> url >> commit >> branch;

    deps.emplace_back(url, commit, branch);
  }

  return deps;
}

std::vector<dep> read_deps_new(std::string const& file_content) {
  namespace pt = boost::property_tree;

  std::stringstream ss;
  ss.str(file_content);

  pt::ptree tree;
  pt::read_ini(ss, tree);
  return utl::to_vec(tree, [](auto const& entry) {
    auto const& settings = entry.second;
    return dep{settings.template get<std::string>(pt::path{"url"}, ""),
               settings.template get<std::string>(pt::path{"commit"}, ""),
               settings.template get<std::string>(pt::path{"branch"}, "")};
  });
}

std::optional<std::string> read_file(fs::path const& path) {
  std::ifstream f{path.string().c_str(), std::ios::binary | std::ios::ate};
  f.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  auto const size = f.tellg();
  f.seekg(0, std::ios::beg);

  std::string buffer;
  buffer.resize(size);

  return f.read(&buffer[0], size) ? std::make_optional(buffer) : std::nullopt;
}

std::vector<dep> read_deps(std::string const& file_content) {
  if (file_content.empty()) {
    return {};
  }

  auto const first_char_it =
      std::find_if(begin(file_content), end(file_content),
                   [](auto const c) { return !std::isspace(c); });
  return (first_char_it == end(file_content) || *first_char_it != '[')
             ? read_deps_old(file_content)
             : read_deps_new(file_content);
}

std::vector<dep> read_deps(fs::path const& deps_root, dep const* d) {
  auto const p = (d->name() == ROOT) ? (fs::path{ROOT} / PKG_FILE)
                                     : (deps_root / d->name() / PKG_FILE);

  if (!fs::is_regular_file(p)) {
    return {};
  }

  auto const file_content = read_file(p);
  return file_content ? read_deps(*file_content) : std::vector<dep>{};
}

}  // namespace pkg