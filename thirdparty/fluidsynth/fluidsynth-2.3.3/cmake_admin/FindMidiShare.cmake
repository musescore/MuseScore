#[=======================================================================[.rst:
FindMidiShare
-------

Finds the MidiShare library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MidiShare::MidiShare``
  The MidiShare library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``MidiShare_FOUND``
  True if the system has the MidiShare library.
``MidiShare_VERSION``
  The version of the MidiShare library which was found.

#]=======================================================================]

# Find headers and library
find_path(MidiShare_INCLUDE_DIR NAMES "MidiShare.h")
find_library(MidiShare_LIBRARY NAMES "MidiShare")

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  MidiShare REQUIRED_VARS "MidiShare_LIBRARY" "MidiShare_INCLUDE_DIR")

# Create the target
if(MidiShare_FOUND AND NOT TARGET MidiShare::MidiShare)
  add_library(MidiShare::MidiShare UNKNOWN IMPORTED)
  set_target_properties(
    MidiShare::MidiShare
    PROPERTIES IMPORTED_LOCATION "${MidiShare_LIBRARY}"
               INTERFACE_INCLUDE_DIRECTORIES "${MidiShare_INCLUDE_DIR}")
endif()

mark_as_advanced(MidiShare_INCLUDE_DIR MidiShare_LIBRARY)
