#[=======================================================================[.rst:
FindFLAC
-------

Finds the FLAC library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``FLAC::FLAC``
  The FLAC C library.
``FLAC::FLAC++``
  The FLAC C++ library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``FLAC_FOUND``
  True if both libraries were found.
``FLAC_FLAC_FOUND``
  True if the C library was found.
``FLAC_FLAC++_FOUND``
  True if the C++ library was found..

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_FLAC QUIET flac)
pkg_check_modules(PC_FLAC++ QUIET flac++)

# Find the headers and libraries
find_path(
  FLAC_INCLUDE_DIR
  NAMES "FLAC/all.h"
  HINTS "PC_FLAC_INCLUDEDIR")

find_path(
  FLAC++_INCLUDE_DIR
  NAMES "FLAC++/all.h"
  HINTS "PC_FLAC++_INCLUDEDIR")

find_library(
  FLAC_LIBRARY
  NAMES "FLAC"
  HINTS "${PC_FLAC_LIBDIR}")

find_library(
  FLAC++_LIBRARY
  NAMES "FLAC++"
  HINTS "${PC_FLAC++_LIBDIR}")

# Handle transitive dependencies
if(PC_FLAC_FOUND)
  get_target_properties_from_pkg_config("${FLAC_LIBRARY}" "PC_FLAC" "_flac")
else()
  if(NOT TARGET "Ogg::ogg")
    find_package(Ogg QUIET)
  endif()
  set(_flac_link_libraries "Ogg::ogg" ${MATH_LIBRARY})
endif()

if(PC_FLAC++_FOUND)
  get_target_properties_from_pkg_config("${FLAC++_LIBRARY}" "PC_FLAC++"
                                        "_flac++")
else()
  set(_flac++_link_libraries "FLAC::FLAC")
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  FLAC REQUIRED_VARS "FLAC_LIBRARY" "FLAC_INCLUDE_DIR" "FLAC++_LIBRARY"
                     "FLAC++_INCLUDE_DIR")

# Create the target
if(FLAC_FOUND AND NOT TARGET FLAC::FLAC)
  add_library(FLAC::FLAC UNKNOWN IMPORTED)
  set_target_properties(
    FLAC::FLAC
    PROPERTIES IMPORTED_LOCATION "${FLAC_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_flac_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${FLAC_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_flac_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_flac_link_directories}")
  set(FLAC_FLAC_FOUND TRUE)
endif()

if(FLAC_FOUND AND NOT TARGET FLAC::FLAC++)
  add_library(FLAC::FLAC++ UNKNOWN IMPORTED)
  set_target_properties(
    FLAC::FLAC++
    PROPERTIES IMPORTED_LOCATION "${FLAC++_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_flac++_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${FLAC++_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_flac++_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_flac++_link_directories}")
  set(FLAC_FLAC++_FOUND TRUE)
endif()

mark_as_advanced(FLAC_LIBRARY FLAC_INCLUDE_DIR FLAC++_LIBRARY
                 FLAC++_INCLUDE_DIR)
