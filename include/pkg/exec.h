#pragma once

#include "boost/filesystem/path.hpp"

#include "fmt/format.h"

namespace pkg {

struct exec_result {
  std::string out_, err_;
  int return_code_;
};

exec_result exec(boost::filesystem::path const& working_directory,
                 std::string const& cmd);

template <typename... Args>
exec_result exec(boost::filesystem::path const& working_directory,
                 char const* command, Args... args) {
  return exec(working_directory, fmt::format(command, args...));
}

}  // namespace pkg