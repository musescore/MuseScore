#[=======================================================================[.rst:
FindInstPatch
-------

Finds the InstPatch library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``InstPatch::libinstpatch``
  The InstPatch library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``InstPatch_FOUND``
  True if the system has the InstPatch library.
``InstPatch_VERSION``
  The version of the InstPatch library which was found.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_INSTPATCH QUIET libinstpatch-1.0)

# Find the headers and library
find_path(
  InstPatch_INCLUDE_DIR
  NAMES "libinstpatch/libinstpatch.h"
  HINTS "${PC_INSTPATCH_INCLUDEDIR}"
  PATH_SUFFIXES "libinstpatch-1" "libinstpatch-2")

find_library(
  InstPatch_LIBRARY
  NAMES "instpatch-1.0"
  HINTS "${PC_INSTPATCH_LIBDIR}")

# Get version from pkg-config or read the config header
if(PC_INSTPATCH_VERSION)
  set(InstPatch_VERSION "${PC_INSTPATCH_VERSION}")
elseif(InstPatch_INCLUDE_DIR)
  file(READ "${InstPatch_INCLUDE_DIR}/libinstpatch/version.h" _version_h)
  string(REGEX MATCH
               "#define[ \t]+IPATCH_VERSION[ \t]+\"([0-9]+.[0-9]+.[0-9]+)\""
               _instpatch_version_re "${_version_h}")
  set(InstPatch_VERSION "${CMAKE_MATCH_1}")
endif()

# Handle transitive dependencies
if(PC_INSTPATCH_FOUND)
  get_target_properties_from_pkg_config("${InstPatch_LIBRARY}" "PC_INSTPATCH"
                                        "_instpatch")
else()
  if(NOT TARGET GLib2::gobject-2
     OR NOT TARGET GLib2::gthread-2
     OR NOT TARGET GLib2::glib-2)
    find_package(GLib2 QUIET)
  endif()
  if(NOT TARGET SndFile::sndfile)
    find_package(SndFile QUIET)
  endif()
  set(_instpatch_link_libraries "GLib2::gobject-2" "GLib2::gthread-2"
                                "GLib2::glib-2" "SndFile::sndfile")
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  InstPatch
  REQUIRED_VARS "InstPatch_LIBRARY" "InstPatch_INCLUDE_DIR"
  VERSION_VAR "InstPatch_VERSION")

if(InstPatch_FOUND AND NOT TARGET InstPatch::libinstpatch)
  add_library(InstPatch::libinstpatch UNKNOWN IMPORTED)
  set_target_properties(
    InstPatch::libinstpatch
    PROPERTIES IMPORTED_LOCATION "${InstPatch_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_instpatch_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${InstPatch_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_instpatch_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_instpatch_link_directories}")
endif()

mark_as_advanced(InstPatch_INCLUDE_DIR InstPatch_LIBRARY)
