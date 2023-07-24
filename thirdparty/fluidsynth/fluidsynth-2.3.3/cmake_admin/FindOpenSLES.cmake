#[=======================================================================[.rst:
FindOpenSLES
-------

Finds the OpenSLES library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``OpenSLES::libOpenSLES``
  The OpenSLES library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``OpenSLES_FOUND``
  True if the system has the OpenSLES library.

#]=======================================================================]

# Find the headers and library
find_path(OpenSLES_INCLUDE_DIR NAMES "SLES/OpenSLES.h")

find_library(OpenSLES_LIBRARY NAMES "OpenSLES")

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenSLES REQUIRED_VARS "OpenSLES_LIBRARY"
                                                         "OpenSLES_INCLUDE_DIR")

# Create the target
if(OpenSLES_FOUND AND NOT TARGET OpenSLES::OpenSLES)
  add_library(OpenSLES::OpenSLES UNKNOWN IMPORTED)
  set_target_properties(
    OpenSLES::OpenSLES
    PROPERTIES IMPORTED_LOCATION "${OpenSLES_LIBRARY}"
               INTERFACE_INCLUDE_DIRECTORIES "${OpenSLES_INCLUDE_DIR}")
endif()

mark_as_advanced(OpenSLES_INCLUDE_DIR OpenSLES_LIBRARY)
