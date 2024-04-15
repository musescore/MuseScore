

include(GetPlatformInfo)

if (OS_IS_WIN)

    if (MINGW)
          set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
    else (MINGW)
          set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    endif (MINGW)

    find_path(VORBIS_INCDIR vorbisenc.h PATHS ${PROJECT_SOURCE_DIR}/dependencies/include/vorbis;)
    find_library(VORBIS_LIB NAMES libvorbis PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)

    find_path(VORBISFILE_INCDIR vorbisfile.h PATHS ${PROJECT_SOURCE_DIR}/dependencies/include/vorbis;)
    find_library(VORBISFILE_LIB NAMES libvorbisfile PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)

    find_path(OGG_INCDIR ogg.h PATHS ${PROJECT_SOURCE_DIR}/dependencies/include/ogg;)
    find_library(OGG_LIB NAMES libogg PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)

else()

    include(UsePkgConfig1)
    PKGCONFIG1(vorbis 1.3.3 VORBIS_INCDIR VORBIS_LIBDIR VORBIS_LIB VORBIS_CPP)
    PKGCONFIG1(ogg 1.3.0 OGG_INCDIR OGG_LIBDIR OGG_LIB OGG_CPP)

endif()

if (VORBIS_INCDIR AND VORBIS_LIB)
    message(STATUS "Found vorbis: ${VORBIS_LIB} ${VORBIS_INCDIR}")
else()
    message(FATAL_ERROR "Could not find: vorbis")
endif()

if (OGG_INCDIR AND OGG_LIB)
    message(STATUS "Found ogg: ${OGG_LIB} ${OGG_INCDIR}")
else()
    message(FATAL_ERROR "Could not find: ogg")
endif()
