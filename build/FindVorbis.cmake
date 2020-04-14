find_path(VORBIS_INCLUDE_DIR vorbisenc.h PATHS ${PROJECT_SOURCE_DIR}/dependencies/include/vorbis;)

if (MINGW)
      set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
else (MINGW)
      set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
endif (MINGW)
find_library(VORBIS_LIBRARY NAMES libvorbis PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)

message(STATUS ${VORBIS_LIBRARY})

if (VORBIS_INCLUDE_DIR AND VORBIS_LIBRARY)
      set(VORBIS_FOUND TRUE)
endif (VORBIS_INCLUDE_DIR AND VORBIS_LIBRARY)

if (VORBIS_FOUND)
      message (STATUS "Found vorbis: ${VORBIS_LIBRARY}")
else (VORBIS_FOUND)
      message (FATAL_ERROR "Could not find: vorbis")
endif (VORBIS_FOUND)
