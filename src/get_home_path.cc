#include "pkg/get_home_path.h"

#include <mutex>

#ifdef _MSC_VER
#include <shlobj.h>
#include <windows.h>
#else
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace pkg {

static std::mutex m;

boost::filesystem::path get_home_path() {
  std::scoped_lock l{m};

#ifdef _MSC_VER
  char path[MAX_PATH];
  if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path) == S_OK) {
    return path;
  } else {
    throw std::runtime_error{"unable to retrieve home path"};
  }
#else
  auto const pw = getpwuid(getuid());
  if (pw != nullptr) {
    return pw->pw_dir;
  } else {
    throw std::runtime_error{"unable to retrieve home path"};
  }
#endif
}

}  // namespace pkg