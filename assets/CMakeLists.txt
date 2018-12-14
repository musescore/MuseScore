# Create a new CMake project for the assets to allow them to be generated
# independently of MuseScore's build.
project("MuseScore assets"
  DESCRIPTION "Build MuseScore's icons and resources from SVG source files"
  LANGUAGES NONE
  )

cmake_minimum_required(VERSION 3.0)

if(WIN32)
  set(WIN_NOT_AVAIL "Not available on Windows")
  option(BUILD_MACOS_ICONS "${WIN_NOT_AVAIL}" OFF)
endif(WIN32)

# The following options are enabled/disabled as appropriate for somebody
# building the assets alone. If MuseScore requires different settings then
# these should be made in MuseScore's top-level CMakeLists.txt. Options with
# the same name in MuseScore's CMakeLists.txt will override the ones here.
option(STRICT_FONTS "Fail build if required fonts are not installed (disable to allow build to proceed using a fallback font)" ON)
option(STRICT_VARIABLES "Fail build if required variables are not defined (disable to allow build to proceed using default values)" ON)
option(BUILD_WINDOWS_ICONS "Build ICO icons for Windows and associated PNG sizes." ON)
option(BUILD_MACOS_ICONS "Build ICNS icons for macOS and associated PNG sizes." ON)
option(BUILD_FREEDESKTOP_ICONS "Build PNG icons at sizes commonly used on Linux and other systems that obey the FreeDesktop.org standards." ON)
option(OPTIMIZE_SVGS "Optimize SVG file size using SVGO." ON)
option(OPTIMIZE_PNGS "Optimize PNG file size using PNGCRUSH." ON)
option(OPTIMIZE_PNGS_BRUTE "Try all PNG compression methods to ensure smallest possible size (takes time)." OFF)

# Load CMake code stored in other files - modular programming (sort of)
include("basic-functions.cmake")
include("image-functions.cmake")
if(STRICT_FONTS)
  include("font-dependencies.cmake")
else(STRICT_FONTS)
  message(STATUS "Not checking system fonts (STRICT_FONTS is OFF)")
  message(WARNING "Text in assets will look wrong if required fonts are not installed.")
endif(STRICT_FONTS)

# Some assets depend on variables defined in MuseScore's CMakeLists.txt
# set default values here to allow independent building of the assets.
required_variable(MUSESCORE_NAME "MuseScore")
required_variable(MUSESCORE_VERSION_FULL "X.Y.Z")

# Optionally pack assets into a ZIP file for easy distribution.
set(ASSETS_ARCHIVE_NAME "${MUSESCORE_NAME}-assets-${MUSESCORE_VERSION_FULL}.zip")

# Store a list of all assets generated as part of the build.
set(ASSETS_MANIFEST "assets-manifest.txt") # used for archiving and CI tests.
# As a basic CI test, a reference copy of this file is checked into the repo
# and compared to the one generated during the build to ensure they match.
# This will catch simple errors, but assets must still be compared visually.

# create empty manifest file to append to
file(WRITE "${PROJECT_BINARY_DIR}/${ASSETS_MANIFEST}" "# DO NOT EDIT!!! This file is generated during the build.\n")

function(add_to_manifest # add assets to the manifest file
  ASSET # name of asset file to be added to manifest
  # ARGN optionally specify more asset files as additional arguments
  )
  foreach(FILE "${ASSET}" ${ARGN})
    # expand path to asset file to make it relative to the build directory
    file(RELATIVE_PATH PATH "${PROJECT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/${FILE}")
    file(APPEND "${PROJECT_BINARY_DIR}/${ASSETS_MANIFEST}" "${PATH}\n")
  endforeach(FILE)
endfunction(add_to_manifest)

add_custom_target("assets" ALL) # main target to build all assets

add_subdirectory(resources)
add_subdirectory(brand)
add_subdirectory(platform)
add_subdirectory(splash)

# Read in the manifest we just created (ignore lines that begin with '#')
file(STRINGS "${PROJECT_BINARY_DIR}/${ASSETS_MANIFEST}" ASSET_FILES REGEX "^[^#]")
set(ASSET_FILES_ABS "")
foreach(ASSET ${ASSET_FILES})
  list(APPEND ASSET_FILES_ABS "${PROJECT_BINARY_DIR}/${ASSET}")
endforeach(ASSET)

# optional target to create a compressed ZIP archive containing all assets
add_custom_target("assets_archive" DEPENDS "${ASSETS_ARCHIVE_NAME}")
add_dependencies("assets_archive" "assets") # assets must be built first
add_custom_command(
  OUTPUT "${ASSETS_ARCHIVE_NAME}"
  DEPENDS ${ASSET_FILES_ABS} # rebuild if any of these files have changed (absolute paths required)
  COMMAND "${CMAKE_COMMAND}" -E tar "cfv" "${ASSETS_ARCHIVE_NAME}" --format=zip ${ASSET_FILES}
  VERBATIM
  )
