include(GetPlatformInfo)

if (OS_IS_WIN AND (NOT MINGW))
    # TODO
    return()

elseif (OS_IS_WASM)
    # TODO
    return()

else()
    # Use pkg-config to get hints about paths
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(LIBINSTPATCH_PKGCONF QUIET libinstpatch-1.0>=1.1.0)
    endif()

    # Include dir
    find_path(LIBINSTPATCH_INCLUDE_DIR
        NAMES libinstpatch/libinstpatch.h
        PATHS ${LIBINSTPATCH_PKGCONF_INCLUDEDIR}
        NO_DEFAULT_PATH
    )

    # Library
    find_library(LIBINSTPATCH_LIBRARY
        NAMES instpatch-1.0 libinstpatch-1.0
        PATHS ${LIBINSTPATCH_PKGCONF_LIBDIR}
        # On Debian Sid LIBINSTPATCH_PKGCONF_LIBDIR wrongly points to
        # /usr/lib64, while the library actually is in /usr/lib/x86_64-linux-gnu
        # (default path), therefore not using NO_DEFAULT_PATH
    )

    if (LIBINSTPATCH_LIBRARY)
        set(INSTPATCH_LIB ${LIBINSTPATCH_LIBRARY})
        set(INSTPATCH_INCDIR ${LIBINSTPATCH_INCLUDE_DIR})
        set(LIBINSTPATCH_SUPPORT 1)
    endif()
endif()

if (LIBINSTPATCH_SUPPORT)
    message(STATUS "Found instpatch: ${INSTPATCH_LIB} ${INSTPATCH_INCDIR}")
else ()
    message(FATAL_ERROR "Could not find: instpatch")
endif ()
