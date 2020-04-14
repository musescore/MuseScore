find_path(OGG_INCLUDE_DIR ogg.h PATHS ${PROJECT_SOURCE_DIR}/dependencies/include/ogg;)

if (MINGW)
      set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
else (MINGW)
      set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
endif (MINGW)
find_library(OGG_LIBRARY NAMES libogg PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)

message(STATUS ${OGG_LIBRARY})

if (OGG_INCLUDE_DIR AND OGG_LIBRARY)
      set(OGG_FOUND TRUE)
endif (OGG_INCLUDE_DIR AND OGG_LIBRARY)

if (OGG_FOUND)
      message (STATUS "Found ogg: ${OGG_LIBRARY}")
else (OGG_FOUND)
      message (FATAL_ERROR "Could not find: ogg")
endif (OGG_FOUND)
