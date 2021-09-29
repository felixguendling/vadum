#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

inline void enable_color_output() {
  auto const stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD console_mode;
  if (GetConsoleMode(stdout_handle, &console_mode) != 0) {
    SetConsoleMode(stdout_handle,
                   console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }
}

#else

inline void enable_color_output() {}

#endif
