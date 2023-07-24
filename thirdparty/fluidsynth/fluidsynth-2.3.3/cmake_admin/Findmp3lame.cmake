#[=======================================================================[.rst:
Findmp3lame
-------

Finds the mp3lame library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``mp3lame::mp3lame``
  The mp3lame library.
``mp3lame::mpghip``
  The mpghip library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``mp3lame_FOUND``
  True if the mp3lame library was found.
``mp3lame_mpghip_FOUND``
  True if the mpghip library was found.

#]=======================================================================]

# Find the headers and libraries
find_path(mp3lame_INCLUDE_DIR NAMES "lame/lame.h")

find_library(mp3lame_mp3lame_LIBRARY NAMES "mp3lame" "libmp3lame"
                                           "libmp3lame-static")
find_library(mp3lame_mpghip_LIBRARY NAMES "mpghip" "libmpghip"
                                          "libmpghip-static")

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  mp3lame REQUIRED_VARS "mp3lame_mp3lame_LIBRARY"
                        "mp3lame_INCLUDE_DIR")

# Create the targets
if(mp3lame_FOUND AND NOT TARGET mp3lame::mp3lame)
  add_library(mp3lame::mp3lame UNKNOWN IMPORTED)
  set_target_properties(
    mp3lame::mp3lame
    PROPERTIES IMPORTED_LOCATION "${mp3lame_mp3lame_LIBRARY}"
               INTERFACE_INCLUDE_DIRECTORIES "${mp3lame_INCLUDE_DIR}")
  set(mp3lame_mp3lame_FOUND TRUE)
endif()

if(mp3lame_mpghip_LIBRARY AND NOT TARGET mp3lame::mpghip)
  add_library(mp3lame::mpghip UNKNOWN IMPORTED)
  set_target_properties(
    mp3lame::mpghip
    PROPERTIES IMPORTED_LOCATION "${mp3lame_mpghip_LIBRARY}"
               INTERFACE_INCLUDE_DIRECTORIES "${mp3lame_INCLUDE_DIR}")
  set(mp3lame_mpghip_FOUND)
endif()

mark_as_advanced(mp3lame_INCLUDE_DIR mp3lame_mp3lame_LIBRARY
                 mp3lame_mpghip_LIBRARY)
