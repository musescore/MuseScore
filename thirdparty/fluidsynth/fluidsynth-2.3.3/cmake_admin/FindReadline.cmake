#[=======================================================================[.rst:
FindReadline
-------

Finds the Readline library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Readline::Readline``
  The Readline library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Readline_FOUND``
  True if the system has the Readline library.
``Readline_VERSION``
  The version of the Readline library which was found.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_READLINE QUIET readline)

# Search for the headers and library
find_path(
  Readline_INCLUDE_DIR
  NAMES "history.h"
  HINTS "${PC_READLINE_INCLUDEDIR}"
  PATH_SUFFIXES "readline")

find_library(
  Readline_LIBRARY
  NAMES "readline"
  HINTS "${PC_READLINE_LIBDIR}")

# Get version from pkg-config or read the config header
if(PC_READLINE_VERSION)
  set(Readline_VERSION "${PC_READLINE_VERSION}")
elseif(Readline_INCLUDE_DIR)
  file(READ "${Readline_INCLUDE_DIR}/readline.h" _readline_h)
  string(REGEX MATCH "#define[ \t]+RL_VERSION_MAJOR[ \t]+([0-9]+)"
               _readline_major_re "${_readline_h}")
  set(_readline_major "${CMAKE_MATCH_1}")
  string(REGEX MATCH "#define[ \t]+RL_VERSION_MINOR[ \t]+([0-9]+)"
               _readline_minor_re "${_readline_h}")
  set(_readline_minor "${CMAKE_MATCH_1}")
  if(_readline_major_re AND _readline_minor_re)
    set(readline_VERSION "${_readline_major}.${_readline_minor}")
  endif()
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Readline
  REQUIRED_VARS "Readline_LIBRARY" "Readline_INCLUDE_DIR"
  VERSION_VAR "Readline_VERSION")

# Handle transitive dependencies
if(PC_READLINE_FOUND)
  get_target_properties_from_pkg_config("${ReadLine_LIBRARY}" "PC_READLINE"
                                        "_readline")
endif()

# Create the target
if(Readline_FOUND AND NOT TARGET Readline::Readline)
  add_library(Readline::Readline UNKNOWN IMPORTED)
  set_target_properties(
    Readline::Readline
    PROPERTIES IMPORTED_LOCATION "${Readline_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_readline_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES
               "${Readline_INCLUDE_DIR};${Readline_INCLUDE_DIR}/.."
               INTERFACE_LINK_LIBRARIES "${_readline_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_readline_link_directories}")
endif()

mark_as_advanced(Readline_INCLUDE_DIR Readline_LIBRARY)
