set(GENERATED_FILES "") # empty list

foreach(FILE musescore-icon-round musescore-icon-square)
  standalone_svg("${CMAKE_CURRENT_SOURCE_DIR}/${FILE}.svg" "${FILE}-standalone.svg")
  optimize_svg("${FILE}-standalone.svg" "${FILE}.svg")
  list(APPEND GENERATED_FILES "${FILE}.svg" "${FILE}.ico" "${FILE}.icns")
  rasterize_svg_sizes("${FILE}" ${ALL_ICON_SIZES})
  create_icon_ico_sizes("${FILE}" ${WINDOWS_ICO_SIZES})
  create_icon_icns_sizes("${FILE}" ${MACOS_ICNS_SIZES})
endforeach(FILE)

add_custom_target("assets_icons_app" DEPENDS ${GENERATED_FILES})
add_dependencies("assets" "assets_icons_app")
add_to_manifest(${GENERATED_FILES})