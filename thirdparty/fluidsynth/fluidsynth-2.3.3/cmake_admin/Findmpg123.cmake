#[=======================================================================[.rst:
Findmpg123
-------

Finds the mpg123 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MPG123::libmpg123``
  The mpg123 decoder library
``MPG123::libout123``
  The mpg123 output library
``MPG123::libsyn123``
  The mpg123 signal synthesis library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``mpg123_FOUND``
  True if the package was found.
``mpg123_libmpg123_FOUND``
  True if the decoder library was found.
``mpg123_libout123_FOUND``
  True if the output library was found.
``mpg123_libsyn123_FOUND``
  True if the signal synthesis library was found.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_MPG123 QUIET libmpg123)
pkg_check_modules(PC_OUT123 QUIET libout123)
pkg_check_modules(PC_SYN123 QUIET libsyn123)

# Find the headers and libraries
find_path(
  mpg123_INCLUDE_DIR
  NAMES "mpg123.h"
  HINTS "${PC_MPG123_INCLUDEDIR}")

find_library(
  mpg123_libmpg123_LIBRARY
  NAMES "mpg123"
  HINTS "${PC_MPG123_LIBDIR}")

find_library(
  mpg123_libout123_LIBRARY
  NAMES "out123"
  HINTS "${PC_OUT123_LIBDIR}")

find_library(
  mpg123_libsyn123_LIBRARY
  NAMES "syn123"
  HINTS "${PC_SYN123_LIBDIR}")

# Extract additional flags if pkg-config is available
if(PC_MPG123_FOUND)
  get_target_properties_from_pkg_config("${mpg123_libmpg123_LIBRARY}"
                                        "PC_MPG123" "_libmpg123")
endif()
if(PC_OUT123_FOUND)
  get_target_properties_from_pkg_config("${mpg123_libout123_LIBRARY}"
                                        "PC_OUT123" "_libout123")
endif()
if(PC_SYN123_FOUND)
  get_target_properties_from_pkg_config("${mpg123_libsyn123_LIBRARY}"
                                        "PC_SYN123" "_libsyn123")
endif()

# Mark which component were found
foreach(_component libmpg123 libout123 libsyn123)
  if(mpg123_${_component}_LIBRARY)
    set(mpg123_${_component}_FOUND TRUE)
  else()
    set(mpg123_${_component}_FOUND FALSE)
  endif()
endforeach()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  mpg123
  REQUIRED_VARS "mpg123_libmpg123_LIBRARY" "mpg123_INCLUDE_DIR"
  HANDLE_COMPONENTS)

# Create the targets
foreach(_component libmpg123 libout123 libsyn123)
  if(mpg123_${_component}_FOUND AND NOT TARGET MPG123::${_component})
    add_library(MPG123::${_component})
    set_target_properties(
      MPG123::${_component}
      PROPERTIES IMPORTED_LOCATION "${mpg123_${_component}_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${mpg123_INCLUDE_DIR}"
                 INTERFACE_COMPILE_OPTIONS "${_${_component}_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_${_component}_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES
                 "${_${_component}_link_directories}")
  endif()
endforeach()

mark_as_advanced(mpg123_libmpg123_LIBRARY mpg123_libout123_LIBRARY
                 mpg123_libsyn123_LIBRARY mpg123_INCLUDE_DIR)
