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

function(size_suffix # value appended to raster names to indicate pixel size
  SUFFIXV # return suffix in this variable
  SIZE # integer size in "points" (device-independent pixels)
  SCALE # integer device pixel ratio (number of pixels per point)
  )
  # Follow Apple's naming scheme: https://stackoverflow.com/a/11788723
  set(SUFFIX "_${SIZE}x${SIZE}")
  if(SCALE GREATER 1)
    set(SUFFIX "${SUFFIX}@${SCALE}x")
  endif(SCALE GREATER 1)
  set("${SUFFIXV}" "${SUFFIX}" PARENT_SCOPE)
endfunction(size_suffix)

function(rasterize_svg_sizes # convert an SVG to PNGs at at various sizes
  FILE_BASE # path to input SVG file minus the file extension
  SCALE # integer device pixel ratio (SIZE * SCALE = physical pixel size)
  PNG_PATHS_OUTV # list variable to store paths to output PNGs
  # ARGN remaining arguments are integer SIZES of exported PNGs
  )
  set(SVG_FILE_IN "${FILE_BASE}.svg")
  set(PNG_PATHS_OUT "") # empty list
  foreach(SIZE ${ARGN})
    size_suffix(SIZE_SUFFIX "${SIZE}" "${SCALE}")
    set(PNG_FILE_OUT "${FILE_BASE}${SIZE_SUFFIX}.png")
    set(PNG_NOT_OPTIMIZED "${FILE_BASE}${SIZE_SUFFIX}-bloated.png")
    math(EXPR PIXELS "${SIZE} * ${SCALE}")
    rasterize_svg("${SVG_FILE_IN}" "${PNG_NOT_OPTIMIZED}" "--export-width=${PIXELS}" "--export-height=${PIXELS}")
    optimize_png("${PNG_NOT_OPTIMIZED}" "${PNG_FILE_OUT}")
    list(APPEND PNG_PATHS_OUT "${PNG_FILE_OUT}")
  endforeach(SIZE)
  set("${PNG_PATHS_OUTV}" "${PNG_PATHS_OUT}" PARENT_SCOPE)
endfunction(rasterize_svg_sizes)

function(create_icon_ico # convert multiple PNG files into a single ICO icon
  ICO_FILE_OUT # path where the output ICO file will be written
  # ARGN remaining arguments are PNG input files
  )
  if(BUILD_WINDOWS_ICONS)
    add_custom_command(
      OUTPUT "${ICO_FILE_OUT}"
      DEPENDS ${ARGN} # paths can be relative since all PNGs are generated
      COMMAND "${IMAGEMAGICK}" ${ARGN} "${ICO_FILE_OUT}"
      VERBATIM
      )
  endif(BUILD_WINDOWS_ICONS)
endfunction(create_icon_ico)

function(create_icon_icns # convert multiple PNG files into a single ICNS icon
  ICNS_FILE_OUT # path where the output ICNS file will be written
  PNG_BASE_IN # name of input PNGs minus size suffix and extension
  # ARGN remaining arguments are integer sizes of input PNGs
  )
  if(BUILD_MACOS_ICONS)
    if(APPLE)
      # copy PNGs into an "iconset" (directory with name ending ".iconset")
      get_filename_component(ICON_FILE_BASE "${ICNS_FILE_OUT}" NAME_WE)
      set(ICONSET "${ICON_FILE_BASE}.iconset")
      set(ICONSET_PNGS "") # empty list
      foreach(SCALE 1 2) # also embed images at double size for retina displays
        foreach(SIZE ${ARGN})
          size_suffix(SIZE_SUFFIX "${SIZE}" "${SCALE}")
          set(ICONSET_PNG "${ICONSET}/icon${SIZE_SUFFIX}.png")
          copy_during_build("${PNG_BASE_IN}${SIZE_SUFFIX}.png" "${ICONSET_PNG}")
          list(APPEND ICONSET_PNGS "${ICONSET_PNG}")
        endforeach(SIZE)
      endforeach(SCALE)
      add_custom_command(
        OUTPUT "${ICNS_FILE_OUT}"
        DEPENDS ${ICONSET_PNGS}
        COMMAND "${ICONUTIL}" -c icns -o "${ICNS_FILE_OUT}" "${ICONSET}"
        VERBATIM
        )
    else(APPLE)
      set(PNG_FILES_IN "") # empty list
      set(SCALE 1) # See https://sourceforge.net/p/icns/bugs/12/
      foreach(SIZE ${ARGN})
        size_suffix(SIZE_SUFFIX "${SIZE}" "${SCALE}")
        list(APPEND PNG_FILES_IN "${PNG_BASE_IN}${SIZE_SUFFIX}.png")
      endforeach(SIZE)
      add_custom_command(
        OUTPUT "${ICNS_FILE_OUT}"
        DEPENDS ${PNG_FILES_IN}
        COMMAND "${PNG2ICNS}" "${ICNS_FILE_OUT}" ${PNG_FILES_IN}
        VERBATIM
        )
    endif(APPLE)
  endif(BUILD_MACOS_ICONS)
endfunction(create_icon_icns)

function(create_icon_ico_sizes # create a Windows ICO icon file from PNGs
  FILE_BASE # name of input PNGs minus size and file extension
  # ARGN remaining arguments are integer sizes for input PNGs
  )
  if(BUILD_WINDOWS_ICONS)
    set(PNG_INPUT_FILES "") # empty list
    foreach(SIZE ${ARGN})
      size_suffix(SIZE_SUFFIX "${SIZE}" 1)
      list(APPEND PNG_INPUT_FILES "${FILE_BASE}${SIZE_SUFFIX}.png")
    endforeach(SIZE)
    create_icon_ico("${FILE_BASE}.ico" ${PNG_INPUT_FILES})
    list(APPEND GENERATED_FILES "${FILE_BASE}.ico")
    set(GENERATED_FILES "${GENERATED_FILES}" PARENT_SCOPE)
  endif(BUILD_WINDOWS_ICONS)
endfunction(create_icon_ico_sizes)

function(create_icon_icns_sizes # create a macOS ICNS icon file from PNGs
  FILE_BASE # name of input PNGs minus size and file extension
  # ARGN remaining arguments are integer sizes for input PNGs
  )
  if(BUILD_MACOS_ICONS)
    set(PNG_INPUT_FILES "") # empty list ("unset" exposes cached values)
    create_icon_icns("${FILE_BASE}.icns" "${FILE_BASE}" ${ARGN})
    list(APPEND GENERATED_FILES "${FILE_BASE}.icns")
    set(GENERATED_FILES "${GENERATED_FILES}" PARENT_SCOPE)
  endif(BUILD_MACOS_ICONS)
endfunction(create_icon_icns_sizes)
