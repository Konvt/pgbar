cmake_minimum_required(VERSION 3.5)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12")
  project(pgbar VERSION 0.1.0 LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/Konvt/pgbar"
    DESCRIPTION "A lightweight progress bar for Modern C++.")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "3.9")
  project(pgbar VERSION 0.1.0 LANGUAGES CXX
    DESCRIPTION "A lightweight progress bar for Modern C++.")
else()
  project(pgbar VERSION 0.1.0 LANGUAGES CXX)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

option(PGBAR_INSTALL "Enable install target" ON)
option(PGBAR_PACKAGE "Enable packaging support" ON)
option(PGBAR_BUILD_DEMO "Build demo programs from demo/ directory using Makefile" OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(pgbar INTERFACE)
add_library(pgbar::pgbar ALIAS pgbar)

target_compile_features(pgbar INTERFACE cxx_std_11)
target_include_directories(pgbar INTERFACE
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
  set_property(TARGET pgbar PROPERTY LICENSE "MIT")
endif()

if(PGBAR_INSTALL)
  install(TARGETS pgbar EXPORT pgbarTargets)

  install(EXPORT pgbarTargets
    FILE pgbarTargets.cmake
    NAMESPACE pgbar::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pgbar)

  install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/pgbar)

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pgbarConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pgbar
  )
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfigVersion.cmake
    VERSION 0.1.0
    COMPATIBILITY AnyNewerVersion
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/pgbarConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pgbar)
endif()

if(PGBAR_PACKAGE)
  include(CPack)
  set(CPACK_SOURCE_GENERATOR "TGZ;TXZ")
  set(CPACK_GENERATOR "TGZ;TXZ")
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "pgbar-0.1.0")

  if(EXISTS "${CMAKE_SOURCE_DIR}/.gitignore")
    file(READ "${CMAKE_SOURCE_DIR}/.gitignore" GITIGNORE_CONTENT)
    string(REPLACE "\n" ";" GITIGNORE_CONTENT "${GITIGNORE_CONTENT}")
    foreach(line IN LISTS GITIGNORE_CONTENT)
      if(line STREQUAL "" OR line MATCHES "^#")
        continue()
      endif()
      string(REPLACE "\\" "\\\\" line "${line}")
      string(REPLACE "." "\\." line "${line}")
      string(REPLACE "*" ".*" line "${line}")
      string(REPLACE "+" "\\+" line "${line}")
      list(APPEND CPACK_SOURCE_IGNORE_FILES "${line}")
    endforeach()
  endif()

  list(APPEND CPACK_SOURCE_IGNORE_FILES
    .clangd
    .clang-format
    .git
    .gitignore
    .gitattributes
    .github
    .travis.yml
    .appveyor.yml)
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

  add_custom_target(demo
    DEPENDS ${DEMO_TARGETS}
    COMMENT "Building all demo programs")
endif()

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
  COMMENT "Uninstalling...")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
"if(NOT EXISTS \"\${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt\")
  message(FATAL_ERROR \"Cannot find install_manifest.txt\")
endif()

file(READ \"\${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt\" files)
string(REPLACE \"\\n\" \";\" files \"\${files}\")
foreach(file \${files})
  if(EXISTS \"\${file}\" OR IS_SYMLINK \"\${file}\")
    file(REMOVE \"\${file}\")
    message(STATUS \"Removed: \${file}\")
  else()
    message(STATUS \"Already removed: \${file}\")
  endif()
endforeach()")
