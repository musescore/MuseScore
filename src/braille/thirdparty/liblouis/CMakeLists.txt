declare_module(liblouis)

add_subdirectory(tables)

set(MODULE_DEF
    -DTABLESDIR=dataPathPtr         # compileTranslationTable.c
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

set(MODULE_SRC
    # Alphabetical order please!
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/commonTranslationFunctions.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/compileTranslationTable.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/config.h
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/internal.h
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/liblouis.h
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/logging.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/lou_backTranslateString.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/lou_translateString.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/maketable.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/pattern.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/utils.c
)

set(MODULE_USE_PCH OFF)
set(MODULE_USE_UNITY OFF)
set(MODULE_USE_COVERAGE OFF)

setup_module()

target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -w)
