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
        pkg_check_modules(GLIB_PKGCONF QUIET glib-2.0)
    endif()

    # Library
    find_library(GLIB_LIBRARY
        NAMES glib-2.0
        PATHS ${GLIB_PKGCONF_LIBDIR}
        NO_DEFAULT_PATH
    )

    if (GLIB_LIBRARY AND GLIB_PKGCONF_INCLUDE_DIRS)
        set(GLIB_LIB ${GLIB_LIBRARY})
        set(GLIB_INCDIRS ${GLIB_PKGCONF_INCLUDE_DIRS})
    endif()
endif()

if (GLIB_INCDIRS)
    message(STATUS "Found glib: ${GLIB_LIB}, ${GLIB_INCDIRS}")
else ()
    message(FATAL_ERROR "Could not find: glib")
endif ()
