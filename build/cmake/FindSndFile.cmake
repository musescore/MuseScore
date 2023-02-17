include(GetPlatformInfo)

if (OS_IS_WIN)
    find_path(SNDFILE_INCDIR sndfile.h PATHS ${PROJECT_SOURCE_DIR}/dependencies/include;)
    if (MINGW)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
    else (MINGW)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    endif (MINGW)
    find_library(SNDFILE_LIB NAMES sndfile libsndfile-1 PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
    if (MINGW)
        set(SNDFILE_DLL ${SNDFILE_LIB})
    else (MINGW)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
        find_library(SNDFILE_DLL NAMES sndfile libsndfile-1 PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
        message(STATUS "Found sndfile DLL: ${SNDFILE_DLL}")
    endif (MINGW)

elseif (OS_IS_WASM)
    set(LIBSND_PATH "" CACHE PATH "Path to libsnd sources")
    set(LIBOGG_PATH "" CACHE PATH "Path to libogg sources")
    set(LIBVORBIS_PATH "" CACHE PATH "Path to libogg sources")
    set(SNDFILE_INCDIR LIBSND_PATH)
    set(MODULE sndfile)

    set(MODULE_SRC
        ${LIBSND_PATH}/sndfile.c
        ${LIBSND_PATH}/sndfile.hh
        ${LIBSND_PATH}/command.c
        ${LIBSND_PATH}/common.c
        ${LIBSND_PATH}/common.h
        ${LIBSND_PATH}/au.c
        ${LIBSND_PATH}/caf.c
        ${LIBSND_PATH}/file_io.c
        ${LIBSND_PATH}/ogg.c
        ${LIBSND_PATH}/ogg_vorbis.c

        #ogg
        ${LIBOGG_PATH}/include/ogg/ogg.h
        ${LIBOGG_PATH}/include/ogg/os_types.h
        ${LIBOGG_PATH}/src/bitwise.c
        ${LIBOGG_PATH}/src/framing.c

        #vorbis
        ${LIBVORBIS_PATH}/lib/vorbisenc.c
        ${LIBVORBIS_PATH}/lib/info.c
        ${LIBVORBIS_PATH}/lib/analysis.c
        ${LIBVORBIS_PATH}/lib/bitrate.c
        ${LIBVORBIS_PATH}/lib/block.c
        ${LIBVORBIS_PATH}/lib/codebook.c
        ${LIBVORBIS_PATH}/lib/envelope.c
        ${LIBVORBIS_PATH}/lib/floor0.c
        ${LIBVORBIS_PATH}/lib/floor1.c
        ${LIBVORBIS_PATH}/lib/lookup.c
        ${LIBVORBIS_PATH}/lib/lpc.c
        ${LIBVORBIS_PATH}/lib/lsp.c
        ${LIBVORBIS_PATH}/lib/mapping0.c
        ${LIBVORBIS_PATH}/lib/mdct.c
        ${LIBVORBIS_PATH}/lib/psy.c
        ${LIBVORBIS_PATH}/lib/registry.c
        ${LIBVORBIS_PATH}/lib/res0.c
        ${LIBVORBIS_PATH}/lib/sharedbook.c
        ${LIBVORBIS_PATH}/lib/smallft.c
        ${LIBVORBIS_PATH}/lib/vorbisfile.c
        ${LIBVORBIS_PATH}/lib/window.c
        ${LIBVORBIS_PATH}/lib/synthesis.c
        )

    set(MODULE_INCLUDE
        ${LIBSND_PATH}
        ${LIBOGG_PATH}/include
        ${LIBVORBIS_PATH}/include
        ${LIBVORBIS_PATH}/lib
        )

    include(${PROJECT_SOURCE_DIR}/build/module.cmake)

else()
    # Use pkg-config to get hints about paths
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(LIBSNDFILE_PKGCONF sndfile>=1.0.25 QUIET)
    endif()

    # Include dir
    find_path(LIBSNDFILE_INCLUDE_DIR
        NAMES sndfile.h
        PATHS ${LIBSNDFILE_PKGCONF_INCLUDEDIR}
        NO_DEFAULT_PATH
        )

    # Library
    find_library(LIBSNDFILE_LIBRARY
        NAMES sndfile libsndfile-1
        PATHS ${LIBSNDFILE_PKGCONF_LIBDIR}
        NO_DEFAULT_PATH
        )

    if (LIBSNDFILE_LIBRARY)
        set(SNDFILE_LIB ${LIBSNDFILE_LIBRARY})
        set(SNDFILE_INCDIR ${LIBSNDFILE_INCLUDE_DIR})
    endif()
endif()

if (SNDFILE_INCDIR)
    message(STATUS "Found sndfile: ${SNDFILE_LIB} ${SNDFILE_INCDIR}")
else ()
    message(FATAL_ERROR "Could not find: sndfile")
endif ()
