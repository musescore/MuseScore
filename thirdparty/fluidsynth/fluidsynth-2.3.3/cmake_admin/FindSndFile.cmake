#[=======================================================================[.rst:
FindSndFile
-------

Finds the SndFile library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SndFile::sndfile``
  The SndFile library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SndFile_FOUND``
  True if the system has the SndFile library.
``SndFile_VERSION``
  The version of the SndFile library which was found.
``SndFile_WITH_EXTERNAL_LIBS``
  True if the library was built with Xiph codecs.

For compatibility with upstream, the following variables are also set:

``SndFile_WITH_MPEG``
``SndFile_VERSION_MAJOR``
``SndFile_VERSION_MINOR``
``SndFile_VERSION_PATCH``
``SndFile_LIBRARY``
``SndFile_LIBRARIES``
``SNDFILE_LIBRARY``
``SNDFILE_LIBRARIES``
``SNDFILE_INCLUDE_DIR``

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_SNDFILE QUIET sndfile)

# Find the headers and libraries
find_path(
  SndFile_INCLUDE_DIR
  NAMES "sndfile.h"
  HINTS "${PC_SNDFILE_INCLUDEDIR}")

find_library(
  _sndfile_library
  NAMES "sndfile"
  HINTS "${PC_SNDFILE_LIBDIR}")

# Get version from pkg-config or read the config header
if(PC_SNDFILE_VERSION)
  set(SndFile_VERSION "${PC_SNDFILE_VERSION}")
  string(REPLACE "." ";" _sndfile_version_list "${SndFile_VERSION}")
  list(GET _sndfile_version_list 0 SndFile_VERSION_MAJOR)
  list(GET _sndfile_version_list 1 SndFile_VERSION_MINOR)
  list(GET _sndfile_version_list 2 SndFile_VERSION_PATCH)
elseif(SndFile_INCLUDE_DIR)
  file(READ "${SndFile_INCLUDE_DIR}/sndfile.h" _sndfile_h)
  if("#define SNDFILE_1" MATCHES _snfile_h)
    set(SndFile_VERSION "1")
    set(SndFile_VERSION_MAJOR "1")
  endif()
endif()

# Check the features SndFile was built with
if(PC_SNDFILE_FOUND)
  if("vorbis" IN_LIST PC_SNDFILE_STATIC_LIBRARIES)
    set(SndFile_WITH_EXTERNAL_LIBS TRUE)
  endif()
  if("mpg123" IN_LIST PC_SNDFILE_STATIC_LIBRARIES)
    set(SndFile_WITH_MPEG TRUE)
  endif()
elseif(_sndfile_library)
  # sndfile may need any of these libraries
  find_package(Ogg 1.3 QUIET)
  find_package(Vorbis QUIET)
  find_package(FLAC QUIET)
  find_package(Opus QUIET)
  find_package(mp3lame QUIET)
  find_package(mpg123 QUIET)

  if(NOT CMAKE_CROSSCOMPILING)
    include(CheckSourceRuns)
    set(_backup_includes ${CMAKE_REQUIRED_INCLUDES})
    set(_backup_libraries ${CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES "${SndFile_INCLUDE_DIR}")
    set(CMAKE_REQUIRED_LIBRARIES "${_sndfile_library}")

    set(_optional_libs "MPG123::libmpg123" "mp3lame::mp3lame" "FLAC::FLAC"
                       "Opus::opus" "Vorbis::vorbisenc" "Ogg::ogg")
    foreach(_target ${_optional_libs})
      if(TARGET "${_target}")
        list(APPEND CMAKE_REQUIRED_LIBRARIES "${_target}")
      endif()
    endforeach()

    check_source_runs(
      C
      "#include <stdlib.h>
#include <sndfile.h>
int main() {
  SF_FORMAT_INFO info = {SF_FORMAT_VORBIS};
  sf_command(NULL, SFC_GET_FORMAT_INFO, &info, sizeof info);
  return info.name != NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}"
      SNDFILE_SUPPORTS_VORBIS)

    check_source_runs(
      C
      "#include <stdlib.h>
#include <sndfile.h>
int main() {
  SF_FORMAT_INFO info = {SF_FORMAT_MPEG_LAYER_III};
  sf_command(NULL, SFC_GET_FORMAT_INFO, &info, sizeof info);
  return info.name != NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}"
      SNDFILE_SUPPORTS_MPEG)

    set(SndFile_WITH_EXTERNAL_LIBS ${SNDFILE_SUPPORTS_VORBIS})
    set(SndFile_WITH_MPEG ${SNDFILE_SUPPORTS_MPEG})
    set(CMAKE_REQUIRED_INCLUDES ${_backup_includes})
    set(CMAKE_REQUIRED_LIBRARIES ${_backup_libraries})
  else()
    message(
      STATUS
        "Cross-compiling without pkg-config - cannot check for external libraries."
        "If you have the upstream CMake config set CMAKE_FIND_PACKAGE_PREFER_CONFIG to true for accurate results."
    )
    set(SndFile_WITH_EXTERNAL_LIBS FALSE)
    set(SndFile_WITH_MPEG FALSE)
  endif()
endif()

# Handle transitive dependencies
if(PC_SNDFILE_FOUND)
  get_target_properties_from_pkg_config("${_sndfile_library}" "PC_SNDFILE"
                                        "_sndfile")
else()
  if(SndFile_WITH_EXTERNAL_LIBS)
    list(APPEND _sndfile_link_libraries "FLAC::FLAC" "Opus::opus"
         "Vorbis::vorbisenc" "Ogg::ogg")
  endif()
  if(SndFile_WITH_MPEG)
    list(APPEND _sndfile_link_libraries "MPG123::libmpg123" "mp3lame::mp3lame")
  endif()
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SndFile
  REQUIRED_VARS "_sndfile_library" "SndFile_INCLUDE_DIR"
  VERSION_VAR "SndFile_VERSION")

if(SndFile_FOUND AND NOT TARGET SndFile::sndfile)
  add_library(SndFile::sndfile UNKNOWN IMPORTED)
  set_target_properties(
    SndFile::sndfile
    PROPERTIES IMPORTED_LOCATION "${_sndfile_library}"
               INTERFACE_COMPILE_OPTIONS "${_sndfile_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${SndFile_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_sndfile_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_sndfile_link_directories}")

  # Set additional variables for compatibility with upstream config
  set(SNDFILE_FOUND TRUE)
  set(SndFile_LIBRARY SndFile::sndfile)
  set(SndFile_LIBRARIES SndFile::sndfile)
  set(SNDFILE_LIBRARY SndFile::sndfile)
  set(SNDFILE_LIBRARIES SndFile::sndfile)
  set(SNDFILE_INCLUDE_DIR "${SndFile_INCLUDE_DIR}")
endif()

mark_as_advanced(_sndfile_library)
