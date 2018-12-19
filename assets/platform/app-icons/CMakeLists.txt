set(SOURCE_FILES
  musescore-icon-round.svg.xml
  musescore-icon-square.svg.xml
  )

assert_alphabetical(SOURCE_FILES) # keep list ordered for easier maintenance

set(GENERATED_FILES "") # create empty list

foreach(FILE ${SOURCE_FILES})
  get_filename_component(FILEBASE "${FILE}" NAME_WE) # strip file extensions
  standalone_svg("${CMAKE_CURRENT_SOURCE_DIR}/${FILE}" "${FILEBASE}-standalone.svg" "../../resources/mscore.svg")
  optimize_svg("${FILEBASE}-standalone.svg" "${FILEBASE}.svg")
  # newer Freedesktop systems can use the SVG directly
  list(APPEND GENERATED_FILES "${FILEBASE}.svg") # archive the SVG
  # Need many sizes of PNG - some to archive and some to embed in icon files.
  # Different platforms need different sizes, but there is some overlap. We
  # create all required sizes upfront to avoid building some twice.
  png_sizes(CREATE "${FILEBASE}" 1 ALL_PNGS ${ALL_ICON_SIZES}) # standard displays
  png_sizes(CREATE "${FILEBASE}" 2 ALL_PNGS_2X ${MACOS_ICNS_SIZES}) # retina displays
  # Windows & macOS have special icon formats that are built from a subset of
  # the PNGs just created. The actual PNGs aren't needed by those platforms.
  create_icon_ico("${FILEBASE}" GENERATED_ICO ${WINDOWS_ICO_SIZES})
  create_icon_icns("${FILEBASE}" GENERATED_ICNS ${MACOS_ICNS_SIZES})
  # some Freedesktop systems use PNGs as icons so we list them for archiving
  png_sizes(LIST "${FILEBASE}" 1 GENERATED_PNGS ${FREEDESKTOP_COMMON_SIZES})
  # only archive files we actually need, not any used only to build other icons
  list(APPEND GENERATED_FILES ${GENERATED_ICO} ${GENERATED_ICNS} ${GENERATED_PNGS})
endforeach(FILE)

add_custom_target("assets_platform_appicons" DEPENDS ${GENERATED_FILES})
add_dependencies("assets_platform_appicons" "assets_resources")
add_dependencies("assets" "assets_platform_appicons")
add_to_manifest(${GENERATED_FILES})
