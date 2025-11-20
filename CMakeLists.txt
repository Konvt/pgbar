cmake_minimum_required(VERSION 3.10)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(PGBAR_IS_TOP_LEVEL ON)
else()
  set(PGBAR_IS_TOP_LEVEL OFF)
endif()

set(
  PGBAR_PROJECT
  pgbar
  VERSION 0.1.0
  LANGUAGES CXX
  DESCRIPTION "A lightweight progress bar for Modern C++.")
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12")
  list(APPEND PGBAR_PROJECT HOMEPAGE_URL "https://github.com/Konvt/pgbar")
endif()

project(${PGBAR_PROJECT})

option(PGBAR_INSTALL "Enable install target" ${PGBAR_IS_TOP_LEVEL})
option(PGBAR_PACKAGE "Enable packaging support" ${PGBAR_IS_TOP_LEVEL})
option(PGBAR_BUILD_DEMO "Build demo programs from demo/ directory using Makefile" OFF)

include(CMakePackageConfigHelpers)
if(PGBAR_IS_TOP_LEVEL)
  include(GNUInstallDirs)
endif()

add_library(pgbar INTERFACE)
add_library(pgbar::pgbar ALIAS pgbar)

if(PGBAR_IS_TOP_LEVEL)
  target_compile_features(pgbar INTERFACE cxx_std_11)
  set_target_properties(pgbar PROPERTIES CXX_EXTENSIONS OFF)
endif()
target_include_directories(
  pgbar INTERFACE
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
  set_property(TARGET pgbar PROPERTY LICENSE "MIT")
endif()

if(PGBAR_INSTALL)
  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pgbarConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pgbar)
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pgbar.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/pgbar.pc
    @ONLY)

  install(TARGETS pgbar EXPORT pgbarTargets)
  install(
    EXPORT pgbarTargets
    FILE pgbarTargets.cmake
    NAMESPACE pgbar::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pgbar)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/pgbar.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
  install(FILES README.md DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/docs/pgbar)
  install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/pgbar)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pgbar)
  install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PATTERN "Makefile" EXCLUDE)
  install(
    DIRECTORY docs/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/docs/pgbar)
endif()

if(PGBAR_PACKAGE)
  # Only read the.gitignore in the root directory
  if(EXISTS "${PROJECT_SOURCE_DIR}/.gitignore")
    file(READ ".gitignore" DOT_GITIGNORE)
    string(REPLACE ";" "RANDOMSEQUENCE" DOT_GITIGNORE "${DOT_GITIGNORE}")
    string(REPLACE "\n" ";" DOT_GITIGNORE "${DOT_GITIGNORE}")
    string(REPLACE "RANDOMSEQUENCE" "\\;" DOT_GITIGNORE "${DOT_GITIGNORE}")
    foreach(IGNORE_LINE ${DOT_GITIGNORE})
      if(NOT IGNORE_LINE OR IGNORE_LINE MATCHES "^#")
        continue()
      endif()

      string(REPLACE "\\" "\\\\" IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "." "\\\\." IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "*" ".*" IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "+" "\\\\+" IGNORE_LINE "${IGNORE_LINE}")
      list(APPEND CPACK_IGNORE_FILES "${IGNORE_LINE}")
      list(APPEND CPACK_SOURCE_IGNORE_FILES "${IGNORE_LINE}")
    endforeach()
  endif()

  list(
    APPEND CPACK_IGNORE_FILES
    "(^|/)\\\\.[^/]+$"
    "pgbar\\\\.pc"
    "pgbar\\\\.pc\\\\.in"
    "include/Makefile"
    "misc/"
    "images/")
  list(APPEND CPACK_SOURCE_IGNORE_FILES "misc/")
  set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
  set(CPACK_SOURCE_GENERATOR "ZIP")
  set(CPACK_GENERATOR "ZIP")

  include(CPack)
endif()

if(PGBAR_BUILD_DEMO)
  file(GLOB DEMO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/demo/*.cpp")

  set(DEMO_TARGETS)
  foreach(DEMO_FILE ${DEMO_SOURCES})
    get_filename_component(DEMO_NAME ${DEMO_FILE} NAME_WE)
    set(TARGET_NAME "demo_${DEMO_NAME}")

    add_custom_target(${TARGET_NAME}
      COMMAND ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_CURRENT_SOURCE_DIR}/demo ${DEMO_NAME}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/demo
      COMMENT "Building demo ${DEMO_NAME} using Makefile"
      USES_TERMINAL)
    list(APPEND DEMO_TARGETS ${TARGET_NAME})
  endforeach()

  # Manually inject to trigger the generation of compile commands.json.
  add_executable(main "demo/main.cpp")
  target_link_libraries(main PRIVATE pgbar)
  set_target_properties(main PROPERTIES EXCLUDE_FROM_ALL TRUE)

  add_custom_target(demo
    DEPENDS ${DEMO_TARGETS}
    COMMENT "Building all demo programs")
endif()

if(NOT TARGET uninstall)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake
    @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake
    COMMENT "Uninstalling files from install_manifest.txt")
endif()
