project(example)
cmake_minimum_required(VERSION 3.11)

add_subdirectory(deps EXCLUDE_FROM_ALL)

add_executable(example src/main.cc)
target_link_libraries(example repo1 repo2)
