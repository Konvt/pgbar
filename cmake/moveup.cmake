# DIR: target DIR
# INCLUDE_DOTFILES: boolean
if(NOT IS_DIRECTORY "${DIR}")
  message(FATAL_ERROR "moveup: directory not found: ${DIR}")
endif()

if(NOT DEFINED INCLUDE_DOTFILES)
  set(INCLUDE_DOTFILES OFF)
endif()

get_filename_component(parent "${DIR}" DIRECTORY)
if(NOT parent)
  message(FATAL_ERROR "moveup: cannot determine parent of: ${DIR}")
endif()

if(INCLUDE_DOTFILES)
  file(GLOB children RELATIVE "${DIR}" "${DIR}/*" "${DIR}/.*")
else()
  file(GLOB children RELATIVE "${DIR}" "${DIR}/*")
endif()

foreach(child IN LISTS children)
  if(child STREQUAL "." OR child STREQUAL "..")
    continue()
  endif()

  set(src "${DIR}/${child}")
  set(dst "${parent}/${child}")

  if(EXISTS "${dst}")
    if(IS_DIRECTORY "${dst}")
      file(REMOVE_RECURSE "${dst}")
    else()
      file(REMOVE "${dst}")
    endif()
  endif()

  execute_process(
    COMMAND ${CMAKE_COMMAND} -E rename "${src}" "${dst}"
    RESULT_VARIABLE _rv
    OUTPUT_QUIET ERROR_QUIET)
  if(NOT _rv EQUAL 0)
    if(IS_DIRECTORY "${src}")
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${src}" "${dst}" RESULT_VARIABLE _rv2 OUTPUT_QUIET ERROR_QUIET)
      if(_rv2 EQUAL 0)
        execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory "${src}" OUTPUT_QUIET ERROR_QUIET)
      else()
        message(FATAL_ERROR "moveup: failed to copy_directory '${src}' -> '${dst}'")
      endif()
    else()
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${src}" "${dst}" RESULT_VARIABLE _rv2 OUTPUT_QUIET ERROR_QUIET)
      if(_rv2 EQUAL 0)
        execute_process(COMMAND ${CMAKE_COMMAND} -E remove "${src}" OUTPUT_QUIET ERROR_QUIET)
      else()
        message(FATAL_ERROR "moveup: failed to copy '${src}' -> '${dst}'")
      endif()
    endif()
  endif()
endforeach()
