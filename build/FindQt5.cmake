set(_components
    Core
    Gui
    #Designer
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
    OpenGL
    LinguistTools
    Help
  )
if (NOT MINGW)
  set(_components
    ${_components}
	WebEngine
    WebEngineCore
    WebEngineWidgets
	)
endif(NOT MINGW)
foreach(_component ${_components})
  find_package(Qt5${_component})
  list(APPEND QT_LIBRARIES ${Qt5${_component}_LIBRARIES})
  list(APPEND QT_INCLUDES ${Qt5${_component}_INCLUDE_DIRS})
  add_definitions(${Qt5${_component}_DEFINITIONS})
endforeach()

include_directories(${QT_INCLUDES})
set(QT_INSTALL_PREFIX ${_qt5Core_install_prefix})

#add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)
