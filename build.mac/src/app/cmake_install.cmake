# Install script for directory: /Users/zainazeeza/MuseScore/src/app

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE DIRECTORY FILES "/Users/zainazeeza/MuseScore/build.mac/src/app/mscore.app" USE_SOURCE_PERMISSIONS)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mscore.app/Contents/Resources/fonts" TYPE FILE FILES
    "/Users/zainazeeza/MuseScore/fonts/bravura/BravuraText.otf"
    "/Users/zainazeeza/MuseScore/fonts/campania/Campania.otf"
    "/Users/zainazeeza/MuseScore/fonts/edwin/Edwin-BdIta.otf"
    "/Users/zainazeeza/MuseScore/fonts/edwin/Edwin-Bold.otf"
    "/Users/zainazeeza/MuseScore/fonts/edwin/Edwin-Italic.otf"
    "/Users/zainazeeza/MuseScore/fonts/edwin/Edwin-Roman.otf"
    "/Users/zainazeeza/MuseScore/fonts/gootville/GootvilleText.otf"
    "/Users/zainazeeza/MuseScore/fonts/FreeSans.ttf"
    "/Users/zainazeeza/MuseScore/fonts/FreeSerif.ttf"
    "/Users/zainazeeza/MuseScore/fonts/FreeSerifBold.ttf"
    "/Users/zainazeeza/MuseScore/fonts/FreeSerifItalic.ttf"
    "/Users/zainazeeza/MuseScore/fonts/FreeSerifBoldItalic.ttf"
    "/Users/zainazeeza/MuseScore/fonts/leland/Leland.otf"
    "/Users/zainazeeza/MuseScore/fonts/leland/LelandText.otf"
    "/Users/zainazeeza/MuseScore/fonts/mscore-BC.ttf"
    "/Users/zainazeeza/MuseScore/fonts/mscoreTab.ttf"
    "/Users/zainazeeza/MuseScore/fonts/mscore/MScoreText.ttf"
    "/Users/zainazeeza/MuseScore/fonts/musejazz/MuseJazzText.otf"
    "/Users/zainazeeza/MuseScore/fonts/petaluma/PetalumaScript.otf"
    "/Users/zainazeeza/MuseScore/fonts/petaluma/PetalumaText.otf"
    "/Users/zainazeeza/MuseScore/fonts/finalemaestro/FinaleMaestroText-Regular.otf"
    "/Users/zainazeeza/MuseScore/fonts/finalebroadway/FinaleBroadwayText.otf"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/zainazeeza/MuseScore/build.mac/src/app/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
