#[[Cmake helper function to parse source files from make files
this is to avoid breaking existing make and auto make support
but still have the option to use CMake with only lists at one place]]

cmake_minimum_required(VERSION 3.1)

function(get_library_version OPUS_LIBRARY_VERSION OPUS_LIBRARY_VERSION_MAJOR)
  file(STRINGS configure.ac opus_lt_current_string
       LIMIT_COUNT 1
       REGEX "OPUS_LT_CURRENT=")
  string(REGEX MATCH
               "OPUS_LT_CURRENT=([0-9]*)"
               _
               ${opus_lt_current_string})
  set(OPUS_LT_CURRENT ${CMAKE_MATCH_1})

  file(STRINGS configure.ac opus_lt_revision_string
       LIMIT_COUNT 1
       REGEX "OPUS_LT_REVISION=")
  string(REGEX MATCH
               "OPUS_LT_REVISION=([0-9]*)"
               _
               ${opus_lt_revision_string})
  set(OPUS_LT_REVISION ${CMAKE_MATCH_1})

  file(STRINGS configure.ac opus_lt_age_string
       LIMIT_COUNT 1
       REGEX "OPUS_LT_AGE=")
  string(REGEX MATCH
               "OPUS_LT_AGE=([0-9]*)"
               _
               ${opus_lt_age_string})
  set(OPUS_LT_AGE ${CMAKE_MATCH_1})

  math(EXPR OPUS_LIBRARY_VERSION_MAJOR "${OPUS_LT_CURRENT} - ${OPUS_LT_AGE}")
  set(OPUS_LIBRARY_VERSION_MINOR ${OPUS_LT_AGE})
  set(OPUS_LIBRARY_VERSION_PATCH ${OPUS_LT_REVISION})
  set(
    OPUS_LIBRARY_VERSION
    "${OPUS_LIBRARY_VERSION_MAJOR}.${OPUS_LIBRARY_VERSION_MINOR}.${OPUS_LIBRARY_VERSION_PATCH}"
    PARENT_SCOPE)
  set(OPUS_LIBRARY_VERSION_MAJOR ${OPUS_LIBRARY_VERSION_MAJOR} PARENT_SCOPE)
endfunction()

function(get_package_version PACKAGE_VERSION)
  find_package(Git)
  if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --match "v*"
                    OUTPUT_VARIABLE OPUS_PACKAGE_VERSION)
    if(OPUS_PACKAGE_VERSION)
      string(STRIP ${OPUS_PACKAGE_VERSION}, OPUS_PACKAGE_VERSION)
      string(REPLACE \n
                     ""
                     OPUS_PACKAGE_VERSION
                     ${OPUS_PACKAGE_VERSION})
      string(REPLACE ,
                     ""
                     OPUS_PACKAGE_VERSION
                     ${OPUS_PACKAGE_VERSION})

      string(SUBSTRING ${OPUS_PACKAGE_VERSION}
                       1
                       -1
                       OPUS_PACKAGE_VERSION)
      set(PACKAGE_VERSION ${OPUS_PACKAGE_VERSION} PARENT_SCOPE)
      return()
    endif()
  endif()

  if(EXISTS "${CMAKE_SOURCE_DIR}/package_version")
    # Not a git repo, lets' try to parse it from package_version file if exists
    file(STRINGS package_version opus_package_version_string
         LIMIT_COUNT 1
         REGEX "PACKAGE_VERSION=")
    string(REPLACE "PACKAGE_VERSION="
                   ""
                   opus_package_version_string
                   ${opus_package_version_string})
    string(REPLACE "\""
                   ""
                   opus_package_version_string
                   ${opus_package_version_string})
    set(PACKAGE_VERSION ${opus_package_version_string} PARENT_SCOPE)
    return()
  endif()

  # if all else fails set to 0
  set(PACKAGE_VERSION 0 PARENT_SCOPE)
endfunction()

function(check_and_set_flag NAME FLAG)
  include(CheckCCompilerFlag)
  check_c_compiler_flag(${FLAG} ${NAME}_SUPPORTED)
  if(${NAME}_SUPPORTED)
    add_definitions(${FLAG})
  endif()
endfunction()

function(check_flag NAME FLAG)
  include(CheckCCompilerFlag)
  check_c_compiler_flag(${FLAG} ${NAME}_SUPPORTED)
endfunction()

