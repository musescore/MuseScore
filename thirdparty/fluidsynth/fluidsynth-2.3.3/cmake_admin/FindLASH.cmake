#[=======================================================================[.rst:
FindLASH
-------

Finds the LASH library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``LASH::LASH``
  The LASH library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``LASH_FOUND``
  True if the system has the LASH library.
``LASH_VERSION``
  The version of the LASH library which was found.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_LASH QUIET lash-1.0)

# Find the headers and library
find_path(
  LASH_INCLUDE_DIR
  NAMES "lash/lash.h"
  HINTS "${PC_LASH_INCLUDEDIR}"
  PATH_SUFFIXES "lash-1.0")

find_library(
  LASH_LIBRARY
  NAMES "lash"
  HINTS "${PC_LASH_LIBDIR}")

# Get version from pkg-config or read the config header
if(PC_LASH_VERSION)
  set(LASH_VERSION "${PC_LASH_VERSION}")
else()
  if(NOT LASH_FIND_VERSION)
    set(_assumed_version "0.5.0")
  else()
    set(_assumed_version "${LASH_FIND_VERSION}")
  endif()
  message(
    NOTICE
    "LASH does not expose its version outside of pkg-config. Assuming version ${_assumed_version}, expect failure if your version is lower."
  )
  set(LASH_VERSION ${_assumed_version})
endif()

# Handle transitive dependencies
if(PC_LASH_FOUND)
  get_target_properties_from_pkg_config("${LASH_LIBRARY}" "PC_LASH" "_lash")
else()
  set(_lash_link_libraries "Jack::Jack" "Threads::Threads" "ALSA::ALSA" "uuid")
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LASH
  REQUIRED_VARS "LASH_LIBRARY" "LASH_INCLUDE_DIR"
  VERSION_VAR "LASH_VERSION")

if(LASH_FOUND AND NOT TARGET LASH::LASH)
  add_library(LASH::LASH UNKNOWN IMPORTED)
  set_target_properties(
    LASH::LASH
    PROPERTIES IMPORTED_LOCATION "${LASH_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_lash_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${LASH_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_lash_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_lash_link_directories}")
endif()

mark_as_advanced(LASH_INCLUDE_DIR LASH_LIBRARY)
