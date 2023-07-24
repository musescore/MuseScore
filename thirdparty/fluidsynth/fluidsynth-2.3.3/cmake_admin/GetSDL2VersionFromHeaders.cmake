# Get the version of SDL2 through headers
# This is needed for autotools builds before 2.0.12
function(get_sdl2_version_from_headers INCLUDE_DIR OUT_VAR)
  file(READ "${INCLUDE_DIR}/SDL_version.h" SDL_VERSION_H)
  string(REGEX MATCH "#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)"
               SDL2_MAJOR_RE "${SDL_VERSION_H}")
  set(SDL2_MAJOR "${CMAKE_MATCH_1}")
  string(REGEX MATCH "#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)"
               SDL2_MINOR_RE "${SDL_VERSION_H}")
  set(SDL2_MINOR "${CMAKE_MATCH_1}")
  string(REGEX MATCH "#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)" SDL2_PATCH_RE
               "${SDL_VERSION_H}")
  set(SDL2_PATCH "${CMAKE_MATCH_1}")
  if(SDL2_MAJOR_RE
     AND SDL2_MINOR_RE
     AND SDL2_PATCH_RE)
    set(${OUT_VAR}
        "${SDL2_MAJOR}.${SDL2_MINOR}.${SDL2_PATCH}"
        PARENT_SCOPE)
  endif()
endfunction()
