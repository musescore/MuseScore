set(GENERATED_FILES "") # empty list

foreach(FILE musescore-logo-alpha)
  standalone_svg("${CMAKE_CURRENT_SOURCE_DIR}/${FILE}.svg" "${FILE}-standalone.svg")
  optimize_svg("${FILE}-standalone.svg" "${FILE}.svg")
  list(APPEND GENERATED_FILES "${FILE}.svg")
endforeach(FILE)

add_custom_target("assets_brand" DEPENDS ${GENERATED_FILES})
add_dependencies("assets" "assets_brand")
add_to_manifest(${GENERATED_FILES})