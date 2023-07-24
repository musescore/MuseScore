#[=======================================================================[.rst:
FindPortAudio
-------

Finds the PortAudio library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``PortAudio::PortAudio``
  The PortAudio library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PortAudio_FOUND``
  True if the system has the PortAudio library.
``PortAudio_VERSION``
  The version of the PortAudio library which was found.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_PORTAUDIO QUIET portaudio-2.0)

# Find the headers and library
find_path(
  PortAudio_INCLUDE_DIR
  NAMES "portaudio.h"
  HINTS "${PC_PORTAUDIO_INCLUDEDIR}")

find_library(
  PortAudio_LIBRARY
  NAMES "portaudio"
  HINTS "${PC_PORTAUDIO_LIBDIR}")

# Handle transitive dependencies
if(PC_PORTAUDIO_FOUND)
  get_target_properties_from_pkg_config("${PortAudio_LIBRARY}" "PC_PORTAUDIO"
                                        "_portaudio")
else()
  set(_portaudio_link_libraries "ALSA::ALSA" ${MATH_LIBRARY} "Threads::Threads")
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  PortAudio REQUIRED_VARS "PortAudio_LIBRARY" "PortAudio_INCLUDE_DIR")

if(PortAudio_FOUND AND NOT TARGET PortAudio::PortAudio)
  add_library(PortAudio::PortAudio UNKNOWN IMPORTED)
  set_target_properties(
    PortAudio::PortAudio
    PROPERTIES IMPORTED_LOCATION "${PortAudio_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_portaudio_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${PortAudio_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_portaudio_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_portaudio_link_directories}")
endif()

mark_as_advanced(PortAudio_INCLUDE_DIR PortAudio_LIBRARY)
