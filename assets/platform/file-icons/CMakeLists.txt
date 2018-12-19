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
  # newer Freedesktop systems (Linux and friends) can use the SVG as an icon
  list(APPEND GENERATED_FILES "${TYPE_LOWER}.svg") # archive the SVG
  # Need many sizes of PNG - some to archive and some to embed in icon files.
  # Different platforms need different sizes, but there is some overlap. We
  # create all required sizes upfront to avoid building some twice.
  png_sizes(CREATE "${TYPE_LOWER}" 1 ALL_PNGS ${ALL_ICON_SIZES})
  png_sizes(CREATE "${TYPE_LOWER}" 2 ALL_PNGS_2X ${MACOS_ICNS_SIZES}) # retina sizes
  # Windows & macOS have special icon formats that are built from a subset of
  # the PNGs just created. The actual PNGs aren't needed by those platforms.
  create_icon_ico("${TYPE_LOWER}" GENERATED_ICO ${WINDOWS_ICO_SIZES})
  create_icon_icns("${TYPE_LOWER}" GENERATED_ICNS ${MACOS_ICNS_SIZES})
  # some Freedesktop systems use PNGs as icons so we list them for archiving
  png_sizes(LIST "${TYPE_LOWER}" 1 GENERATED_PNGS ${FREEDESKTOP_COMMON_SIZES})
  # only archive files we actually need, not any used only to build other icons
  list(APPEND GENERATED_FILES ${GENERATED_ICO} ${GENERATED_ICNS} ${GENERATED_PNGS})
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
