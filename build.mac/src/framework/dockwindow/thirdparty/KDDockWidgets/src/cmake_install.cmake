# Install script for directory: /Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/zainazeeza/MuseScore/build.mac/src/framework/dockwindow/thirdparty/KDDockWidgets/src/libkddockwidgets-qt6.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libkddockwidgets-qt6.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libkddockwidgets-qt6.a")
    execute_process(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libkddockwidgets-qt6.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kddockwidgets-qt6/kddockwidgets" TYPE FILE FILES
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/docks_export.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/Config.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/FrameworkWidgetFactory.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/DockWidgetBase.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/KDDockWidgets.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/Qt5Qt6Compat_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/FocusScope.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/QWidgetAdapter.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/LayoutSaver.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/MainWindowMDI.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/MainWindowBase.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/DockWidgetQuick.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kddockwidgets-qt6/kddockwidgets/private" TYPE FILE FILES
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/DragController_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/Draggable_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/DropArea_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/DropIndicatorOverlayInterface_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/FloatingWindow_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/Frame_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/LayoutSaver_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/MultiSplitter_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/LayoutWidget_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/SideBar_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/TitleBar_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/WindowBeingDragged_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/WidgetResizeHandler_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/DockRegistry_p.h"
    "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/TabWidget_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kddockwidgets-qt6/kddockwidgets/private/multisplitter" TYPE FILE FILES "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/multisplitter/Item_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kddockwidgets-qt6/kddockwidgets/private/multisplitter" TYPE FILE FILES "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/multisplitter/Widget.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kddockwidgets-qt6/kddockwidgets/private/multisplitter" TYPE FILE FILES "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/multisplitter/Separator_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kddockwidgets-qt6/kddockwidgets/private/indicators" TYPE FILE FILES "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/indicators/ClassicIndicators_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kddockwidgets-qt6/kddockwidgets/private/indicators" TYPE FILE FILES "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/indicators/SegmentedIndicators_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kddockwidgets-qt6/kddockwidgets/private/quick" TYPE FILE FILES "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/quick/QWidgetAdapter_quick_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kddockwidgets-qt6/kddockwidgets/private/multisplitter" TYPE FILE FILES "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/multisplitter/Separator_quick.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kddockwidgets-qt6/kddockwidgets/private/multisplitter" TYPE FILE FILES "/Users/zainazeeza/MuseScore/src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/multisplitter/Widget_quick.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kddockwidgets-qt6/kddockwidgets" TYPE FILE FILES "/Users/zainazeeza/MuseScore/build.mac/src/framework/dockwindow/thirdparty/KDDockWidgets/src/kddockwidgets_version.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/KDDockWidgets-qt6/KDDockWidgets-qt6Targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/KDDockWidgets-qt6/KDDockWidgets-qt6Targets.cmake"
         "/Users/zainazeeza/MuseScore/build.mac/src/framework/dockwindow/thirdparty/KDDockWidgets/src/CMakeFiles/Export/469d0f163197a5755dd81629a59db3ef/KDDockWidgets-qt6Targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/KDDockWidgets-qt6/KDDockWidgets-qt6Targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/KDDockWidgets-qt6/KDDockWidgets-qt6Targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/KDDockWidgets-qt6" TYPE FILE FILES "/Users/zainazeeza/MuseScore/build.mac/src/framework/dockwindow/thirdparty/KDDockWidgets/src/CMakeFiles/Export/469d0f163197a5755dd81629a59db3ef/KDDockWidgets-qt6Targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/KDDockWidgets-qt6" TYPE FILE FILES "/Users/zainazeeza/MuseScore/build.mac/src/framework/dockwindow/thirdparty/KDDockWidgets/src/CMakeFiles/Export/469d0f163197a5755dd81629a59db3ef/KDDockWidgets-qt6Targets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/KDDockWidgets-qt6" TYPE FILE FILES
    "/Users/zainazeeza/MuseScore/build.mac/src/framework/dockwindow/thirdparty/KDDockWidgets/src/KDDockWidgets-qt6Config.cmake"
    "/Users/zainazeeza/MuseScore/build.mac/src/framework/dockwindow/thirdparty/KDDockWidgets/src/KDDockWidgets-qt6ConfigVersion.cmake"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/zainazeeza/MuseScore/build.mac/src/framework/dockwindow/thirdparty/KDDockWidgets/src/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
