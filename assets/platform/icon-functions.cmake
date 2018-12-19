if(BUILD_WINDOWS_ICONS)
  set(CONVERT "convert") # old name of ImageMagick's command line tool
  if(WIN32)
    # ImageMagick's convert conflicts with a Windows system tool of the same
    # name. A common solution is to rename ImageMagick's convert binary:
    set(CONVERT "imconvert") # name commonly used for "convert" on Windows
  endif(WIN32)
  required_program(IMAGEMAGICK "ImageMagick image tool - https://www.imagemagick.org" "magick" "${CONVERT}")
endif(BUILD_WINDOWS_ICONS)

if(BUILD_MACOS_ICONS)
  if(APPLE)
    set(BUILT_IN_MSG "This tool should be built-in. Check your PATH environment variable.")
    required_program(ICONUTIL "Tool to create macOS icons (libicns) - ${BUILT_IN_MSG}" "iconutil")
  else(APPLE)
    # NOTE: PNG2ICNS doesn't support icons for HDPI / "retina" displays. See
    # https://sourceforge.net/p/icns/bugs/12/. This probably doesn't matter if
    # using the same source SVG for retina and non-rentina icons anyway.
    required_program(PNG2ICNS "Tool to create macOS icons (libicns) - https://icns.sourceforge.io/" "png2icns")
    # There's more than one program called "png2icns". Do we have the right one?
    required_program(ICNS2PNG "You have the wrong png2icns. You need libicns from https://icns.sourceforge.io/" "icns2png")
  endif(APPLE)
endif(BUILD_MACOS_ICONS)

function(size_suffix # string appended to raster names to indicate pixel size
  SUFFIXV # return suffix in this variable
  SIZE # integer size in "points" (device-independent pixels)
  SCALE # integer device pixel ratio (number of pixels per "point")
  # NOTE: PIXEL_SIZE = SIZE * SCALE (so PIXEL_SIZE = SIZE when SCALE = 1)
  )
  # Follow Apple's naming scheme: https://stackoverflow.com/a/11788723
  set(SUFFIX "_${SIZE}x${SIZE}")
  if(SCALE GREATER 1)
    # images with SCALE > 1 are intended for use on "retina" (HDPI) displays
    set(SUFFIX "${SUFFIX}@${SCALE}x")
  endif(SCALE GREATER 1)
  set("${SUFFIXV}" "${SUFFIX}" PARENT_SCOPE)
endfunction(size_suffix)

function(png_sizes # perform operations on an image at different pixel sizes
  VERB # must be one of: LIST, CREATE or ICONSET
  FILE_BASE_PATH # path to PNGs minus any size suffix, or SVG minus extension
  SCALE # integer device pixel ratio (number of pixels per "point")
  PNG_PATHS_LISTV # list variable to store paths to output PNGs
  # ARGN remaining arguments are integer sizes (see function "size_suffix")
  )
  set(PNG_LIST "") # create empty list
  foreach(SIZE ${ARGN})
    size_suffix(SIZE_SUFFIX "${SIZE}" "${SCALE}")
    set(PNG_BASE_PATH "${FILE_BASE_PATH}${SIZE_SUFFIX}") # no extenion yet
    if(VERB STREQUAL "CREATE")
      # actually create the PNG
      math(EXPR PIXELS "${SIZE} * ${SCALE}")
      rasterize_svg("${FILE_BASE_PATH}.svg" "${PNG_BASE_PATH}-bloated.png" "--export-width=${PIXELS}" "--export-height=${PIXELS}")
      optimize_png("${PNG_BASE_PATH}-bloated.png" "${PNG_BASE_PATH}.png")
    elseif(VERB STREQUAL "ICONSET")
      # copy PNGs into iconset folder: https://stackoverflow.com/a/11788723
      set(ICONSET_PNG_BASE "${FILE_BASE_PATH}.iconset/icon${SIZE_SUFFIX}")
      copy_during_build("${PNG_BASE_PATH}.png" "${ICONSET_PNG_BASE}.png")
      set(PNG_BASE_PATH "${ICONSET_PNG_BASE}") # returned list will be iconset PNGs
    elseif(NOT (VERB STREQUAL "LIST"))
      message(FATAL_ERROR "VERB has unrecognised value '${VERB}'")
    endif(VERB STREQUAL "CREATE")
    list(APPEND PNG_LIST "${PNG_BASE_PATH}.png") # do this regardless of VERB
  endforeach(SIZE)
  set("${PNG_PATHS_LISTV}" "${PNG_LIST}" PARENT_SCOPE) # return the list
