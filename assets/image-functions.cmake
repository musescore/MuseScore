required_program(INKSCAPE "SVG vector graphics editing program - https://inkscape.org/" "inkscape")
required_program(XMLLINT "Tool for parsing XML files - http://xmlsoft.org/xmllint.html" "xmllint")

if(OPTIMIZE_SVGS)
  required_program(SVGO "Tool for optimizing SVG vector graphics files" "svgo")
endif(OPTIMIZE_SVGS)

if(OPTIMIZE_PNGS)
  required_program(PNGCRUSH "Tool for optimizing PNG image files losslessly" "pngcrush")
endif(OPTIMIZE_PNGS)

# Need a function to convert relative paths to absolute paths:
#
#  1. CMake's add_custom_command() gives inconsistent results if relative
#     paths are used for its DEPENDS argument (see below).
#  2. Inkscape must be called with absolute paths on macOS. See
#     https://bugs.launchpad.net/inkscape/+bug/181639.
#
# All path arguments to add_custom_command() are relative to the current build
# directory (CMAKE_CURRENT_BINARY_DIR), except for some reason DEPENDS is
# relative to the source directory (CMAKE_CURRENT_SOURCE_DIR). However, if the
# named dependency was generated as OUTPUT to a previous add_custom_command()
# then DEPENDS becomes relative to build directory again!
function(absolute_path # prepend CMAKE_CURRENT_BINARY_DIR to relative paths
  ABS_PATHV # absolute path is returned through this variable
  PATH # input path - can be relative or absolute
  )
  if(IS_ABSOLUTE "${PATH}")
    set("${ABS_PATHV}" "${PATH}" PARENT_SCOPE)
  else(IS_ABSOLUTE "${PATH}")
    set("${ABS_PATHV}" "${CMAKE_CURRENT_BINARY_DIR}/${PATH}" PARENT_SCOPE)
  endif(IS_ABSOLUTE "${PATH}")
endfunction(absolute_path)

function(copy_during_build # copy a file at build time
  SOURCE_FILE # path to file being copied
  DEST_FILE # path to new location
  )
  absolute_path(SOURCE_FILE_ABS "${SOURCE_FILE}") # needed for DEPENDS
  add_custom_command(
    OUTPUT "${DEST_FILE}" # relative path required
    DEPENDS "${SOURCE_FILE_ABS}" # absolute path required
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE_FILE}" "${DEST_FILE}"
    VERBATIM
    )
endfunction(copy_during_build)

function(standalone_svg # resolve and embed an SVG's external dependencies
  SVG_FILE_IN # path to the input SVG
  SVG_FILE_OUT # path where the output SVG will be written
  # ARGN remaining arguments are additional dependencies
  )
  absolute_path(SVG_FILE_IN_ABS "${SVG_FILE_IN}") # needed for DEPENDS
  absolute_path(SVG_FILE_OUT_ABS "${SVG_FILE_OUT}") # needed for Inkscape on macOS
  set(DEPENDENCIES "") # empty list
  foreach(DEPENDENCY ${ARGN})
    absolute_path(DEPENDENCY_ABS "${DEPENDENCY}") # needed for DEPENDS
    list(APPEND DEPENDENCIES "${DEPENDENCY_ABS}")
  endforeach(DEPENDENCY)
  add_custom_command(
    OUTPUT "${SVG_FILE_OUT}" # relative path required
    DEPENDS "${SVG_FILE_IN_ABS}" ${DEPENDENCIES} # absolute paths required
    COMMAND "${XMLLINT}" "${SVG_FILE_IN}" --xinclude --pretty 1 --output "${SVG_FILE_OUT}"
    COMMAND "${INKSCAPE}" "${SVG_FILE_OUT_ABS}" --verb=EditSelectAll --verb=org.ekips.filter.embedselectedimages --verb=FileSave --verb=FileQuit
    COMMAND "${INKSCAPE}" -z "${SVG_FILE_OUT_ABS}" --export-text-to-path --vacuum-defs --export-plain-svg "${SVG_FILE_OUT_ABS}"
    VERBATIM
    )
