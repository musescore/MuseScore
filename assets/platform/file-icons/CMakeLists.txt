set(GENERATED_FILES "") # empty list

function(filetype_icon # create filetype/mimetype icons from a source SVG
  SOURCE_FILE # source SVG/XML file that may contain @TYPE@ placeholder
  TYPE # file type text to substitute @TYPE@ placeholder with in source
  # ARGN remaining arguments are dependencies (resources or generated files)
  )
  get_filename_component(FILEBASE "${SOURCE_FILE}" NAME_WE) # strip file extensions
  string(TOLOWER "${TYPE}" TYPE_LOWER)
  configure_file("${SOURCE_FILE}" "${FILEBASE}_${TYPE_LOWER}.svg.xml") # substitute @TYPE@ in file
  file(READ "${CMAKE_CURRENT_BINARY_DIR}/${FILEBASE}_${TYPE_LOWER}.svg.xml" YOUR_CONTENT_HERE)
  string(REGEX REPLACE "^[^>]+>(.*)</[^<]+$" "\\1" YOUR_CONTENT_HERE "${YOUR_CONTENT_HERE}")
  set(YOUR_CONTENT_HERE "BEGIN ${FILEBASE}.svg -->\n${YOUR_CONTENT_HERE}\n<!-- END ${FILEBASE}.svg")
  configure_file("../../resources/folded-page.svg" "${TYPE_LOWER}-raw.svg") # substitute @YOUR_CONTENT_HERE@ in file
  standalone_svg("${TYPE_LOWER}-raw.svg" "${TYPE_LOWER}-vector.svg" ${ARGN})
  optimize_svg("${TYPE_LOWER}-vector.svg" "${TYPE_LOWER}.svg")
  list(APPEND GENERATED_FILES "${TYPE_LOWER}.svg")
  rasterize_svg_sizes("${TYPE_LOWER}" ${ALL_ICON_SIZES})
  create_icon_ico_sizes("${TYPE_LOWER}" ${WINDOWS_ICO_SIZES})
  create_icon_icns_sizes("${TYPE_LOWER}" ${MACOS_ICNS_SIZES})
  set(GENERATED_FILES "${GENERATED_FILES}" PARENT_SCOPE)
endfunction(filetype_icon)

# icons for MuseScore files
foreach(TYPE "MSCX" "MSCZ")
  filetype_icon(musescore-file.svg.xml.in "${TYPE}" "../../resources/mscore.svg")
endforeach(TYPE)

# icons for MusicXML files
foreach(TYPE "XML" "MXL")
  filetype_icon(musicxml-file-unofficial.svg.xml.in "${TYPE}" "../../resources/musicxml-logo-unofficial.svg")
endforeach(TYPE)

add_custom_target("assets_platform_fileicons" DEPENDS ${GENERATED_FILES})
add_dependencies("assets_platform_fileicons" "assets_resources")
add_dependencies("assets" "assets_platform_fileicons")

add_to_manifest(${GENERATED_FILES})
