set(SOURCE_FILES
  mscore-logo.svg.xml
  )

assert_alphabetical(SOURCE_FILES) # keep list ordered for easier maintenance

set(GENERATED_FILES "") # create empty list

foreach(FILE ${SOURCE_FILES})
  get_filename_component(FILEBASE "${FILE}" NAME_WE) # strip file extensions
  standalone_svg("${CMAKE_CURRENT_SOURCE_DIR}/${FILE}" "${FILEBASE}-standalone.svg" "../resources/mscore.svg")
  optimize_svg("${FILEBASE}-standalone.svg" "${FILEBASE}.svg")
  list(APPEND GENERATED_FILES "${FILEBASE}.svg")
endforeach(FILE)

# custom build commands above will only run if a target depends on their output
add_custom_target("assets_brand" DEPENDS ${GENERATED_FILES})
# must make sure the commands don't run until after the resources are built
add_dependencies("assets_brand" "assets_resources")
# ensure that this target gets built when the main assets target is built
add_dependencies("assets" "assets_brand")
add_to_manifest(${GENERATED_FILES})
