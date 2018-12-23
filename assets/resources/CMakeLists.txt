# list of resource files (in alphabetical order)
set(RESOURCES
  cymbals.svg
  folded-page.svg
  mscore.svg
  musicxml-logo-unofficial.svg
  splash-background.jpg
  )

assert_alphabetical(RESOURCES) # easier to maintain if we keep lists ordered

# copy into build directory so that links can be resolved at build time
foreach(FILE ${RESOURCES})
  copy_during_build("${CMAKE_CURRENT_SOURCE_DIR}/${FILE}" "${FILE}")
endforeach(FILE)

add_custom_target("assets_resources" ALL DEPENDS ${RESOURCES})