endfunction(png_sizes)

function(create_icon_ico # convert various sized PNGs into a single ICO icon
  FILE_BASE_PATH # path to input PNGs minus size suffix and extension
  ICO_FILE_OUTV # variable to store path to output ICO file if it is created
  # ARGN remaining arguments are integer sizes of input PNGs
  )
  if(BUILD_WINDOWS_ICONS)
    set(ICO_FILE_OUT "${FILE_BASE_PATH}.ico")
    png_sizes(LIST "${FILE_BASE_PATH}" 1 INPUT_PNGS ${ARGN})
    add_custom_command(
      OUTPUT "${ICO_FILE_OUT}"
      DEPENDS ${INPUT_PNGS} # paths can be relative since all PNGs are generated
      COMMAND "${IMAGEMAGICK}" ${INPUT_PNGS} "${ICO_FILE_OUT}"
      VERBATIM
      )
    set("${ICO_FILE_OUTV}" "${ICO_FILE_OUT}" PARENT_SCOPE) # return ICO path
  else(BUILD_WINDOWS_ICONS)
    set("${ICO_FILE_OUTV}" "" PARENT_SCOPE) # return nothing
  endif(BUILD_WINDOWS_ICONS)
endfunction(create_icon_ico)

function(create_icon_icns # convert various sized PNGs into a single ICNS icon
  FILE_BASE_PATH # path to input PNGs minus size suffix and extension
  ICNS_FILE_OUTV # variable to store path to output ICNS file if it is created
  # ARGN remaining arguments are integer sizes of input PNGs
  )
  if(BUILD_MACOS_ICONS)
    set(ICNS_FILE_OUT "${FILE_BASE_PATH}.icns")
    if(APPLE)
      # Apple's iconutil takes an "iconset" as input (directory of PNGs)
      set(INPUT_PNGS "") # empty list
      foreach(SCALE 1 2)
        png_sizes(ICONSET "${FILE_BASE_PATH}" "${SCALE}" ICONSET_PNGS ${ARGN})
        list(APPEND INPUT_PNGS "${ICONSET_PNGS}")
      endforeach(SCALE 1 2)
      add_custom_command(
        OUTPUT "${ICNS_FILE_OUT}"
        DEPENDS ${INPUT_PNGS}
        COMMAND "${ICONUTIL}" -c icns -o "${ICNS_FILE_OUT}" "${FILE_BASE_PATH}.iconset"
        VERBATIM
        )
    else(APPLE)
      # no retina support in png2icns https://sourceforge.net/p/icns/bugs/12/
      png_sizes(ICONSET "${FILE_BASE_PATH}" 1 ICONSET_PNGS ${ARGN}) # SCALE = 1 only
      add_custom_command(
        OUTPUT "${ICNS_FILE_OUT}"
        DEPENDS ${ICONSET_PNGS}
        COMMAND "${PNG2ICNS}" "${ICNS_FILE_OUT}" ${ICONSET_PNGS}
        VERBATIM
        )
    endif(APPLE)
    set("${ICNS_FILE_OUTV}" "${ICNS_FILE_OUT}" PARENT_SCOPE) # return path
  else(BUILD_MACOS_ICONS)
    set("${ICNS_FILE_OUTV}" "" PARENT_SCOPE) # return nothing
  endif(BUILD_MACOS_ICONS)
endfunction(create_icon_icns)
