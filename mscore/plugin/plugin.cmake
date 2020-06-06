
set (PLUGIN_API_SRC
    ${CMAKE_CURRENT_LIST_DIR}/api/cursor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/cursor.h
    ${CMAKE_CURRENT_LIST_DIR}/api/elements.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/elements.h
    ${CMAKE_CURRENT_LIST_DIR}/api/enums.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/enums.h
    ${CMAKE_CURRENT_LIST_DIR}/api/excerpt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/excerpt.h
    ${CMAKE_CURRENT_LIST_DIR}/api/fraction.h
    ${CMAKE_CURRENT_LIST_DIR}/api/instrument.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/instrument.h
    ${CMAKE_CURRENT_LIST_DIR}/api/part.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/part.h
    ${CMAKE_CURRENT_LIST_DIR}/api/playevent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/playevent.h
    ${CMAKE_CURRENT_LIST_DIR}/api/qmlpluginapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/qmlpluginapi.h
    ${CMAKE_CURRENT_LIST_DIR}/api/score.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/scoreelement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/scoreelement.h
    ${CMAKE_CURRENT_LIST_DIR}/api/score.h
    ${CMAKE_CURRENT_LIST_DIR}/api/selection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/selection.h
    ${CMAKE_CURRENT_LIST_DIR}/api/style.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/style.h
    ${CMAKE_CURRENT_LIST_DIR}/api/tie.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/tie.h
    ${CMAKE_CURRENT_LIST_DIR}/api/util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/util.h
    )

set (PLUGIN_SRC
    ${PLUGIN_API_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/mscorePlugins.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pluginCreator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pluginCreator.h
    ${CMAKE_CURRENT_LIST_DIR}/pluginManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pluginManager.h
    ${CMAKE_CURRENT_LIST_DIR}/qmledit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qmledit.h
    ${CMAKE_CURRENT_LIST_DIR}/qmliconview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qmliconview.h
    ${CMAKE_CURRENT_LIST_DIR}/qmlplugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qmlpluginengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qmlpluginengine.h
    ${CMAKE_CURRENT_LIST_DIR}/qmlplugin.h
    )

set (PLUGIN_UI
    ${CMAKE_CURRENT_LIST_DIR}/pluginManager.ui
    ${CMAKE_CURRENT_LIST_DIR}/pluginCreator.ui
    )
