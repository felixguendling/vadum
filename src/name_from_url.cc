#include "vadum/name_from_url.h"

#include "utl/verify.h"

#include "vadum/dep.h"

namespace vadum {

std::string name_from_url(std::string const& url) {
  if (url == ROOT) {
    return ROOT;
  }

  auto const slash_pos = url.find_last_of('/');
  auto const dot_pos = url.find_last_of('.');
  utl::verify(slash_pos != std::string::npos, "no slash in url");
  utl::verify(dot_pos != std::string::npos, "no dot in url");
  utl::verify(slash_pos < dot_pos, "slash and dot in wrong order");
  return url.substr(slash_pos + 1, dot_pos - slash_pos - 1);
}

}  // namespace vadum
