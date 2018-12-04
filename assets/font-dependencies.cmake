set(BUILT_IN_MSG "This tool should be built-in. Check your PATH environment variable.")
if(WIN32)
  required_program(REG "Access Windows Registry - ${BUILT_IN_MSG}" "reg")
  required_program(FINDSTR "Search for matching strings - ${BUILT_IN_MSG}" "findstr")
elseif(APPLE)
  required_program(SYSTEM_PROFILER "Report system configuration - ${BUILT_IN_MSG}" "system_profiler")
  required_program(GREP "Search for matching strings - ${BUILT_IN_MSG}" "grep")
elseif(UNIX) # Linux and friends
  required_program(FC_LIST "List available fonts - https://www.freedesktop.org/wiki/Software/fontconfig/" "fc-list")
  required_program(GREP "Search for matching strings - ${BUILT_IN_MSG}" "grep")
endif(WIN32)

function(required_font # ensure that a font dependency is installed
  CACHEV # cache this variable if font is found to avoid checking in future
  DESCRIPTION # build fails with this error message if font is missing
  FONT_NAME # the name of the font (e.g. "Roboto")
  )
  unset(${CACHEV}) # expose cached value (if any)
  if(NOT DEFINED ${CACHEV})
    if(WIN32)
      execute_process(
        COMMAND "${REG}" query "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"
        COMMAND "${FINDSTR}" /r "/c:^ *${FONT_NAME} (OpenType)" "/c:^ *${FONT_NAME} (TrueType)"
        RESULT_VARIABLE RET_VAL
        OUTPUT_QUIET
        )
    elseif(APPLE)
      execute_process(
        COMMAND "${SYSTEM_PROFILER}" "SPFontsDataType"
        COMMAND "${GREP}" -B9 -A13 "^ *Family: ${FONT_NAME}$"
        RESULT_VARIABLE RET_VAL
        OUTPUT_QUIET
        )
    elseif(UNIX) # Linux and friends
      execute_process(
        COMMAND "${FC_LIST}"
        COMMAND "${GREP}" -E "^[^:]+\\.(ttf|otf): ${FONT_NAME}:style="
        RESULT_VARIABLE RET_VAL
        OUTPUT_QUIET
        )
    endif(WIN32)
    if(RET_VAL STREQUAL "0") # string compare as RET_VAL not always a number
      # Font is installed so no need to check again on future CMake runs
      set(${CACHEV} "FOUND" CACHE INTERNAL "${FONT_NAME} - ${DESCRIPTION}")
    else(RET_VAL STREQUAL "0")
      # Fail the build, but use SEND_ERROR instead of FATAL_ERROR to allow other
      # checks to complete (show all missing dependencies, not just the first)
      message(SEND_ERROR "Font not found: ${FONT_NAME} - ${DESCRIPTION}")
    endif(RET_VAL STREQUAL "0")
  endif(NOT DEFINED ${CACHEV})
endfunction(required_font)

# Keep this list up-to-date. Run "grep -r font-family ." from the assets
# source directory to list fonts used in SVGs anywhere in the project.
required_font(ROBOTO_FONT "https://fonts.google.com/specimen/Roboto" "Times New Roman")
required_font(RALEWAY_FONT "https://fonts.google.com/specimen/Raleway" "Raleway")
