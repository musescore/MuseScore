declare_module(braille)

add_subdirectory(tables)
add_subdirectory(thirdparty/liblouis)

set(MODULE_QRC ${CMAKE_CURRENT_LIST_DIR}/braille.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/braillemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/braillemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/ibrailleconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/ibrailleconverter.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationbraille.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/braillecode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/braillecode.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/brailleconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/brailleconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/brailleconverter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/brailleconverter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/brailleinput.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/brailleinput.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/brailleinputparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/brailleinputparser.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/braillewriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/braillewriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/braille.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/braille.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/louis.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/louis.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationbraille.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationbraille.h
    ${CMAKE_CURRENT_LIST_DIR}/view/braillemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/braillemodel.h
    )

set(MODULE_LINK
    engraving
    liblouis
    )

if (QT_SUPPORT)
    list(APPEND MODULE_LINK Qt::Qml)
endif()

setup_module()

if (MUE_BUILD_BRAILLE_TESTS)
    add_subdirectory(tests)
endif()
