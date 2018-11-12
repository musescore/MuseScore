# TLDR: CMakes's add_custom_command() gives unexpected results if relative
# paths are used for its DEPENDS argument, hence we must use absolute paths.
#
# All path arguments to add_custom_command() are relative to the current
# binary dir, except for some reason DEPENDS is relative to the source dir.
# However, if the named dependency was generated as OUTPUT to a previous
# add_custom_command() then DEPENDS becomes relative to the binary dir again!
function(make_path_absolute VAR_NAME)
  set(FILE_PATH "${${VAR_NAME}}")
  if(NOT IS_ABSOLUTE "${FILE_PATH}")
    set("${VAR_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/${FILE_PATH}" PARENT_SCOPE)
  endif(NOT IS_ABSOLUTE "${FILE_PATH}")
endfunction(make_path_absolute)

function(required_program VARIABLE COMMAND DESCRIPTION)
  # ARGN alternative names for the command
  if(BUILD_ASSETS)
    find_program("${VARIABLE}" NAMES ${COMMAND} ${ARGN} DOC "${DESCRIPTION}")
    if(NOT ${VARIABLE} OR NOT EXISTS "${${VARIABLE}}")
      set(MSG_TYPE FATAL_ERROR) # fail build due to missing dependency
      if(DOWNLOAD_ASSETS)
        set(MSG_TYPE WARNING) # don't fail the build
        set(BUILD_ASSETS OFF PARENT_SCOPE) # download assets instead of building them
      endif(DOWNLOAD_ASSETS)
      message("${MSG_TYPE}" "Not found: ${COMMAND} - ${DESCRIPTION}")
      message("A build dependency is missing so assets will be downloaded.")
    endif(NOT ${VARIABLE} OR NOT EXISTS "${${VARIABLE}}")
  endif(BUILD_ASSETS)
endfunction(required_program)

required_program(INKSCAPE "inkscape" "SVG vector graphics editing program - https://inkscape.org/")
required_program(XMLLINT "xmllint" "Tool for parsing XML files - http://xmlsoft.org/xmllint.html")

set(CONVERT "convert") # old name of ImageMagick's command line tool
if(WIN32)
  # Windows has a system tool called "convert" that conflicts with ImageMagick
  set(CONVERT "imconvert") # common solution is to rename ImageMagick's binary
endif(WIN32)
required_program(IMAGEMAGICK "magick" "ImageMagick image tool - https://www.imagemagick.org" "${CONVERT}")

# NOTE: Very few programs support the ICNS icon format used on macOS. On Linux
# we can use PNG2ICNS, but it doesn't support adding separate images for retina
# displays https://sourceforge.net/p/icns/bugs/12/. This is not currently a
# problem since we don't have separate images for retina displays anyway.
required_program(PNG2ICNS "png2icns" "Tool to create macOS icons (libicns)- https://icns.sourceforge.io/")
required_program(ICNS2PNG "icns2png" "If this is missing then you have the wrong png2icns")

if(OPTIMIZE_SVGS)
  required_program(SVGO "svgo" "Tool for optimizing SVG vector graphics files")
endif(OPTIMIZE_SVGS)

if(OPTIMIZE_PNGS)
  required_program(PNGCRUSH "pngcrush" "Tool for optimizing PNG image files losslessly")
endif(OPTIMIZE_PNGS)

function(standalone_svg SVG_FILE_IN SVG_FILE_OUT)
  if(BUILD_ASSETS)
    make_path_absolute(SVG_FILE_IN) # absolute path needed for add_custom_command
    add_custom_command(
      OUTPUT "${SVG_FILE_OUT}"
      DEPENDS "${SVG_FILE_IN}" # absolute path required
      COMMAND "${XMLLINT}" "${SVG_FILE_IN}" --xinclude --pretty 1 --output "${SVG_FILE_OUT}"
      COMMAND "${INKSCAPE}" "${SVG_FILE_OUT}" --verb=EditSelectAll --verb=EditUnlinkClone --verb=EditSelectAll --verb=org.ekips.filter.embedselectedimages --verb=FileSave --verb=FileQuit
      COMMAND "${INKSCAPE}" -z "${SVG_FILE_OUT}" --export-text-to-path --vacuum-defs --export-plain-svg "${SVG_FILE_OUT}"
      VERBATIM
      )
  endif(BUILD_ASSETS)
endfunction(standalone_svg)

