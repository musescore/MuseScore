set(MODULE braille)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/braillemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/braillemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/braillewriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/braillewriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/braille.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/braille.h
    )

set(MODULE_LINK
    engraving
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

add_subdirectory(tables)
add_subdirectory(thirdparty/liblouis)

if (MUE_BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()
