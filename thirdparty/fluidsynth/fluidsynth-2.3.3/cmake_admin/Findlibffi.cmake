#[=======================================================================[.rst:
Findlibffi
-------

Finds the libffi library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``libffi``
  The ffi library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``libffi_FOUND``
  True if the system has the Ffi library.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_LIBFFI QUIET libffi)

# Find the headers and library
find_path(
  libffi_INCLUDE_DIR
  NAMES "ffi.h"
  HINTS "${PC_LIBFFI_INCLUDE_DIR}")

find_library(
  libffi_LIBRARY
  NAMES "ffi" "libffi"
  HINTS "${PC_FFI_LIBDIR}")

# Extract additional flags if pkg-config is available
if(PC_FFI_FOUND)
  get_target_properties_from_pkg_config("${libffi_LIBRARY}" "PC_FFI" "_libffi")
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libffi REQUIRED_VARS "libffi_LIBRARY"
                                                       "libffi_INCLUDE_DIR")

if(libffi_FOUND AND NOT TARGET libffi)
  add_library(libffi UNKNOWN IMPORTED)
  set_target_properties(
    libffi
    PROPERTIES IMPORTED_LOCATION "${libffi_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_libffi_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${libffi_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_libffi_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_libffi_link_directories}")
endif()

mark_as_advanced(libffi_LIBRARY libffi_INCLUDE_DIR)
