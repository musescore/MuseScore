set(GENERATED_FILES "") # empty list

function(filetype_icon SVG_FILE_IN)
  foreach(TYPE ${ARGN})
    string(TOLOWER "${TYPE}" TYPE_LOWER)
    configure_file("${SVG_FILE_IN}" "${SVG_FILE_IN}") # substitute @TYPE@ in file
    file(READ "${CMAKE_CURRENT_BINARY_DIR}/${SVG_FILE_IN}" YOUR_CONTENT_HERE)
    string(REGEX REPLACE "^[^>]+>(.*)</[^<]+$" "\\1" YOUR_CONTENT_HERE "${YOUR_CONTENT_HERE}")
    set(YOUR_CONTENT_HERE "BEGIN ${SVG_FILE_IN} -->\n${YOUR_CONTENT_HERE}\n<!-- END ${SVG_FILE_IN}")
    configure_file("../../resources/folded-page.svg" "${TYPE_LOWER}-raw.svg") # substitute @YOUR_CONTENT_HERE@ in file
    standalone_svg("${TYPE_LOWER}-raw.svg" "${TYPE_LOWER}-vector.svg")
    optimize_svg("${TYPE_LOWER}-vector.svg" "${TYPE_LOWER}.svg")
    list(APPEND GENERATED_FILES "${TYPE_LOWER}.svg" "${TYPE_LOWER}.ico" "${TYPE_LOWER}.icns")
    rasterize_svg_sizes("${TYPE_LOWER}" ${ALL_ICON_SIZES})
    create_icon_ico_sizes("${TYPE_LOWER}" ${WINDOWS_ICO_SIZES})
    create_icon_icns_sizes("${TYPE_LOWER}" ${MACOS_ICNS_SIZES})
  endforeach(TYPE)
  set(GENERATED_FILES "${GENERATED_FILES}" PARENT_SCOPE)
endfunction(filetype_icon)

# icons for MuseScore files
filetype_icon(musescore-file.svg "MSCX" "MSCZ")

# icons for MusicXML files
filetype_icon(musicxml-file.svg "XML" "MXL")

add_custom_target("assets_icons_file" DEPENDS ${GENERATED_FILES})
add_dependencies("assets" "assets_icons_file")

add_to_manifest(${GENERATED_FILES})