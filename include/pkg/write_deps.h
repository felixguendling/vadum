#pragma once

#include <ostream>

#include "pkg/dep.h"

namespace pkg {

inline void write_deps(std::ostream& os, std::vector<dep> const& deps) {
  for (auto const d : deps) {
    os << "[" << d.name() << "]\n"  //
       << "  url=" << d.url_ << "\n"  //
       << "  branch=" << d.branch_ << "\n"  //
       << "  commit=" << d.commit_ << "\n";
  }
  os << "\n";
}

}  // namespace pkg
