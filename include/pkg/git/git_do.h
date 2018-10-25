#pragma once

#define git2_do(...)                                                    \
  if (__VA_ARGS__ != 0) {                                               \
    auto const e = giterr_last();                                       \
    throw std::runtime_error(e ? e->message : "unknown libgit2 error"); \
  }