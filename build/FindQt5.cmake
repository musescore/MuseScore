set(_components
    Core
    Gui
    Designer
    Network
    Test
    Qml
    Quick
    QuickWidgets
    Xml
    XmlPatterns
    Svg
    Sql
    Widgets
    PrintSupport
    Concurrent
    UiTools
    OpenGL
    LinguistTools
    Help
  )
if (NOT MINGW)
  set(_components
    ${_components}
    WebEngine
    WebEngineCore
    WebEngineWidgets)
endif()
foreach(_component ${_components})
  find_package(Qt5${_component})
  list(APPEND QT_LIBRARIES ${Qt5${_component}_LIBRARIES})
  list(APPEND QT_INCLUDES ${Qt5${_component}_INCLUDE_DIRS})
endforeach()

include_directories(${QT_INCLUDES})

#add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)