include(CheckIncludeFile)
# function to check if compiler supports SSE, SSE2, SSE4.1 and AVX if target
# systems may not have SSE support then use OPUS_MAY_HAVE_SSE option if target
# system is guaranteed to have SSE support then OPUS_PRESUME_SSE can be used to
# skip SSE runtime check
function(opus_detect_sse COMPILER_SUPPORT_SIMD)
  message(STATUS "Check SIMD support by compiler")
  check_include_file(xmmintrin.h HAVE_XMMINTRIN_H) # SSE1
  if(HAVE_XMMINTRIN_H)
    if(MSVC)
      # different arch options for 32 and 64 bit target for MSVC
      if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        check_flag(SSE1 /arch:SSE)
      else()
        set(SSE1_SUPPORTED 1 PARENT_SCOPE)
      endif()
    else()
      check_and_set_flag(SSE1 -msse)
    endif()
  else()
    set(SSE1_SUPPORTED 0 PARENT_SCOPE)
  endif()

  check_include_file(emmintrin.h HAVE_EMMINTRIN_H) # SSE2
  if(HAVE_EMMINTRIN_H)
    if(MSVC)
      if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        check_flag(SSE2 /arch:SSE2)
      else()
        set(SSE2_SUPPORTED 1 PARENT_SCOPE)
      endif()
    else()
      check_and_set_flag(SSE2 -msse2)
    endif()
  else()
    set(SSE2_SUPPORTED 0 PARENT_SCOPE)
  endif()

  check_include_file(smmintrin.h HAVE_SMMINTRIN_H) # SSE4.1
  if(HAVE_SMMINTRIN_H)
    if(MSVC)
      if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        check_flag(SSE4_1 /arch:SSE2) # SSE2 and above
      else()
        set(SSE4_1_SUPPORTED 1 PARENT_SCOPE)
      endif()
    else()
      check_and_set_flag(SSE4_1 -msse4.1)
    endif()
  else()
    set(SSE4_1_SUPPORTED 0 PARENT_SCOPE)
  endif()

  check_include_file(immintrin.h HAVE_IMMINTRIN_H) # AVX
  if(HAVE_IMMINTRIN_H)
    if(MSVC)
      check_flag(AVX /arch:AVX)
    else()
      check_and_set_flag(AVX -mavx)
    endif()
  else()
    set(AVX_SUPPORTED 0 PARENT_SCOPE)
  endif()

  if(MSVC) # To avoid warning D9025 of overriding compiler options
    if(AVX_SUPPORTED) # on 64 bit and 32 bits
      add_definitions(/arch:AVX)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4) # if AVX not supported then set SSE flag
      if(SSE4_1_SUPPORTED OR SSE2_SUPPORTED)
        add_definitions(/arch:SSE2)
      elseif(SSE1_SUPPORTED)
        add_definitions(/arch:SSE)
      endif()
    endif()
  endif()

  if(SSE1_SUPPORTED OR SSE2_SUPPORTED OR SSE4_1_SUPPORTED OR AVX_SUPPORTED)
    set(COMPILER_SUPPORT_SIMD 1 PARENT_SCOPE)
  else()
    message(STATUS "No SIMD support in compiler")
  endif()
endfunction()

function(opus_detect_neon COMPILER_SUPPORT_NEON)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "(armv7-a|aarch64)")
    message(STATUS "Check NEON support by compiler")
    check_include_file(arm_neon.h HAVE_ARM_NEON_H)
    if(HAVE_ARM_NEON_H)
      set(COMPILER_SUPPORT_NEON ${HAVE_ARM_NEON_H} PARENT_SCOPE)
    endif()
  endif()
endfunction()

function(opus_supports_cpu_detection RUNTIME_CPU_CAPABILITY_DETECTION)
  if(MSVC)
    check_include_file(intrin.h HAVE_INTRIN_H)
  else()
    check_include_file(cpuid.h HAVE_CPUID_H)
  endif()
  if(HAVE_INTRIN_H OR HAVE_CPUID_H)
    set(RUNTIME_CPU_CAPABILITY_DETECTION 1 PARENT_SCOPE)
  else()
    set(RUNTIME_CPU_CAPABILITY_DETECTION 0 PARENT_SCOPE)
  endif()
endfunction()

function(add_sources_group target group)
  target_sources(${target} PRIVATE ${ARGN})
  source_group(${group} FILES ${ARGN})
endfunction()

function(get_opus_sources SOURCE_GROUP MAKE_FILE SOURCES)
  # read file, each item in list is one group
  file(STRINGS ${MAKE_FILE} opus_sources)

  # add wildcard for regex match
  string(CONCAT SOURCE_GROUP ${SOURCE_GROUP} ".*$")

  # find group
  foreach(val IN LISTS opus_sources)
    if(val MATCHES ${SOURCE_GROUP})
      list(LENGTH val list_length)
      if(${list_length} EQUAL 1)
        # for tests split by '=' and clean up the rest into a list
        string(FIND ${val} "=" index)
        math(EXPR index "${index} + 1")
        string(SUBSTRING ${val}
                         ${index}
                         -1
                         sources)
        string(REPLACE " "
                       ";"
                       sources
                       ${sources})
      else()
        # discard the group
        list(REMOVE_AT val 0)
        set(sources ${val})
      endif()
      break()
    endif()
  endforeach()

  list(LENGTH sources list_length)
  if(${list_length} LESS 1)
    message(
      FATAL_ERROR
        "No files parsed succesfully from ${SOURCE_GROUP} in ${MAKE_FILE}")
  endif()

  # remove trailing whitespaces
  set(list_var "")
  foreach(source ${sources})
    string(STRIP "${source}" source)
    list(APPEND list_var "${source}")
  endforeach()

  set(${SOURCES} ${list_var} PARENT_SCOPE)
endfunction()
