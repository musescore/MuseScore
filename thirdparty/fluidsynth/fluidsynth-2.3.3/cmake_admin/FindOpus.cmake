#[=======================================================================[.rst:
FindOpus
-------

Finds the Opus library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Opus::opus``
  The Opus library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Opus_FOUND``
  True if the system has the Opus library.
``Opus_VERSION``
  The version of the Opus library which was found.

For compatibility with upstream, the following variables are also set:

``OPUS_VERSION_STRING``
``OPUS_VERSION_MAJOR``
``OPUS_VERSION_MINOR``
``OPUS_VERSION_PATCH``
``OPUS_INCLUDE_DIR``
``OPUS_INCLUDE_DIRS``
``OPUS_LIBRARY``
``OPUS_LIBRARIES``
``OPUS_FOUND``

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_OPUS QUIET opus)

# Find the headers and library
find_path(
  Opus_INCLUDE_DIR
  NAMES "opus/opus.h"
  HINTS "${PC_OPUS_INCLUDEDIR}")

find_library(
  Opus_LIBRARY
  NAMES "opus"
  HINTS "${PC_OPUS_LIBDIR}")

# Get the version from pkg-config
if(PC_OPUS_VERSION)
  set(Opus_VERSION "${PC_OPUS_VERSION}")
  set(OPUS_VERSION "${Opus_VERSION}")
  set(OPUS_VERSION_STRING "${Opus_VERSION}")
  string(REPLACE "." ";" _opus_version_list "${Opus_VERSION}")
  list(GET _opus_version_list 0 OPUS_VERSION_MAJOR)
  list(GET _opus_version_list 1 OPUS_VERSION_MINOR)
  list(GET _opus_version_list 2 OPUS_VERSION_PATCH)
else()
  message(STATUS "Unable to get Opus version without pkg-config.")
  set(Opus_VERSION)
  set(OPUS_VERSION)
  set(OPUS_VERSION_STRING)
  set(OPUS_VERSION_MAJOR)
  set(OPUS_VERSION_MINOR)
  set(OPUS_VERSION_PATCH)
endif()

# Extract additional flags if pkg-config is available
if(PC_OPUS_FOUND)
  get_target_properties_from_pkg_config("${Opus_LIBRARY}" "PC_OPUS" "_opus")
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Opus
  REQUIRED_VARS "Opus_LIBRARY" "Opus_INCLUDE_DIR"
  VERSION_VAR "Opus_VERSION")

# Create the target
if(Opus_FOUND AND NOT TARGET Opus::opus)
  set(_opus_include_dirs "${Opus_INCLUDE_DIR}" "${Opus_INCLUDE_DIR}/opus")

  add_library(Opus::opus UNKNOWN IMPORTED)
  set_target_properties(
    Opus::opus
    PROPERTIES IMPORTED_LOCATION "${Opus_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_opus_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${_opus_include_dirs}"
               INTERFACE_LINK_LIBRARIES "${_opus_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_opus_link_directories}")

  # Set additional variables for compatibility with upstream config
  set(OPUS_FOUND TRUE)
  set(OPUS_LIBRARY Opus::opus)
  set(OPUS_LIBRARIES Opus::opus)
  set(OPUS_INCLUDE_DIR "${_opus_include_dirs}")
  set(OPUS_INCLUDE_DIRS "${_opus_include_dirs}")
endif()

mark_as_advanced(Opus_LIBRARY Opus_INCLUDE_DIR)
