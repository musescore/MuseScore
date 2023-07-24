#[=======================================================================[.rst:
FindSystemd
-------

Finds the Systemd library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Systemd::libsystemd``
  The Systemd library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Systemd_FOUND``
  True if the system has the Systemd library.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_SYSTEMD QUIET libsystemd)

# Find the headers and library
find_path(
  Systemd_INCLUDE_DIR
  NAMES "systemd/sd-daemon.h"
  HINTS "${PC_SYSTEMD_INCLUDEDIR}")

find_library(
  Systemd_LIBRARY
  NAMES "systemd"
  HINTS "${PC_SYSTEMD_LIBDIR}")

# Extract additional flags if pkg-config is available
if(PC_SYSTEMD_FOUND)
  get_target_properties_from_pkg_config("${Systemd_LIBRARY}" "PC_SYSTEMD"
                                        "_systemd")
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Systemd REQUIRED_VARS "Systemd_LIBRARY"
                                                        "Systemd_INCLUDE_DIR")

if(Systemd_FOUND AND NOT TARGET Systemd::libsystemd)
  add_library(Systemd::libsystemd UNKNOWN IMPORTED)
  set_target_properties(
    Systemd::libsystemd
    PROPERTIES IMPORTED_LOCATION "${Systemd_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_systemd_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${Systemd_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_systemd_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_systemd_link_directories}")
endif()

mark_as_advanced(Systemd_INCLUDE_DIR Systemd_LIBRARY)
