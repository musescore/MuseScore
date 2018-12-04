function(rasterize_svg_sizes # convert an SVG to PNGs at at various sizes
  FILE_BASE # path to input SVG file minus the file extension
  # ARGN remaining arguments are integer sizes of exported PNGs
  )
  foreach(SIZE ${ARGN})
    set(SVG_FILE_IN "${FILE_BASE}.svg")
    set(PNG_NOT_OPTIMIZED "${FILE_BASE}-${SIZE}-bloated.png")
    set(PNG_FILE_OUT "${FILE_BASE}-${SIZE}.png")
    rasterize_svg("${SVG_FILE_IN}" "${PNG_NOT_OPTIMIZED}" "--export-width=${SIZE}" "--export-height=${SIZE}")
    optimize_png("${PNG_NOT_OPTIMIZED}" "${PNG_FILE_OUT}")
    list(APPEND GENERATED_FILES "${PNG_FILE_OUT}")
  endforeach(SIZE)
  set(GENERATED_FILES "${GENERATED_FILES}" PARENT_SCOPE)
endfunction(rasterize_svg_sizes)

function(create_icon_ico_sizes # create a Windows ICO icon file from PNGs
  FILE_BASE # name of input PNGs minus size and file extension
  # ARGN remaining arguments are integer sizes for input PNGs
  )
  if(BUILD_WINDOWS_ICONS)
    set(PNG_INPUT_FILES "") # empty list ("unset" exposes cached values)
    foreach(SIZE ${ARGN})
      list(APPEND PNG_INPUT_FILES "${FILE_BASE}-${SIZE}.png")
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
    foreach(SIZE ${ARGN})
      list(APPEND PNG_INPUT_FILES "${FILE_BASE}-${SIZE}.png")
    endforeach(SIZE)
    create_icon_icns("${FILE_BASE}.icns" ${PNG_INPUT_FILES})
    list(APPEND GENERATED_FILES "${FILE_BASE}.icns")
    set(GENERATED_FILES "${GENERATED_FILES}" PARENT_SCOPE)
  endif(BUILD_MACOS_ICONS)
endfunction(create_icon_icns_sizes)
