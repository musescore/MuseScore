set(OPUS_VERSION 1.5.2)
set(OPUS_VERSION_STRING 1.5.2)
set(OPUS_VERSION_MAJOR 1)
set(OPUS_VERSION_MINOR 5)
set(OPUS_VERSION_PATCH 2)


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was OpusConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set_and_check(OPUS_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
set(OPUS_INCLUDE_DIR ${OPUS_INCLUDE_DIR};${OPUS_INCLUDE_DIR}/opus)
set(OPUS_INCLUDE_DIRS "${PACKAGE_PREFIX_DIR}/include;${PACKAGE_PREFIX_DIR}/include/opus")

include(${CMAKE_CURRENT_LIST_DIR}/OpusTargets.cmake)

set(OPUS_LIBRARY Opus::opus)
set(OPUS_LIBRARIES Opus::opus)

check_required_components(Opus)

set(OPUS_FOUND 1)
