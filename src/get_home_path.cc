#include "pkg/get_home_path.h"

#include <mutex>

#ifdef _MSC_VER
#include <userenv.h>
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
  HANDLE process = 0;
  OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &process);

  TCHAR home_dir_buf[MAX_PATH] = {0};
  DWORD size = MAX_PATH;
  GetUserProfileDirectory(process, home_dir_buf, &size);

  CloseHandle(process);

  return std::string{home_dir_buf};
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