function(optimize_svg SVG_FILE_IN SVG_FILE_OUT)
  if(BUILD_ASSETS)
    if(OPTIMIZE_SVGS)
      make_path_absolute(SVG_FILE_IN) # absolute path needed for add_custom_command
      add_custom_command(
        OUTPUT "${SVG_FILE_OUT}"
        DEPENDS "${SVG_FILE_IN}" # absolute path required
        COMMAND "${SVGO}" "${SVG_FILE_IN}" -o "${SVG_FILE_OUT}"
        VERBATIM
        )
    else(OPTIMIZE_SVGS)
      copy_during_build("${SVG_FILE_IN}" "${SVG_FILE_OUT}")
    endif(OPTIMIZE_SVGS)
  endif(BUILD_ASSETS)
endfunction(optimize_svg)

function(vectorize_svg SVG_FILE_IN SVG_FILE_OUT)
  if(BUILD_ASSETS)
    make_path_absolute(SVG_FILE_IN) # absolute path needed for add_custom_command
    add_custom_command(
      OUTPUT "${SVG_FILE_OUT}"
      DEPENDS "${SVG_FILE_IN}" # absolute path required
      COMMAND "${INKSCAPE}" -z "${SVG_FILE_IN}" --export-text-to-path --vacuum-defs --export-plain-svg "${SVG_FILE_OUT}"
      VERBATIM
      )
  endif(BUILD_ASSETS)
endfunction(vectorize_svg)

function(rasterize_svg SVG_FILE_IN PNG_FILE_OUT)
  if(BUILD_ASSETS)
    # any additional arguments will be passed to inkscape (see ${ARGN})
    make_path_absolute(SVG_FILE_IN) # absolute path needed for add_custom_command
    add_custom_command(
      OUTPUT "${PNG_FILE_OUT}"
      DEPENDS "${SVG_FILE_IN}" # absolute path required
      COMMAND "${INKSCAPE}" -z "${SVG_FILE_IN}" ${ARGN} --export-png "${PNG_FILE_OUT}"
      VERBATIM
      )
  endif(BUILD_ASSETS)
endfunction(rasterize_svg)

function(optimize_png PNG_FILE_IN PNG_FILE_OUT)
  if(BUILD_ASSETS)
    if(OPTIMIZE_PNGS)
      set(OPTS -rem allb)
      if (OPTIMIZE_PNGS_BRUTE)
        list(APPEND OPTS -brute)
      endif(OPTIMIZE_PNGS_BRUTE)
      make_path_absolute(PNG_FILE_IN) # absolute path needed for add_custom_command
      add_custom_command(
        OUTPUT "${PNG_FILE_OUT}"
        DEPENDS "${PNG_FILE_IN}" # absolute path required
        COMMAND "${PNGCRUSH}" ${OPTS} "${PNG_FILE_IN}" "${PNG_FILE_OUT}"
        VERBATIM
        )
    else(OPTIMIZE_PNGS)
      copy_during_build("${PNG_FILE_IN}" "${PNG_FILE_OUT}")
    endif(OPTIMIZE_PNGS)
  endif(BUILD_ASSETS)
endfunction(optimize_png)

function(create_icon_ico ICO_FILE_OUT)
  # ARGN additional arguments are PNG input files for ImageMagick
  if(BUILD_ASSETS)
    add_custom_command(
      OUTPUT "${ICO_FILE_OUT}"
      DEPENDS ${ARGN} # paths can be relative since all PNGs are generated
      COMMAND "${IMAGEMAGICK}" ${ARGN} "${ICO_FILE_OUT}"
      VERBATIM
      )
  endif(BUILD_ASSETS)
endfunction(create_icon_ico)

function(create_icon_icns ICNS_FILE_OUT)
  # ARGN additional arguments are PNG input files for ImageMagick
  if(BUILD_ASSETS)
    add_custom_command(
      OUTPUT "${ICNS_FILE_OUT}"
      DEPENDS ${ARGN} # paths can be relative since all PNGs are generated
      COMMAND "${PNG2ICNS}" "${ICNS_FILE_OUT}" ${ARGN}
      VERBATIM
      )
  endif(BUILD_ASSETS)
endfunction(create_icon_icns)

function(copy_during_build SOURCE_FILE DEST_FILE)
  if(BUILD_ASSETS)
    make_path_absolute(SOURCE_FILE) # absolute path needed for add_custom_command
    add_custom_command(
      OUTPUT "${DEST_FILE}"
      DEPENDS "${SOURCE_FILE}" # absolute path required
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE_FILE}" "${DEST_FILE}"
      VERBATIM
      )
  endif(BUILD_ASSETS)
endfunction(copy_during_build)
