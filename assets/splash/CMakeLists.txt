set(GENERATED_FILES "") # empty list

function(splash # create splash screen image from a source SVG
  SVG_FILE_IN # source SVG that may contain @BUILD@ placeholder text
  BUILD # build type (text to set in place of @BUILD@ in the SVG)
  # ARGN remaining arguments are dependencies (resources or generated files)
  )
  string(TOLOWER "${BUILD}" BUILD_LOWER)
  set(FILE_OUT "splash-${BUILD_LOWER}")
  configure_file("${SVG_FILE_IN}" "${FILE_OUT}-raw.svg") # substitute @BUILD@
  standalone_svg("${FILE_OUT}-raw.svg" "${FILE_OUT}-standalone.svg")
  optimize_svg("${FILE_OUT}-standalone.svg" "${FILE_OUT}.svg")
  list(APPEND GENERATED_FILES "${FILE_OUT}.svg")
  set(GENERATED_FILES "${GENERATED_FILES}" PARENT_SCOPE)
endfunction(splash)

# splash screen for stable releases
splash(splash-stable.svg.xml.in "STABLE" "../platform/app-icons/musescore-icon-round.svg")

# splash screen for unstable builds
foreach(BUILD "NIGHTLY" "DEVELOPMENT")
  splash(splash-unstable.svg.xml.in "${BUILD}" "../platform/app-icons/musescore-icon-round.svg")
endforeach(BUILD)

add_custom_target("assets_splash" DEPENDS ${GENERATED_FILES})
add_dependencies("assets_splash" "assets_platform_appicons")
add_dependencies("assets" "assets_splash")
add_to_manifest(${GENERATED_FILES})
