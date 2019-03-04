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
add_executable(pkg ${pkg-files})
set_target_properties(pkg PROPERTIES COMPILE_FLAGS "-std=c++17")
target_include_directories(pkg PRIVATE include)
target_link_libraries(pkg
  utl
  fmt
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)
