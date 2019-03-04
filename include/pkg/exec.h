#pragma once

#include "boost/filesystem/path.hpp"
#include "boost/process.hpp"

#include "fmt/format.h"

namespace pkg {

struct exec_result {
  std::string out_, err_;
  int return_code_;
};

template <typename... Args>
exec_result exec(boost::filesystem::path const& working_directory,
                 char const* command, Args... args) {
  namespace bp = boost::process;

  auto cmd = fmt::format(command, args...);
  printf("-> [%s]\n", cmd.c_str());

  bp::ipstream out, err;
  bp::child c(cmd, bp::start_dir = working_directory, bp::std_out > out,
              bp::std_err > err);

  std::string line;
  std::stringstream out_ss, err_ss;
  while (true) {
    if (!out && !err) {
      break;
    }

    if (out) {
      std::getline(out, line);
      if (!line.empty()) {
        out_ss << line << "\n";
        continue;
      }
    }

    if (err) {
      std::getline(err, line);
      if (!line.empty()) {
        err_ss << line << "\n";
        continue;
      }
    }
  }

  c.wait();

  printf("ERROR %d\n", c.exit_code());
  printf("OUTPUT:\n%s\n", out_ss.str().c_str());
  printf("ERROR:\n%s\n", err_ss.str().c_str());
  if (c.exit_code() != 0) {
    throw std::runtime_error("exit code != 0");
  }

  exec_result r;
  r.out_ = out_ss.str();
  r.err_ = err_ss.str();
  r.return_code_ = c.exit_code();
  return r;
}

}  // namespace pkg