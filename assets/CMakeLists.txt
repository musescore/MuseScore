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
option(TESTS "Run code validation and other checks during the build process" ON)
option(STRICT_FONTS "Fail build if required fonts are not installed (disable to allow build to proceed using a fallback font)" ON)
option(STRICT_VARIABLES "Fail build if required variables are not defined (disable to allow build to proceed using default values)" ON)
option(BUILD_WINDOWS_ICONS "Build ICO icon files for Windows" ON)
option(BUILD_MACOS_ICONS "Build ICNS icon files for macOS" ON)
option(BUILD_FREEDESKTOP_ICONS "Build PNG icons for Linux and other systems that follow the FreeDesktop.org specifications" ON)
option(OPTIMIZE_SVGS "Optimize SVG file size using SVGO" ON)
option(OPTIMIZE_PNGS "Optimize PNG file size using PNGCRUSH" ON)
option(OPTIMIZE_PNGS_BRUTE "Try all PNG compression methods to ensure smallest possible size (takes time)" OFF)

# Load CMake functions defined in other files - modular programming (sort of)
include("basic-functions.cmake")
include("image-functions.cmake")
if(STRICT_FONTS)
  include("font-dependencies.cmake")
else(STRICT_FONTS)
  message(STATUS "Not checking system fonts (STRICT_FONTS is OFF)")
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

add_subdirectory(resources) # resources first (everything depends on them)

add_custom_target("assets" ALL) # main target to build all other assets

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

if(TESTS)
  # POST CONFIGURE CHECKS - these are run by CMake just before it exits.
  # NOTE: CMake only runs on the first build or when the CMake or configured
  # files change. All other files must be checked at build time.
  required_program(GIT "Distributed VCS - https://git-scm.com/" "git")
  execute_process(
    COMMAND "${GIT}" diff --color --no-index "${PROJECT_SOURCE_DIR}/${ASSETS_MANIFEST}" "${ASSETS_MANIFEST}"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    RESULT_VARIABLE RET_VAL
    )
  if(RET_VAL STREQUAL "1") # compare strings as RET_VAL not always a number
    message(FATAL_ERROR "Generated manifest doesn't match the repository version.")
  elseif(NOT RET_VAL STREQUAL "0")
    message(FATAL_ERROR "Diff command exited with '${RET_VAL}' indicating an error occured.")
  endif(RET_VAL STREQUAL "1")

  # PRE BUILD CHECKS - Run by the native build tool as soon as it starts.
  # NOTE: The build tool runs on every build, so these checks do too.
  required_program(ECLINT "Tool for checking code formatting and indentation - https://github.com/jedmao/eclint" "eclint")
  add_custom_target("assets_sanitycheck" ALL
    COMMAND "${ECLINT}" check # ignores all files in .gitignore
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    VERBATIM USES_TERMINAL
    )
  # now ensure check runs before everything else
  add_dependencies("assets_resources" "assets_sanitycheck")
endif(TESTS)
