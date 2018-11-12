set(GENERATED_FILES "") # empty list

function(splash SVG_FILE_IN BUILD)
  string(TOLOWER "${BUILD}" BUILD_LOWER)
  set(FILE_OUT "splash-${BUILD_LOWER}")
  configure_file("${SVG_FILE_IN}" "${FILE_OUT}-raw.svg") # substitute @BUILD@
  standalone_svg("${FILE_OUT}-raw.svg" "${FILE_OUT}-standalone.svg")
  optimize_svg("${FILE_OUT}-standalone.svg" "${FILE_OUT}.svg")
  list(APPEND GENERATED_FILES "${FILE_OUT}.svg")
  set(GENERATED_FILES "${GENERATED_FILES}" PARENT_SCOPE)
endfunction(splash)

splash(splash-stable.svg STABLE)
splash(splash-unstable.svg NIGHTLY)
splash(splash-unstable.svg DEVELOPMENT)

add_custom_target("assets_splash" DEPENDS ${GENERATED_FILES})
add_dependencies("assets_splash" "assets_icons_app") # splash contains app icon
add_dependencies("assets" "assets_splash")
add_to_manifest(${GENERATED_FILES})