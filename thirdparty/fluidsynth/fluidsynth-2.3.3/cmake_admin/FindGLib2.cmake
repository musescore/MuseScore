#[=======================================================================[.rst:
FindGLib2
-------

Finds the GLib2 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``GLib2::glib-2``
  The GLib core library
``Glib2::gthread-2``
  The GLib threading library
``Glib2::gmodule-2``
  The GLib dynamic loader library
``Glib2::gobject-2``
  The GLib class type system library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``GLib2_FOUND``
  True if glib-2 and gthread-2 have been found.
``GLib2_VERSION``
  The version of the GLib2 library which was found.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_GLIB2 QUIET glib-2.0)
pkg_check_modules(PC_GTHREAD2 QUIET gthread-2.0)
pkg_check_modules(PC_GMODULE2 QUIET gmodule-2.0)
pkg_check_modules(PC_GMODULE2 QUIET gobject-2.0)

# Find the headers and libraries
find_path(
  GLib2_INCLUDE_DIR
  NAMES "glib.h"
  HINTS "${PC_GLIB2_INCLUDEDIR}"
  PATH_SUFFIXES "glib-2.0")

find_library(
  GLib2_glib-2_LIBRARY
  NAMES "glib-2.0"
  HINTS "${PC_GLIB2_LIBDIR}")

find_library(
  GLib2_gthread-2_LIBRARY
  NAMES "gthread-2.0"
  HINTS "${PC_GTHREAD2_LIBDIR}")

find_library(
  GLib2_gmodule-2_LIBRARY
  NAMES "gmodule-2.0"
  HINTS "${PC_GMODULE2_LIBDIR}")

find_library(
  GLib2_gobject-2_LIBRARY
  NAMES "gobject-2.0"
  HINTS "${PC_GOBJECT2_LIBDIR}")

# GLib stores its config in lib/glib-2.0/include
get_filename_component(_glib2_libdir "${GLib2_glib-2_LIBRARY}" PATH)
find_path(
  _glib2_config_header "glibconfig.h"
  PATH_SUFFIXES "glib-2.0/include"
  HINTS "${PC_GLIB2_INCLUDEDIR}" "${_glib2_libdir}")

set(GLib2_INCLUDE_DIRS "${GLib2_INCLUDE_DIR}" "${_glib2_config_header}")

# Get version from pkg-config or read the config header
if(PC_GLIB2_VERSION)
  set(GLib2_VERSION "${PC_GLIB2_VERSION}")
elseif(_glib2_config_header)
  file(READ "${_glib2_config_header}/glibconfig.h" _glib2_config_h)
  string(REGEX MATCH "#define[ \t]+GLIB_MAJOR_VERSION[ \t]+([0-9]+)"
               _glib2_major_re "${_glib2_config_h}")
  set(_glib2_major "${CMAKE_MATCH_1}")
  string(REGEX MATCH "#define[ \t]+GLIB_MINOR_VERSION[ \t]+([0-9]+)"
               _glib2_minor_re "${_glib2_config_h}")
  set(_glib2_minor "${CMAKE_MATCH_1}")
  string(REGEX MATCH "#define[ \t]+GLIB_MICRO_VERSION[ \t]+([0-9]+)"
               _glib2_patch_re "${_glib2_config_h}")
  set(_glib2_patch "${CMAKE_MATCH_1}")
  if(_glib2_major_re
     AND _glib2_minor_re
     AND _glib2_patch_re)
    set(GLib2_VERSION "${_glib2_major}.${_glib2_minor}.${_glib2_patch}")
  endif()
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  GLib2
  REQUIRED_VARS "GLib2_glib-2_LIBRARY" "GLib2_gthread-2_LIBRARY"
                "GLib2_INCLUDE_DIRS"
  VERSION_VAR "GLib2_VERSION")

