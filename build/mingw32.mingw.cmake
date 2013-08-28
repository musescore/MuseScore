#
#     toolchain file for windows on windows
#

set (CMAKE_SYSTEM_NAME "Windows")

set (CROSS C:/Qt/Tools/mingw48_32)
set (CROSSQT C:/Qt/5.1.1/mingw48_32)

set (CMAKE_C_COMPILER     ${CROSS}/bin/gcc.exe)
set (CMAKE_CXX_COMPILER   ${CROSS}/bin/g++.exe)
set (CMAKE_STRIP          ${CROSS}/bin/strip.exe)
set (CMAKE_FIND_ROOT_PATH ${CROSS})

set (QT_INCLUDE_DIR           ${CROSSQT}/include)
set (QT_QT_INCLUDE_DIR        ${CROSSQT}/include/Qt)
set (QT_QTCORE_INCLUDE_DIR    ${CROSSQT}/include/QtCore)
set (QT_QTXML_INCLUDE_DIR     ${CROSSQT}/include/QtXml)
set (QT_QTXMLPATTERNS_INCLUDE_DIR ${CROSSQT}/include/QtXmlPatterns)
set (QT_QTGUI_INCLUDE_DIR     ${CROSSQT}/include/QtGui)
set (QT_QTNETWORK_INCLUDE_DIR ${CROSSQT}/include/QtNetwork)
set (QT_QTUITOOLS_INCLUDE_DIR ${CROSSQT}/include/QtUiTools)
set (QT_QTSCRIPT_INCLUDE_DIR  ${CROSSQT}/include/QtScript)
set (QT_QTWEBKIT_INCLUDE_DIR  ${CROSSQT}/include/QtWebkit)
set (QT_LIBRARY_DIR           ${CROSSQT}/lib)

set (QT_MOC_EXECUTABLE        "${CROSSQT}/bin/moc.exe")
set (QT_UIC_EXECUTABLE        "${CROSSQT}/bin/uic.exe")
set (QT_RCC_EXECUTABLE        "${CROSSQT}/bin/rcc")
set (QT_QTCORE_LIBRARY        "mops")

set (QT_WRC_EXECUTABLE        ${CMAKE_CURRENT_LIST_DIR}/wrc.bat)
set (QT_WINE_EXECUTABLE       ${CMAKE_CURRENT_LIST_DIR}/wine.bat)


set (QT_INCLUDES ${QT_INCLUDE_DIR} ${QT_QT_INCLUDE_DIR}
     ${QT_QTCORE_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR} ${QT_GUI_INCLUDE_DIR}
     ${QT_QTNETWORK_INCLUDE_DIR} ${QT_QTWEBKIT_INCLUDE_DIR}
     ${QT_QTXMLPATTERNS_INCLUDE_DIR}
     )
set (QT_mingw_LIBRARIES
    Qt5Svg
    Qt5Gui
    Qt5Core
    Qt5Widgets
    Qt5Xml
    Qt5Network
    Qt5WebKitWidgets
    Qt5WebKit
    Qt5V8
    Qt5XmlPatterns
    Qt5PrintSupport
    Qt5Quick
    Qt5Qml
    )

#    Qt3Support4

set (WIN32 ON)
set (MINGW ON)
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

