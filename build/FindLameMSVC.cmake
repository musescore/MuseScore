find_path(LAMEMSVC_INCLUDE_DIR lame.h PATHS ${PROJECT_SOURCE_DIR}/dependencies/include/lame;)

find_library(LAMEMSVC_LIBRARY NAMES lame_enc PATHS ${PROJECT_SOURCE_DIR}/dependencies/libx86 NO_DEFAULT_PATH)

if (MINGW)
  set(LAMEMSVC_INCLUDE_DIR "")
  set(LAMEMSVC_LIBRARY "")
endif(MINGW)

message(STATUS ${LAMEMSVC_LIBRARY})

if (LAMEMSVC_INCLUDE_DIR AND LAMEMSVC_LIBRARY)
      set(LAME_FOUND TRUE)
endif (LAMEMSVC_INCLUDE_DIR AND LAMEMSVC_LIBRARY)

if (LAME_FOUND)
      message (STATUS "Found vorbis: ${LAMEMSVC_LIBRARY}")
else (LAME_FOUND)
      message (FATAL_ERROR "Could not find: lame")
endif (LAME_FOUND)
