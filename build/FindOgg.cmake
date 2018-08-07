find_path(OGG_INCLUDE_DIR ogg.h PATHS ${PROJECT_SOURCE_DIR}/dependencies/include/ogg;)

find_library(OGG_LIBRARY NAMES ogg PATHS ${PROJECT_SOURCE_DIR}/dependencies/libx86 NO_DEFAULT_PATH)

if (MINGW)
  set(OGG_INCLUDE_DIR "")
  set(OGG_LIBRARY "")
endif(MINGW)

message(STATUS ${OGG_LIBRARY})

if (OGG_INCLUDE_DIR AND OGG_LIBRARY)
      set(OGG_FOUND TRUE)
endif (OGG_INCLUDE_DIR AND OGG_LIBRARY)

if (OGG_FOUND)
      message (STATUS "Found ogg: ${OGG_LIBRARY}")
else (OGG_FOUND)
      message (FATAL_ERROR "Could not find: ogg")
endif (OGG_FOUND)
