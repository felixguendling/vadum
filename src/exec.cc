#include "pkg/exec.h"

#include <iostream>
#include <sstream>

#include "boost/process/child.hpp"
#include "boost/process/io.hpp"
#include "boost/process/start_dir.hpp"

namespace pkg {

exec_result exec(boost::filesystem::path const& working_directory,
                 std::string const& cmd) {
  std::cout << "[" << working_directory << "] " << cmd << std::endl;

  namespace bp = boost::process;
  try {
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

    if (c.exit_code() != 0) {
      throw std::runtime_error(
          fmt::format("COMMAND [{}] in [{}] failed: \n"
                      "EXIT_CODE {}\n"
                      "OUTPUT:\n{}\n"
                      "ERROR:\n{}\n",
                      cmd, working_directory.string(), c.exit_code(),
                      out_ss.str(), err_ss.str()));
    }

    exec_result r;
    r.out_ = out_ss.str();
    r.err_ = err_ss.str();
    r.return_code_ = c.exit_code();
    return r;
  } catch (std::exception const& e) {
    exec_result r;
    r.err_ = e.what();
    r.return_code_ = -1;
    return r;
  }
}

}  // namespace pkg