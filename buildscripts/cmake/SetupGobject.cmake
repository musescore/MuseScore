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
        pkg_check_modules(GOBJECT_PKGCONF QUIET gobject-2.0)
    endif()

    # Library
    find_library(GOBJECT_LIBRARY
        NAMES gobject-2.0
        PATHS ${GOBJECT_PKGCONF_LIBDIR}
        NO_DEFAULT_PATH
    )

    if (GOBJECT_LIBRARY AND GOBJECT_PKGCONF_INCLUDE_DIRS)
        set(GOBJECT_LIB ${GOBJECT_LIBRARY})
        set(GOBJECT_INCDIRS ${GOBJECT_PKGCONF_INCLUDE_DIRS})
    endif()
endif()

if (GOBJECT_INCDIRS)
    message(STATUS "Found gobject: ${GOBJECT_LIB}, ${GOBJECT_INCDIRS}")
else ()
    message(FATAL_ERROR "Could not find: gobject")
endif ()
