set(pkg-bin "${CMAKE_CURRENT_BINARY_DIR}/dl/pkg")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(pkg-url "pkg?job=linux-release")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(pkg-url "pkg.exe?job=windows-release")
else()
  message(STATUS "Not downloading 'pgk' tool. Using pkg from PATH.")
  set(pkg-bin "pkg")
endif()

if (pkg-url)
  if (NOT EXISTS ${pkg-bin})
    message(STATUS "Downloading pkg binary.")
    file(DOWNLOAD "http://dl.motis-project.de/pkg" ${pkg-bin})
    if (UNIX)
      execute_process(COMMAND chmod +x ${pkg-bin})
    endif()
  else()
    message(STATUS "Pkg binary located in project.")
  endif()
endif()

message(STATUS ${pkg-bin})
execute_process(
  COMMAND ${pkg-bin} -l
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)