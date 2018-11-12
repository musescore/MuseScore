function(zip_archive ARCHIVE)
  # any additional arguments will be passed to the zip utility (see ${ARGN})
  add_custom_command(
    OUTPUT "${ARCHIVE}"
    DEPENDS assets # is a target, not a file, so no issue with relative paths
    COMMAND "${CMAKE_COMMAND}" -E tar "cfv" "${ARCHIVE}" --format=zip ${ARGN}
    VERBATIM
  )
endfunction(zip_archive)

function(unzip_archive ARCHIVE)
  add_custom_target("assets_unzip"
    DEPENDS assets
    COMMAND "${CMAKE_COMMAND}" -E tar "xfv" "${ARCHIVE}"
    VERBATIM
  )
endfunction(unzip_archive)

function(create_directory DIRECTORY)
  # any additional arguments will be passed to the zip utility (see ${ARGN})
  add_custom_target(
    OUTPUT "${DIRECTORY}"
    DEPENDS assets # is a target, not a file, so no issue with relative paths
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${DIRECTORY}"
    VERBATIM
  )
endfunction(create_directory)