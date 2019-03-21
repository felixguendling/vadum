MACRO(SUBDIRLIST result curdir)
FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
SET(dirlist "")
FOREACH(child ${children})
  IF(IS_DIRECTORY ${curdir}/${child})
    LIST(APPEND dirlist ${child})
  ENDIF()
ENDFOREACH()
SET(${result} ${dirlist})
ENDMACRO()

set(pkg-bin "${CMAKE_CURRENT_BINARY_DIR}/dl/pkg")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(pkg-url "https://git.motis-project.de/dl/pkg/-/jobs/45539/artifacts/raw/build-release/pkg")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(pkg-url "https://git.motis-project.de/dl/pkg/-/jobs/45540/artifacts/raw/build-msvc-release/pkg.exe")
else()
  message(STATUS "Not downloading pkg tool. Using pkg from PATH.")
  set(pkg-bin "pkg")
endif()

if (pkg-url)
  if (NOT EXISTS ${pkg-bin})
    message(STATUS "Downloading pkg binary.")
    file(DOWNLOAD "${pkg-url}" ${pkg-bin})
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

SUBDIRLIST(SUBDIRS "${CMAKE_CURRENT_SOURCE_DIR}/deps")
foreach(subdir ${SUBDIRS})
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/deps/${subdir}" EXCLUDE_FROM_ALL)
endforeach()
