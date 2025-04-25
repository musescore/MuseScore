#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Opus::opus" for configuration "Release"
set_property(TARGET Opus::opus APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Opus::opus PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libopus.a"
  )

list(APPEND _cmake_import_check_targets Opus::opus )
list(APPEND _cmake_import_check_files_for_Opus::opus "${_IMPORT_PREFIX}/lib/libopus.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