# Create the targets
if(GLib2_glib-2_LIBRARY AND NOT TARGET GLib2::glib-2)
  # Handle transitive dependencies
  if(PC_GLIB2_FOUND)
    get_target_properties_from_pkg_config("${GLib2_glib-2_LIBRARY}" "PC_GLIB2"
                                          "_glib2")
  else()
    find_package(Intl QUIET)
    find_package(Iconv QUIET)
    list(APPEND _glib2_link_libraries "Intl::Intl" "Iconv::Iconv")
    if(WIN32)
      list(APPEND _glib2_link_libraries "ws2_32" "winmm")
    else()
      list(APPEND _glib2_link_libraries "Threads::Threads")
    endif()
    list(APPEND _glib2_link_libraries ${MATH_LIBRARY})

    # Glib can link to either PCRE 1 or 2
    find_library(
      _pcre2_8bit_library
      NAMES "pcre2-8"
      HINTS "${PC_GLIB2_LIBDIR}")
    if(_pcre2_8bit_library)
      include(CheckCSourceCompiles)
      set(_backup_includes ${CMAKE_REQUIRED_INCLUDES})
      set(_backup_libraries ${CMAKE_REQUIRED_LIBRARIES})
      set(_backup_libdir ${CMAKE_REQUIRED_LIBRARIES})
      set(CMAKE_REQUIRED_INCLUDES "${GLib2_INCLUDE_DIRS}")
      set(CMAKE_REQUIRED_LIBRARIES
          "${GLib2_glib-2_LIBRARY}" "${_glib2_link_libraries}"
          "${_pcre2_8bit_library}")
      check_c_source_compiles(
        "#include <glib.h>
    int main(){
      g_regex_error_quark();
    }"
        GLIB2_USES_PCRE2)
      set(CMAKE_REQUIRED_INCLUDES ${_backup_includes})
      set(CMAKE_REQUIRED_LIBRARIES ${_backup_libraries})
    endif()
    if(GLIB2_USES_PCRE2)
      list(APPEND _glib2_link_libraries "${_pcre2_8bit_library}")
    else()
      list(APPEND _glib2_link_libraries "pcre")
    endif()
  endif()

  # pkg_check_modules consider these as LDFLAGS_OTHER rather instead of
  # libraries
  if(APPLE)
    list(APPEND _glib2_link_libraries "-Wl,-framework,Foundation"
         "-Wl,-framework,CoreFoundation" "-Wl,-framework,AppKit"
         "-Wl,-framework,Carbon")
  endif()

  add_library(GLib2::glib-2 UNKNOWN IMPORTED)
  set_target_properties(
    GLib2::glib-2
    PROPERTIES IMPORTED_LOCATION "${GLib2_glib-2_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_glib2_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${GLib2_INCLUDE_DIRS}"
               INTERFACE_LINK_LIBRARIES "${_glib2_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_glib2_link_directories}")
endif()

if(GLib2_gthread-2_LIBRARY AND NOT TARGET GLib2::gthread-2)
  # Handle transitive dependencies
  if(PC_GTHREAD2_FOUND)
    get_target_properties_from_pkg_config("${GLib2_gthread-2_LIBRARY}"
                                          "PC_GTHREAD2" "_gthread2")
  else()
    set(_gthread2_link_libraries "Threads::Threads" "GLib2::glib-2")
  endif()

  add_library(GLib2::gthread-2 UNKNOWN IMPORTED)
  set_target_properties(
    GLib2::gthread-2
    PROPERTIES IMPORTED_LOCATION "${GLib2_gthread-2_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_gthread2_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${GLib2_INCLUDE_DIRS}"
               INTERFACE_LINK_LIBRARIES "${_gthread2_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_gthread2_link_directories}")
endif()

if(GLib2_gmodule-2_LIBRARY AND NOT TARGET GLib2::gmodule-2)
  # Handle transitive dependencies
  if(PC_GMODULE2_FOUND)
    get_target_properties_from_pkg_config("${GLib2_gmodule-2_LIBRARY}"
                                          "PC_GMODULE2" "_gmodule2")
  else()
    set(_gmodule2_link_libraries "GLib2::glib-2")
  endif()

  add_library(GLib2::gmodule-2 UNKNOWN IMPORTED)
  set_target_properties(
    GLib2::gmodule-2
    PROPERTIES IMPORTED_LOCATION "${GLib2_gmodule-2_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_gmodule2_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${GLib2_INCLUDE_DIRS}"
               INTERFACE_LINK_LIBRARIES "${_gmodule2_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_gmodule2_link_directories}")
endif()

if(GLib2_gobject-2_LIBRARY AND NOT TARGET GLib2::gobject-2)
  # Handle transitive dependencies
  if(PC_GOBJECT2_FOUND)
    get_target_properties_from_pkg_config("${GLib2_gobject-2_LIBRARY}"
                                          "PC_OBJECT2" "_gobject2")
  else()
    find_package(libffi QUIET)
    set(_gobject2_link_libraries "libffi" "GLib2::glib-2")
  endif()

  add_library(GLib2::gobject-2 UNKNOWN IMPORTED)
  set_target_properties(
    GLib2::gobject-2
    PROPERTIES IMPORTED_LOCATION "${GLib2_gobject-2_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_gobject2_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${GLib2_INCLUDE_DIRS}"
               INTERFACE_LINK_LIBRARIES "${_gobject2_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_gobject2_link_directories}")
endif()

mark_as_advanced(GLib2_INCLUDE_DIR GLib2_glib-2_LIBRARY GLib2_gthread-2_LIBRARY
                 GLib2_gmodule-2_LIBRARY GLib2_gobject-2_LIBRARY)
