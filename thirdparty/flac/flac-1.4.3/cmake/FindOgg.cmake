find_package(PkgConfig)
pkg_check_modules(_OGG QUIET ogg)

find_path(OGG_INCLUDE_DIR
    NAMES "ogg/ogg.h"
    PATHS ${_OGG_INCLUDE_DIRS})

find_library(OGG_LIBRARY
    NAMES ogg libogg
    HINTS ${_OGG_LIBRARY_DIRS})

mark_as_advanced(
    OGG_INCLUDE_DIR
    OGG_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ogg
    REQUIRED_VARS OGG_INCLUDE_DIR OGG_LIBRARY
    VERSION_VAR _OGG_VERSION)

if(OGG_FOUND AND NOT TARGET Ogg::ogg)
    add_library(Ogg::ogg UNKNOWN IMPORTED)
    set_target_properties(Ogg::ogg PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${OGG_INCLUDE_DIR}"
        IMPORTED_LOCATION "${OGG_LIBRARY}")
endif()
