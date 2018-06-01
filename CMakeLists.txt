project(pkg)
cmake_minimum_required(VERSION 3.10)

if (MSVC)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME ON)
endif()
find_package(Boost COMPONENTS system filesystem REQUIRED)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Shared Library (OFF for Static)")
add_subdirectory(external_lib/libgit2 EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/utl EXCLUDE_FROM_ALL)
include_directories(external_lib/libgit2/include)
include_directories(include)

file(GLOB_RECURSE pkg-files src/*.cc)
add_executable(pkg ${pkg-files})
target_link_libraries(pkg git2 utl ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
