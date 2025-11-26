cmake_minimum_required(VERSION 3.10)

set(PGBAR_IS_TOP_LEVEL OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(PGBAR_IS_TOP_LEVEL ON)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/parse_version.cmake)
parse_version(PGBAR_VERSION PGBAR_STAGE)

set(PGBAR_PROJECT
    pgbar
    VERSION ${PGBAR_VERSION}
    LANGUAGES CXX
    DESCRIPTION "A lightweight progress bar for Modern C++.")
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.12")
  list(APPEND PGBAR_PROJECT HOMEPAGE_URL "https://github.com/Konvt/pgbar")
endif()

project(${PGBAR_PROJECT})

option(PGBAR_INSTALL "Enable install target" ${PGBAR_IS_TOP_LEVEL})
option(PGBAR_PACKAGE "Enable packaging support" ${PGBAR_IS_TOP_LEVEL})
option(PGBAR_BUILD_DEMO "Build demo programs from demo/ directory using Makefile" OFF)

include(CMakePackageConfigHelpers)

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

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
  set_property(TARGET pgbar PROPERTY LICENSE "MIT")
endif()

if(PGBAR_INSTALL)
  include(GNUInstallDirs)
  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pgbarConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pgbar.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/pgbar.pc
    @ONLY)

  install(TARGETS pgbar EXPORT pgbarTargets)
  install(EXPORT pgbarTargets
          FILE pgbarTargets.cmake
          NAMESPACE pgbar::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pgbar.pc
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
  install(FILES README.md DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/docs/${PROJECT_NAME})
  install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfig.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfigVersion.cmake
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
  install(DIRECTORY include/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
          PATTERN "Makefile" EXCLUDE)
  install(DIRECTORY docs/
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/docs/${PROJECT_NAME})
endif()

if(PGBAR_PACKAGE)
  if(NOT DEFINED PGBAR_PKG_NAME OR PGBAR_PKG_NAME STREQUAL "")
    set(PGBAR_PKG_NAME "${PROJECT_NAME}-${PGBAR_VERSION}-${PGBAR_STAGE}")
  endif()

  set(PGBAR_PKG_ROOT "${CMAKE_CURRENT_BINARY_DIR}/_pkg_root")
  set(PGBAR_PKG_DIR "${PGBAR_PKG_ROOT}/${PROJECT_NAME}")
  set(PGBAR_PKG_FMT "zip")
  set(PGBAR_PKG_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PGBAR_PKG_NAME}.${PGBAR_PKG_FMT}")

  set(PGBAR_INS_STAMP "installed")
  add_custom_command(
    OUTPUT "${PGBAR_PKG_ROOT}/${PGBAR_INS_STAMP}"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PGBAR_PKG_DIR}
    COMMAND ${CMAKE_COMMAND} -E env
            DESTDIR=${PGBAR_PKG_ROOT}
            ${CMAKE_COMMAND} --install ${CMAKE_CURRENT_BINARY_DIR} --prefix "/${PROJECT_NAME}"
    COMMAND ${CMAKE_COMMAND} -E touch "${PGBAR_PKG_ROOT}/${PGBAR_INS_STAMP}"
    COMMENT "Installing ${PROJECT_NAME} into staging directory")

  # Adjust the directory tree structure to conform to the release package structure
  # instead of the GNU installation structure
  set(PGBAR_MOVEUP "${CMAKE_SOURCE_DIR}/cmake/moveup.cmake")
  set(PGBAR_ADJT_STAMP "adjusted")
  add_custom_command(
    OUTPUT "${PGBAR_PKG_ROOT}/${PGBAR_ADJT_STAMP}"
    DEPENDS "${PGBAR_PKG_ROOT}/${PGBAR_INS_STAMP}"

    COMMAND ${CMAKE_COMMAND} -DDIR="${PGBAR_PKG_ROOT}/${PROJECT_NAME}/${CMAKE_INSTALL_DATAROOTDIR}/docs/${PROJECT_NAME}" -P ${PGBAR_MOVEUP}
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${PGBAR_PKG_ROOT}/${PROJECT_NAME}/${CMAKE_INSTALL_DATAROOTDIR}/docs/${PROJECT_NAME}"

    COMMAND ${CMAKE_COMMAND} -DDIR="${PGBAR_PKG_ROOT}/${PROJECT_NAME}/${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}" -P ${PGBAR_MOVEUP}
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${PGBAR_PKG_ROOT}/${PROJECT_NAME}/${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}"

    COMMAND ${CMAKE_COMMAND} -DDIR="${PGBAR_PKG_ROOT}/${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" -P ${PGBAR_MOVEUP}
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${PGBAR_PKG_ROOT}/${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"

    COMMAND ${CMAKE_COMMAND} -E touch "${PGBAR_PKG_ROOT}/${PGBAR_ADJT_STAMP}"
    COMMENT "Adjusting directory tree")

  add_custom_command(
    OUTPUT ${PGBAR_PKG_FILE}
    DEPENDS "${PGBAR_PKG_ROOT}/${PGBAR_ADJT_STAMP}"
    COMMAND ${CMAKE_COMMAND} -E tar
            cf ${PGBAR_PKG_FILE} --format=${PGBAR_PKG_FMT}
            -- ${PROJECT_NAME}
    WORKING_DIRECTORY ${PGBAR_PKG_ROOT}
    COMMENT "Packaging ${PROJECT_NAME}")
  add_custom_target(pack DEPENDS ${PGBAR_PKG_FILE})
endif()

if(PGBAR_BUILD_DEMO)
  file(GLOB DEMO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/demo/*.cpp")

  set(PGBAR_DEMO)
  foreach(DEMO_FILE ${DEMO_SOURCES})
    get_filename_component(DEMO_NAME ${DEMO_FILE} NAME_WE)
    set(PGBAR_DEMO_NAME "demo_${DEMO_NAME}")

    add_custom_target(
      ${PGBAR_DEMO_NAME}
      COMMAND ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_CURRENT_SOURCE_DIR}/demo ${DEMO_NAME}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/demo
      COMMENT "Building demo ${DEMO_NAME} using Makefile"
      USES_TERMINAL)
    list(APPEND PGBAR_DEMO ${PGBAR_DEMO_NAME})
  endforeach()

  # Manually inject to trigger the generation of compile commands.json.
  add_executable(iterate "demo/iterate.cpp")
  target_link_libraries(iterate PRIVATE pgbar)
  set_target_properties(iterate PROPERTIES EXCLUDE_FROM_ALL TRUE)

  add_custom_target(
    demo
    DEPENDS ${PGBAR_DEMO}
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
