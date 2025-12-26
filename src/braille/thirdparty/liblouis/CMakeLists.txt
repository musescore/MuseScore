muse_create_thirdparty_module(liblouis)

add_subdirectory(tables)

target_compile_definitions(liblouis PRIVATE
    TABLESDIR=dataPathPtr           # compileTranslationTable.c
)

set(WIDECHARS_ARE_UCS4 TRUE)        # configMS.h
set(WIDECHAR_TYPE "unsigned int")   # liblouis.h.in

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/configMS.h
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/config.h
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/liblouis.h.in
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/liblouis.h
    @ONLY
)

target_sources(liblouis PRIVATE
    # Alphabetical order please!
    liblouis/commonTranslationFunctions.c
    liblouis/compileTranslationTable.c
    liblouis/config.h
    liblouis/internal.h
    liblouis/liblouis.h
    liblouis/logging.c
    liblouis/lou_backTranslateString.c
    liblouis/lou_translateString.c
    liblouis/maketable.c
    liblouis/pattern.c
    liblouis/utils.c
)

set_target_properties(liblouis PROPERTIES UNITY_BUILD OFF)

target_no_warning(liblouis -Wno-conversion)
target_no_warning(liblouis -w)
