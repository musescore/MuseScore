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
  list(APPEND GENERATED_FILES "${FILEBASE}.svg")
  rasterize_svg_sizes("${FILEBASE}" 1 GENERATED_PNGS ${ALL_ICON_SIZES})
  rasterize_svg_sizes("${FILEBASE}" 2 GENERATED_PNGS_2X ${MACOS_ICNS_SIZES})
  create_icon_ico_sizes("${FILEBASE}" ${WINDOWS_ICO_SIZES})
  create_icon_icns_sizes("${FILEBASE}" ${MACOS_ICNS_SIZES})
  list(APPEND GENERATED_FILES "${GENERATED_PNGS};${GENERATED_PNGS_2X}")
endforeach(FILE)

add_custom_target("assets_platform_appicons" DEPENDS ${GENERATED_FILES})
add_dependencies("assets_platform_appicons" "assets_resources")
add_dependencies("assets" "assets_platform_appicons")
add_to_manifest(${GENERATED_FILES})
