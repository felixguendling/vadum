project(pkg)
cmake_minimum_required(VERSION 3.10)

find_package(Threads)

if (MSVC)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME ON)
endif()
find_package(Boost COMPONENTS system filesystem REQUIRED)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

add_subdirectory(external_lib/utl EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/fmt EXCLUDE_FROM_ALL)

file(GLOB_RECURSE pkg-files src/*.cc)
file(GLOB main-file src/main.cc)
list(REMOVE_ITEM pkg-files main-file)
add_library(pkglib ${pkg-files})
target_include_directories(pkglib PUBLIC include)
target_link_libraries(pkglib
  utl
  fmt
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROPERTYTREE_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(pkg src/main.cc)
target_link_libraries(pkg pkglib)

file(GLOB_RECURSE pkg-test-files test/*_test.cc)
add_executable(pkg-test src/read_deps.cc ${pkg-test-files})
target_link_libraries(pkg-test pkglib)