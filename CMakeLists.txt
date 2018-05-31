project(pkg)
cmake_minimum_required(VERSION 3.10)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Shared Library (OFF for Static)")
add_subdirectory(external_lib/libgit2 EXCLUDE_FROM_ALL)
include_directories(external_lib/libgit2/include)
include_directories(include)

file(GLOB_RECURSE pkg-files src/*.cc)
add_executable(pkg ${pkg-files})
target_link_libraries(pkg git2)