endfunction(standalone_svg)

function(vectorize_svg # convert text to paths to remove font dependencies
  SVG_FILE_IN # path to the input SVG
  SVG_FILE_OUT # path where the output SVG will be written
  )
  absolute_path(SVG_FILE_IN_ABS "${SVG_FILE_IN}") # needed for DEPENDS
  absolute_path(SVG_FILE_OUT_ABS "${SVG_FILE_OUT}") # needed for Inkscape on macOS
  add_custom_command(
    OUTPUT "${SVG_FILE_OUT}" # relative path required
    DEPENDS "${SVG_FILE_IN_ABS}" # absolute path required
    COMMAND "${INKSCAPE}" -z "${SVG_FILE_IN_ABS}" --export-text-to-path --vacuum-defs --export-plain-svg "${SVG_FILE_OUT_ABS}"
    VERBATIM
    )
endfunction(vectorize_svg)

function(optimize_svg # reduce size of an SVG without changing its appearance
  SVG_FILE_IN # path to the input SVG
  SVG_FILE_OUT # path where the output SVG will be written
  )
  if(OPTIMIZE_SVGS)
    absolute_path(SVG_FILE_IN_ABS "${SVG_FILE_IN}") # needed for DEPENDS
    add_custom_command(
      OUTPUT "${SVG_FILE_OUT}" # relative path required
      DEPENDS "${SVG_FILE_IN_ABS}" # absolute path required
      COMMAND "${SVGO}" "${SVG_FILE_IN}" -o "${SVG_FILE_OUT}"
      VERBATIM
      )
  else(OPTIMIZE_SVGS)
    copy_during_build("${SVG_FILE_IN}" "${SVG_FILE_OUT}")
  endif(OPTIMIZE_SVGS)
endfunction(optimize_svg)

function(rasterize_svg # convert SVG to PNG
  SVG_FILE_IN # path to the input SVG
  PNG_FILE_OUT # path where the output PNG will be written
  # ARGN any additional arguments will be passed to inkscape
  )
  absolute_path(SVG_FILE_IN_ABS "${SVG_FILE_IN}") # needed for DEPENDS
  absolute_path(PNG_FILE_OUT_ABS "${PNG_FILE_OUT}") # needed for Inkscape on macOS
  add_custom_command(
    OUTPUT "${PNG_FILE_OUT}" # relative path required
    DEPENDS "${SVG_FILE_IN_ABS}" # absolute path required
    COMMAND "${INKSCAPE}" -z "${SVG_FILE_IN_ABS}" ${ARGN} --export-png "${PNG_FILE_OUT_ABS}"
    VERBATIM
    )
endfunction(rasterize_svg)

function(optimize_png # reduce size of a PNG without changing its appearance
  PNG_FILE_IN # path to the input PNG
  PNG_FILE_OUT # path where the output PNG will be written
  )
  if(OPTIMIZE_PNGS)
    set(OPTS -rem allb)
    if (OPTIMIZE_PNGS_BRUTE)
      list(APPEND OPTS -brute)
    endif(OPTIMIZE_PNGS_BRUTE)
    absolute_path(PNG_FILE_IN_ABS "${PNG_FILE_IN}") # needed for DEPENDS
    add_custom_command(
      OUTPUT "${PNG_FILE_OUT}" # relative path required
      DEPENDS "${PNG_FILE_IN_ABS}" # absolute path required
      COMMAND "${PNGCRUSH}" ${OPTS} "${PNG_FILE_IN}" "${PNG_FILE_OUT}"
      VERBATIM
      )
  else(OPTIMIZE_PNGS)
    copy_during_build("${PNG_FILE_IN}" "${PNG_FILE_OUT}")
  endif(OPTIMIZE_PNGS)
endfunction(optimize_png)
