#[=======================================================================[.rst:
FindPipeWire
-------

Finds the PipeWire library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``PipeWire::PipeWire``
  The PipeWire library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PipeWire_FOUND``
  True if the system has the PipeWire library.
``PipeWire_VERSION``
  The version of the PipeWire library which was found.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_PIPEWIRE QUIET libpipewire-0.3)

# Find the headers and library
find_path(
  PipeWire_INCLUDE_DIR
  NAMES "pipewire/pipewire.h"
  HINTS "${PC_PIPEWIRE_INCLUDEDIR}"
  PATH_SUFFIXES "pipewire-0.3")

find_path(
  Spa_INCLUDE_DIR
  NAMES "spa/support/plugin.h"
  HINTS "${PC_PIPEWIRE_INCLUDEDIR}"
  PATH_SUFFIXES "spa-0.2")

find_library(
  PipeWire_LIBRARY
  NAMES "pipewire-0.3"
  HINTS "${PC_PIPEWIRE_LIBDIR}")

# Get version from pkg-config or read the version header
if(PC_PIPEWIRE_VERSION)
  set(PipeWire_VERSION "${PC_PIPEWIRE_VERSION}")
elseif(PipeWire_INCLUDE_DIR)
  file(READ "${PipeWire_INCLUDE_DIR}/pipewire/version.h" _version_h)
  string(REGEX MATCH "#define[ \t]+PW_MAJOR[ \t]+([0-9]+)" _pipewire_major_re
               "${_version_h}")
  set(_pipewire_major "${CMAKE_MATCH_1}")
  string(REGEX MATCH "#define[ \t]+PW_MINOR[ \t]+([0-9]+)" _pipewire_minor_re
               "${_version_h}")
  set(_pipewire_minor "${CMAKE_MATCH_1}")
  string(REGEX MATCH "#define[ \t]+PW_MICRO[ \t]+([0-9]+)" _pipewire_patch_re
               "${_version_h}")
  set(_pipewire_patch "${CMAKE_MATCH_1}")
  if(_pipewire_major_re
     AND _pipewire_minor_re
     AND _pipewire_patch_re)
    set(pipewire_VERSION
        "${_pipewire_major}.${_pipewire_minor}.${_pipewire_patch}")
  endif()
endif()

# Extract additional flags if pkg-config is available
if(PC_PIPEWIRE_FOUND)
  get_target_properties_from_pkg_config("${PipeWire_LIBRARY}" "PC_PIPEWIRE"
                                        "_pipewire")
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  PipeWire
  REQUIRED_VARS "PipeWire_LIBRARY" "PipeWire_INCLUDE_DIR" "Spa_INCLUDE_DIR"
  VERSION_VAR "PipeWire_VERSION")

# Create the target
if(PipeWire_FOUND AND NOT TARGET PipeWire::PipeWire)
  add_library(PipeWire::PipeWire UNKNOWN IMPORTED)
  set_target_properties(
    PipeWire::PipeWire
    PROPERTIES IMPORTED_LOCATION "${PipeWire_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_pipewire_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES
               "${PipeWire_INCLUDE_DIR};${Spa_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_pipewire_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_pipewire_link_directories}")
endif()

mark_as_advanced(PipeWire_INCLUDE_DIR Spa_INCLUDE_DIR PipeWire_LIBRARY)
