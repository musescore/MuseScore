
find_package(Qt5Core QUIET)

if (Qt5Core_FOUND)
  if (NOT Qt5Transitional_FIND_COMPONENTS)
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
        WebKit
        WebKitWidgets
        OpenGL
        LinguistTools
        Help
      )
    foreach(_component ${_components})
      find_package(Qt5${_component})

      list(APPEND QT_LIBRARIES ${Qt5${_component}_LIBRARIES})
    endforeach()
  else()
    set(_components ${Qt5Transitional_FIND_COMPONENTS})
    foreach(_component ${Qt5Transitional_FIND_COMPONENTS})
      find_package(Qt5${_component} REQUIRED)
      if ("${_component}" STREQUAL "WebKit")
        find_package(Qt5WebKitWidgets REQUIRED)
        list(APPEND QT_LIBRARIES ${Qt5WebKitWidgets_LIBRARIES} )
      endif()
      if ("${_component}" STREQUAL "Gui")
        find_package(Qt5Widgets REQUIRED)
        find_package(Qt5PrintSupport REQUIRED)
        find_package(Qt5Svg REQUIRED)
        list(APPEND QT_LIBRARIES ${Qt5Widgets_LIBRARIES}
                                 ${Qt5PrintSupport_LIBRARIES}
                                 ${Qt5Svg_LIBRARIES} )
      endif()
      if ("${_component}" STREQUAL "Core")
        find_package(Qt5Concurrent REQUIRED)
        list(APPEND QT_LIBRARIES ${Qt5Concurrent_LIBRARIES} )
      endif()
    endforeach()
  endif()

  set(Qt5Transitional_FOUND TRUE)
  set(QT5_BUILD TRUE)

  # Temporary until upstream does this:
  foreach(_component ${_components})
    if (TARGET Qt5::${_component})
      set_property(TARGET Qt5::${_component}
        APPEND PROPERTY
          INTERFACE_INCLUDE_DIRECTORIES ${Qt5${_component}_INCLUDE_DIRS})
      set_property(TARGET Qt5::${_component}
        APPEND PROPERTY
          INTERFACE_COMPILE_DEFINITIONS ${Qt5${_component}_COMPILE_DEFINITIONS})
    endif()
  endforeach()

  get_filename_component(_modules_dir "${CMAKE_CURRENT_LIST_DIR}/../build" ABSOLUTE)
  include("${_modules_dir}/ECMQt4To5Porting.cmake") # TODO: Port away from this.
  include_directories(${QT_INCLUDES}) # TODO: Port away from this.

  if (Qt5_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  endif()

else()
  foreach(_component ${Qt5Transitional_FIND_COMPONENTS})
    if("${_component}" STREQUAL "Widgets")  # new in Qt5
      set(_component Gui)
    elseif("${_component}" STREQUAL "Concurrent")   # new in Qt5
      set(_component Core)
    endif()
    list(APPEND _components Qt${_component})
  endforeach()
  include_directories(${QT_INCLUDES})

  if(QT4_FOUND)
    set(Qt5Transitional_FOUND TRUE)
  endif()
endif()

