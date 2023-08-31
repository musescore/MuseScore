set(_components
    Core
    Gui
    #Designer
    Network
    Test
    Qml
    Quick
    QuickControls2
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
if (USE_WEBENGINE)
  set(_components
    ${_components}
    WebEngine
    WebEngineCore
    WebEngineWidgets
	)
endif(USE_WEBENGINE)

if (WIN32)
    set(_components
      ${_components}
      WinExtras
      )
endif(WIN32)

find_package(Qt5Core ${QT_MIN_VERSION} REQUIRED)

if (Qt5Core_VERSION VERSION_GREATER_EQUAL 5.12.0)
    set(_components
      ${_components}
      QuickTemplates2
      )
endif(Qt5Core_VERSION VERSION_GREATER_EQUAL 5.12.0)

foreach(_component ${_components})
  find_package(Qt5${_component})
  list(APPEND QT_LIBRARIES ${Qt5${_component}_LIBRARIES})
  list(APPEND QT_INCLUDES ${Qt5${_component}_INCLUDE_DIRS})
  add_definitions(${Qt5${_component}_DEFINITIONS})
endforeach()

include_directories(${QT_INCLUDES})

find_program(QT_QMAKE_EXECUTABLE qmake)
set(_qmake_vars
    QT_INSTALL_ARCHDATA
    QT_INSTALL_BINS
    QT_INSTALL_CONFIGURATION
    QT_INSTALL_DATA
    QT_INSTALL_DOCS
    QT_INSTALL_EXAMPLES
    QT_INSTALL_HEADERS
    QT_INSTALL_IMPORTS
    QT_INSTALL_LIBEXECS
    QT_INSTALL_LIBS
    QT_INSTALL_PLUGINS
    QT_INSTALL_PREFIX
    QT_INSTALL_QML
    QT_INSTALL_TESTS
    QT_INSTALL_TRANSLATIONS
    )
foreach(_var ${_qmake_vars})
    execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} "-query" ${_var}
        RESULT_VARIABLE _return_val
        OUTPUT_VARIABLE _out
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    if(_return_val EQUAL 0)
        set(${_var} "${_out}")
    endif(_return_val EQUAL 0)
endforeach(_var)

#add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)
