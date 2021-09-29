cmake_minimum_required(VERSION 3.16)
project(vadum CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(Threads)

add_subdirectory(deps/cista EXCLUDE_FROM_ALL)
add_subdirectory(deps/utl EXCLUDE_FROM_ALL)
add_subdirectory(deps/fmt EXCLUDE_FROM_ALL)
add_subdirectory(deps/zlib EXCLUDE_FROM_ALL)
add_subdirectory(deps/boost EXCLUDE_FROM_ALL)

file(GLOB_RECURSE vadum-files src/*.cc)
file(GLOB main-file src/main.cc)
list(REMOVE_ITEM vadum-files main-file)
add_library(vadumlib ${vadum-files})
target_include_directories(vadumlib PUBLIC include)
target_link_libraries(vadumlib
        cista
        utl
        fmt
        boost
        boost-filesystem
        boost-system
        ${CMAKE_THREAD_LIBS_INIT}
        )
if (MSVC)
    target_link_libraries(vadumlib Userenv.lib)
endif ()
target_compile_features(vadumlib PUBLIC cxx_std_17)

add_executable(vadum src/main.cc)
target_link_libraries(vadum vadumlib)
target_compile_features(vadum PRIVATE cxx_std_17)
if (${CMAKE_BUILD_TYPE} STREQUAL Release AND NOT MSVC)
    add_custom_command(
            TARGET vadum
            POST_BUILD
            COMMAND ${CMAKE_STRIP} ${CMAKE_CURRENT_BINARY_DIR}/vadum
    )
endif ()

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_dir.h.in
        ${CMAKE_BINARY_DIR}/generated/test_dir.h
)
file(GLOB_RECURSE vadum-test-files test/*_test.cc)
add_executable(vadum-test EXCLUDE_FROM_ALL src/read_deps.cc ${vadum-test-files})
target_include_directories(vadum-test PRIVATE ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(vadum-test vadumlib)
set_property(TARGET vadum-test PROPERTY CXX_STANDARD 17)
