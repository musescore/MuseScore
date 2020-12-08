#=============================================================================
#  MuseScore
#  Linux Music Score Editor
#
#  Copyright (C) 2002-2016 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

cmake_minimum_required(VERSION 3.16)

project(mscore LANGUAGES C CXX)

# Conflicted with PCH on Linux, will be resolve later
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}/build
    ${CMAKE_CURRENT_LIST_DIR}/build/cmake
    ${CMAKE_MODULE_PATH}
    )

# Setup version number and general build settings
set(MUSESCORE_REVISION "" CACHE STRING "Build revision")
set(MUSESCORE_BUILD_CONFIG "dev" CACHE STRING "Build config")
# Possible MUSESCORE_BUILD_CONFIG values:
# - dev     - for development/nightly builds
# - testing - for testing versions (alpha, beta, RC)
# - release - for stable release builds

option(BUILD_64 "Build 64 bit version of editor" ON)
option(BUILD_PORTABLEAPPS "Windows build for PortableApps.com" OFF)
option(BUILD_FOR_WINSTORE "Build for the Windows Store." OFF)
option(BUILD_AUTOUPDATE "Build with autoupdate support" OFF)

option(BUILD_TELEMETRY_MODULE "Build with telemetry module" ON)
set(TELEMETRY_TRACK_ID "" CACHE STRING "Telemetry track id")
set(CRASH_REPORT_URL "" CACHE STRING "URL where to send crash reports")

option(SOUNDFONT3    "Ogg Vorbis compressed fonts"        ON)  # Enable Ogg Vorbis compressed fonts, requires Ogg & Vorbis
option(DOWNLOAD_SOUNDFONT "Download the latest soundfont version as part of the build process" ON)

option(BUILD_UNIT_TESTS "Build gtest unit test" OFF)
option(PACKAGE_FILE_ASSOCIATION "File types association" OFF)

option(BUILD_PCH "Build using precompiled headers." OFF)
option(QML_LOAD_FROM_SOURCE "Load qml files from source (not resource)" OFF)

option(USE_SYSTEM_FREETYPE "Use system FreeType"          OFF) # requires freetype >= 2.5.2, does not work on win
option(BUILD_WEBENGINE "Built in webengine support" ON)
set(SCRIPT_INTERFACE  TRUE)

set(VST3_SDK_VERSION "3.7")
option(BUILD_VST "Build VST MODULE" OFF)
set(VST3_SDK_PATH "" CACHE PATH "Path to VST3_SDK. SDK version >= ${VST3_SDK_VERSION} required")

set(JACK_LONGNAME "JACK (Jack Audio Connection Kit)")
set(JACK_MIN_VERSION "0.98.0")
option(BUILD_JACK    "Build with support for ${JACK_LONGNAME} audio backend. JACK >= ${JACK_MIN_VERSION} will be needed." OFF)

#include(SetupCMakePolicy)
include(UsePkgConfig1) # Defines MACRO(PKGCONFIG1 _package _minVersion _include_DIR _link_DIR _link_FLAGS _cflags)

include(SetupBuildEnvironment)
include(GetPlatformInfo)
if (PLATFORM_IS_LINUX)
    include(TryUseCcache)
endif(PLATFORM_IS_LINUX)

include(${CMAKE_CURRENT_LIST_DIR}/config.cmake)

# We need this early, before FindQt5
if (BUILD_WEBENGINE)
   if (COMPILER_IS_MINGW)
      set(USE_WEBENGINE 0)
   else (COMPILER_IS_MINGW)
      set(USE_WEBENGINE 1)
   endif(COMPILER_IS_MINGW)
else (BUILD_WEBENGINE)
   set(USE_WEBENGINE 0)
endif (BUILD_WEBENGINE)

include_directories(${CMAKE_CURRENT_LIST_DIR}/build/qtfix)
set(QT_MIN_VERSION    "5.15.0")
include(FindQt5)
include_directories(${CMAKE_CURRENT_LIST_DIR}/build/qtfix)

include(SetupDependencies)

if (BUILD_JACK)
    include(FindJack)
endif(BUILD_JACK)


if (BUILD_PORTABLEAPPS)
    set(WIN_PORTABLE 1)
endif (BUILD_PORTABLEAPPS)

if (BUILD_FOR_WINSTORE)
    set(FOR_WINSTORE 1)
endif(BUILD_FOR_WINSTORE)

#
#  Sparkle/WinSparkle routine
#
if (BUILD_AUTOUPDATE)

    if (APPLE)
        set(MAC_APPCAST_URL "")
    endif (APPLE)

    include(SetupSparkle)
endif(BUILD_AUTOUPDATE)


# Download MuseScore SoundFont
if (DOWNLOAD_SOUNDFONT)
    include(DownloadSoundFont)
endif(DOWNLOAD_SOUNDFONT)

include(SetupFreetype)

if(SOUNDFONT3)
    include(SetupVorbisAndOgg)
endif(SOUNDFONT3)

##
## produce config.h file
##
configure_file (
      ${PROJECT_SOURCE_DIR}/build/config.h.in
      ${PROJECT_BINARY_DIR}/config.h
      )


#
#  Create precompiled header file
#
# Temporary OFF
set(BUILD_PCH OFF)
if (BUILD_PCH)

    include(CreatePrecompiledHeader)

    # all.h is expected in PROJECT_BINARY_DIR by subdirs, except for MSVC
    if (NOT MSVC)
       execute_process(
          COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/all.h ${PROJECT_BINARY_DIR}/all.h
          WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
          )
    endif (NOT MSVC)

    precompiled_header(QT_INCLUDES all ${BUILD_PCH})

    # MSVC does not require these targets, as all.h is not copied and the
    #   PCH generation is done per-project
    if (NOT MSVC)
       ADD_CUSTOM_TARGET(mops1 DEPENDS ${PROJECT_BINARY_DIR}/all.h)
       ADD_CUSTOM_TARGET(mops2 DEPENDS ${PCH})
    endif (NOT MSVC)

endif(BUILD_PCH)


##
##  Custom target for translation generation
##

## lupdate is used to generate the translation text files based off of the source code

add_custom_target(lupdate
   COMMAND ${PROJECT_SOURCE_DIR}/build/gen-qt-projectfile ${PROJECT_SOURCE_DIR} > mscore.pro
   COMMAND Qt5::lupdate ${PROJECT_BINARY_DIR}/mscore.pro
   COMMAND ${PROJECT_SOURCE_DIR}/build/gen-instruments-projectfile ${PROJECT_SOURCE_DIR}/share/instruments > instruments.pro
   COMMAND Qt5::lupdate ${PROJECT_BINARY_DIR}/instruments.pro
   COMMAND ${PROJECT_SOURCE_DIR}/build/gen-tours-projectfile ${PROJECT_SOURCE_DIR}/share/tours > tours.pro
   COMMAND Qt5::lupdate ${PROJECT_BINARY_DIR}/tours.pro
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

## lrelease is used to compile the translated text files generated by lupdate into binary resources

file(GLOB INSTRUMENTS_TS_FILES
   "share/locale/instruments*.ts"
)
file(GLOB MSCORE_TS_FILES
   "share/locale/mscore*.ts"
)
file(GLOB TOURS_TS_FILES
   "share/locale/tours*.ts"
)
file(GLOB QT_TS_FILES
   "share/locale/qt*.ts"
)

add_custom_target(lrelease
   COMMAND Qt5::lrelease ${INSTRUMENTS_TS_FILES}
   COMMAND Qt5::lrelease ${MSCORE_TS_FILES}
   COMMAND Qt5::lrelease ${TOURS_TS_FILES}
   COMMAND Qt5::lrelease ${QT_TS_FILES}
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)

add_subdirectory(share)
add_subdirectory(thirdparty/qzip)
add_subdirectory(src)

if (BUILD_UNIT_TESTS)
    define_property(TARGET PROPERTY OUTPUT_XML
        BRIEF_DOCS "List XML files outputed by google test."
        FULL_DOCS "List XML files outputed by google test."
    )

    add_subdirectory(thirdparty/googletest)
    enable_testing()

endif(BUILD_UNIT_TESTS)


# ========= Deploy ==========
include(${PROJECT_SOURCE_DIR}/build/CopyFilesMacros.cmake)

if (MINGW)

   # Keep dependencies in alphabetical order. Changes made to this list
   # might need to be made in "build/Linux+BSD/portable/copy-libs" too.
   get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
   get_filename_component (MINGW_ROOT ${COMPILER_DIR} DIRECTORY)
   foreach (QtLibrary ${QT_LIBRARIES})
      message("Library ${QtLibrary}")
      # always use release libs
      set_target_properties(${QtLibrary} PROPERTIES MAP_IMPORTED_CONFIG_DEBUG "RELEASE")
      get_target_property(QtSharedLibrary ${QtLibrary} LOCATION_RELEASE)
      if (EXISTS ${QtSharedLibrary})
          list (APPEND QtInstallLibraries ${QtSharedLibrary})
      endif (EXISTS ${QtSharedLibrary})
   endforeach (QtLibrary ${QT_LIBRARIES})
   list(REMOVE_DUPLICATES QtInstallLibraries)

   # target_link_libraries(mscore ${QT_LIBRARIES})

   install( TARGETS mscore RUNTIME DESTINATION bin )

   if (BUILD_64)
      install( FILES
         ${MINGW_ROOT}/bin/libgcc_s_seh-1.dll
         ${DEPENDENCIES_DIR}/libcrypto-1_1-x64.dll
         ${DEPENDENCIES_DIR}/libssl-1_1-x64.dll
         DESTINATION bin)
   else (BUILD_64)
      install( FILES
         ${MINGW_ROOT}/bin/libgcc_s_dw2-1.dll
         ${DEPENDENCIES_DIR}/libcrypto-1_1.dll
         ${DEPENDENCIES_DIR}/libssl-1_1.dll
         DESTINATION bin)
   endif (BUILD_64)

   install( FILES
      ${MINGW_ROOT}/bin/libstdc++-6.dll
      ${MINGW_ROOT}/bin/libwinpthread-1.dll
      ${OGG_LIBRARY}
      ${SNDFILE_LIBRARY}
      ${VORBIS_LIBRARY}
      ${VORBISFILE_LIBRARY}
      ${QT_INSTALL_BINS}/libEGL.dll
      ${QT_INSTALL_BINS}/libGLESv2.dll
      ${QT_INSTALL_BINS}/opengl32sw.dll
      ${QT_INSTALL_BINS}/d3dcompiler_47.dll
      ${QtInstallLibraries}
      ${PROJECT_SOURCE_DIR}/build/qt.conf
      ${QT_INSTALL_BINS}/Qt5QmlModels.dll
      ${QT_INSTALL_BINS}/Qt5QmlWorkerScript.dll
      DESTINATION bin)

    install(FILES
      ${QT_INSTALL_PLUGINS}/iconengines/qsvgicon.dll
      DESTINATION bin/iconengines)

    install(FILES
      ${QT_INSTALL_PLUGINS}/imageformats/qjpeg.dll
      ${QT_INSTALL_PLUGINS}/imageformats/qsvg.dll
      ${QT_INSTALL_PLUGINS}/imageformats/qtiff.dll
      DESTINATION bin/imageformats)

    install(FILES
      ${QT_INSTALL_PLUGINS}/platforms/qwindows.dll
      DESTINATION bin/platforms)

    install(FILES
      ${QT_INSTALL_PLUGINS}/printsupport/windowsprintersupport.dll
      DESTINATION bin/printsupport)

    install(FILES
      ${QT_INSTALL_PLUGINS}/sqldrivers/qsqlite.dll
      DESTINATION bin/sqldrivers)

    install(DIRECTORY
      ${QT_INSTALL_QML}
      DESTINATION .
      REGEX ".*d\\.dll" EXCLUDE
      REGEX ".*QtMultimedia.*" EXCLUDE
      REGEX ".*QtSensors.*" EXCLUDE
      REGEX ".*QtTest.*" EXCLUDE
      REGEX ".*QtWebkit.*" EXCLUDE)

else (MINGW)

   if ( NOT MSVC )
      ## install qwebengine core
      if (NOT APPLE AND USE_WEBENGINE)
         install(PROGRAMS
            ${QT_INSTALL_LIBEXECS}/QtWebEngineProcess
            DESTINATION bin
            )
         install(DIRECTORY
            ${QT_INSTALL_DATA}/resources
            DESTINATION lib/qt5
            )
         install(DIRECTORY
            ${QT_INSTALL_TRANSLATIONS}/qtwebengine_locales
            DESTINATION lib/qt5/translations
            )
      endif(NOT APPLE AND USE_WEBENGINE)

      set_target_properties (
         mscore
         PROPERTIES
            COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wno-overloaded-virtual -Winvalid-pch"
         )

      if (APPLE)
        target_link_libraries(mscore ${OsxFrameworks})
      else (APPLE)
          target_link_libraries(mscore rt)
      endif (APPLE)

      if (APPLE)
         set_target_properties(mscore
           PROPERTIES
              LINK_FLAGS "-stdlib=libc++"
           )
        if (BUILD_PCH)
            xcode_pch(mscore all)
        endif(BUILD_PCH)
        install (TARGETS mscore BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})
        install (FILES "${PROJECT_SOURCE_DIR}/src/main/res/mscore.icns" DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})
        install (FILES "${PROJECT_SOURCE_DIR}/src/main/res/musescoreDocument.icns" DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})
      else (APPLE)
        #### PACKAGING for Linux and BSD based systems (more in top-level CMakeLists.txt) ####
        # Install mscore executable (package maintainers may add "MuseScore" and/or "musescore" aliases that symlink to mscore)
        install( TARGETS mscore RUNTIME DESTINATION bin )
        if (LN_EXECUTABLE)
           add_custom_target(mscore_alias ALL
               COMMAND echo "Creating symlink alias for mscore executable."
               COMMAND ${LN_EXECUTABLE} -sf "mscore${MSCORE_INSTALL_SUFFIX}" "musescore${MSCORE_INSTALL_SUFFIX}"
               COMMAND echo 'Symlink alias: musescore${MSCORE_INSTALL_SUFFIX} -> mscore${MSCORE_INSTALL_SUFFIX}'
               )
           install( FILES ${PROJECT_BINARY_DIR}/src/main/musescore${MSCORE_INSTALL_SUFFIX} DESTINATION bin)
        else (LN_EXECUTABLE)
           add_custom_target(mscore_alias ALL
               COMMAND echo "No symlink aliases will be created."
               VERBATIM
               )
        endif (LN_EXECUTABLE)
        # Install MuseScore icons (use SVGs where possible, but install PNGs as backup for systems that don't support SVG)
        set(MSCORE_ICON_BASE ${PROJECT_SOURCE_DIR}/assets/musescore-icon-round)
        install(FILES ${MSCORE_ICON_BASE}.svg RENAME mscore${MSCORE_INSTALL_SUFFIX}.svg DESTINATION share/icons/hicolor/scalable/apps)
        install(FILES ${MSCORE_ICON_BASE}-16.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/16x16/apps)
        install(FILES ${MSCORE_ICON_BASE}-24.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/24x24/apps)
        install(FILES ${MSCORE_ICON_BASE}-32.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/32x32/apps)
        install(FILES ${MSCORE_ICON_BASE}-48.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/48x48/apps)
        install(FILES ${MSCORE_ICON_BASE}-64.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/64x64/apps)
        install(FILES ${MSCORE_ICON_BASE}-96.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/96x96/apps)
        install(FILES ${MSCORE_ICON_BASE}-128.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/128x128/apps)
        install(FILES ${MSCORE_ICON_BASE}-512.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/512x512/apps)
        # Install MIME (filetype) icons for each mimetype on Linux
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/mscz-icon.svg RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}.svg
           DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .MSCZ files
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/mscz-icon-48.png RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}.png
           DESTINATION share/icons/hicolor/48x48/mimetypes) # PNG icon for .MSCZ files
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/mscx-icon.svg RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}+xml.svg
           DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .MSCX files
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/mscx-icon-48.png RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}+xml.png
           DESTINATION share/icons/hicolor/48x48/mimetypes) # PNG icon for .MSCX files
        # use a custom icon for MusicXML files (there isn't a standard icon for MusicXML files)
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/mxl-icon.svg RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}.svg
           DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .MXL (compressed MusicXML) files
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/mxl-icon-48.png RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}.png
           DESTINATION share/icons/hicolor/48x48/mimetypes) # PNG icon for .MXL (compressed MusicXML) files
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/xml-icon.svg RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}+xml.svg
           DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .XML (MusicXML) files
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/xml-icon-48.png RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}+xml.png
           DESTINATION share/icons/hicolor/48x48/mimetypes) # PNG icon for .XML (MusicXML) files
        # Note: Must now run "gtk-update-icon-cache" to set the new icons. This is done in the Makefile.
      endif (APPLE)
   else ( NOT MSVC )
      # Microsoft Visual Studio-specific starts here!
      string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

      # Create list of directories to search for libraries
      foreach (item ${CMAKE_LIBRARY_PATH})
         string( APPEND all_library_paths " /LIBPATH:${item}" )
      endforeach()

      # Windows: Add /SUBSYSTEM:WINDOWS to LINK_FLAGS to avoid a console window in release
      if(CMAKE_BUILD_TYPE MATCHES "REL*")
        set_target_properties( mscore
           PROPERTIES
              COMPILE_FLAGS "${PCH_INCLUDE} ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
              LINK_FLAGS    "/LIBPATH:${QT_INSTALL_LIBS} ${all_library_paths}"
              LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
           )
      else(CMAKE_BUILD_TYPE MATCHES "REL*")
        set_target_properties( mscore
           PROPERTIES
              COMPILE_FLAGS "${PCH_INCLUDE} ${QT_DEFINITIONS} /DQT_SVG_LIB /DQT_GUI_LIB /DQT_XML_LIB /DQT_CORE_LIB"
              LINK_FLAGS    "/LIBPATH:${QT_INSTALL_LIBS} ${all_library_paths}"
              LINK_FLAGS "/SUBSYSTEM:CONSOLE"
           )
      endif(CMAKE_BUILD_TYPE MATCHES "REL*")

      # Copy DLL dependencies to .EXE DIRECTORY
      list(APPEND dlls_to_copy
                  "${QT_INSTALL_BINS}/Qt5Core.dll"  "${QT_INSTALL_BINS}/Qt5Gui.dll"  "${QT_INSTALL_BINS}/Qt5Help.dll"
                  "${QT_INSTALL_BINS}/Qt5Network.dll"  "${QT_INSTALL_BINS}/Qt5PrintSupport.dll"
                  "${QT_INSTALL_BINS}/Qt5Qml.dll"  "${QT_INSTALL_BINS}/Qt5Quick.dll"  "${QT_INSTALL_BINS}/Qt5Sql.dll"
                  "${QT_INSTALL_BINS}/Qt5QuickControls2.dll" "${QT_INSTALL_BINS}/Qt5QuickTemplates2.dll"
                  "${QT_INSTALL_BINS}/Qt5Svg.dll"  "${QT_INSTALL_BINS}/Qt5Widgets.dll"  "${QT_INSTALL_BINS}/Qt5Xml.dll"
                  "${QT_INSTALL_BINS}/Qt5XmlPatterns.dll"
                  "${QT_INSTALL_BINS}/Qt5WebChannel.dll"
                  "${QT_INSTALL_BINS}/Qt5QuickWidgets.dll" "${QT_INSTALL_BINS}/Qt5Positioning.dll"
                  "${QT_INSTALL_BINS}/libEGL.dll" "${QT_INSTALL_BINS}/libGLESv2.dll" "${QT_INSTALL_BINS}/opengl32sw.dll"
                  "${QT_INSTALL_BINS}/d3dcompiler_47.dll"
                  "${QT_INSTALL_BINS}/Qt5QmlModels.dll"
                  "${QT_INSTALL_BINS}/Qt5QmlWorkerScript.dll"
                  )
      if (USE_WEBENGINE)
         list(APPEND dlls_to_copy "${QT_INSTALL_BINS}/Qt5WebEngineWidgets.dll" "${QT_INSTALL_BINS}/Qt5WebEngineCore.dll")
      endif(USE_WEBENGINE)

      set(CMAKE_FIND_LIBRARY_PREFIX "")
      set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")

      if (WIN_SPARKLE_ENABLED)
            find_library( dll_winsparkle  NAMES "WinSparkle" PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
            list(APPEND dlls_to_copy ${dll_winsparkle})
      endif (WIN_SPARKLE_ENABLED)

      find_library( dll_ogg         NAMES "libogg"         PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_sndfile     NAMES "sndfile" "libsndfile-1"   PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_vorbis      NAMES "libvorbis"      PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_vorbisfile  NAMES "libvorbisfile"  PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_ssl1        NAMES "libcrypto-1_1-x64" "libcrypto-1_1"  PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_ssl2        NAMES "libssl-1_1-x64" "libssl-1_1"        PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)

      list(APPEND dlls_to_copy ${dll_ogg} ${dll_sndfile} ${dll_vorbis} ${dll_vorbisfile} ${dll_ssl1} ${dll_ssl2} "$<TARGET_FILE_DIR:mscore>/${MSCORE_OUTPUT_NAME}.exe")
      set( output_dir_for_dlls "${CMAKE_INSTALL_PREFIX}/bin")

      COPY_FILES_INTO_DIRECTORY_IF_CHANGED( "${dlls_to_copy}" ${output_dir_for_dlls}  mscore)

      # Keep dependencies in alphabetical order. Changes made to this list
      # might need to be made in "build/Linux+BSD/portable/copy-libs" too.
      get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
      get_filename_component (MINGW_ROOT ${COMPILER_DIR} DIRECTORY)

      foreach (QtLibrary ${QT_LIBRARIES})
         message("Library ${QtLibrary}")
         # always use release libs
         set_target_properties(${QtLibrary} PROPERTIES MAP_IMPORTED_CONFIG_DEBUG "RELEASE")
         get_target_property(QtSharedLibrary ${QtLibrary} LOCATION_RELEASE)
         if (EXISTS ${QtSharedLibrary})
             list (APPEND QtInstallLibraries ${QtSharedLibrary})
         endif (EXISTS ${QtSharedLibrary})
      endforeach (QtLibrary ${QT_LIBRARIES})
      list(REMOVE_DUPLICATES QtInstallLibraries)

      target_link_libraries(mscore ${QT_LIBRARIES})
      add_dependencies(mscore lrelease)

      install( FILES
            ${dll_ogg}
            ${dll_sndfile}
            ${dll_vorbis}
            ${dll_vorbisfile}
            ${dll_ssl1}
            ${dll_ssl2}
            ${dll_winsparkle}
            ${QtInstallLibraries}
            ${QT_INSTALL_BINS}/libEGL.dll
            ${QT_INSTALL_BINS}/libGLESv2.dll
            ${QT_INSTALL_BINS}/opengl32sw.dll
            ${QT_INSTALL_BINS}/d3dcompiler_47.dll
            ${QT_INSTALL_BINS}/Qt5Positioning.dll
            ${QT_INSTALL_BINS}/Qt5WebChannel.dll
            ${QT_INSTALL_BINS}/Qt5QuickTemplates2.dll
            ${QT_INSTALL_BINS}/Qt5QmlModels.dll
            ${QT_INSTALL_BINS}/Qt5QmlWorkerScript.dll
            ${PROJECT_SOURCE_DIR}/build/qt.conf
            DESTINATION bin)

      install(FILES
         ${QT_INSTALL_PLUGINS}/iconengines/qsvgicon.dll
         DESTINATION bin/iconengines)

      install(FILES
         ${QT_INSTALL_PLUGINS}/imageformats/qjpeg.dll
         ${QT_INSTALL_PLUGINS}/imageformats/qsvg.dll
         ${QT_INSTALL_PLUGINS}/imageformats/qtiff.dll
         DESTINATION bin/imageformats)

      install(FILES
         ${QT_INSTALL_PLUGINS}/platforms/qwindows.dll
         DESTINATION bin/platforms)

      install(FILES
         ${QT_INSTALL_PLUGINS}/printsupport/windowsprintersupport.dll
         DESTINATION bin/printsupport)

      install(FILES
         ${QT_INSTALL_PLUGINS}/sqldrivers/qsqlite.dll
         DESTINATION bin/sqldrivers)

      install( TARGETS mscore RUNTIME DESTINATION bin ) # this duplicate due to the correctly package step

      if (USE_WEBENGINE)
         install(FILES
            ${QT_INSTALL_LIBEXECS}/QtWebEngineProcess.exe
            DESTINATION bin
            )
         install(DIRECTORY
            ${QT_INSTALL_DATA}/resources
            DESTINATION bin/webengineresources
            )
         install(DIRECTORY
            ${QT_INSTALL_TRANSLATIONS}/qtwebengine_locales
            DESTINATION bin/webengineresources/translations
            )
      endif (USE_WEBENGINE)

      install(DIRECTORY
         ${QT_INSTALL_QML}
         DESTINATION .
         REGEX ".*d\\.dll" EXCLUDE
         REGEX ".pdb" EXCLUDE
         REGEX ".*QtMultimedia.*" EXCLUDE
         REGEX ".*QtSensors.*" EXCLUDE
         REGEX ".*QtTest.*" EXCLUDE
         REGEX ".*QtWebkit.*" EXCLUDE
         )

      if (WIN_PORTABLE)
        # deploy the files and directory structure needed for the PortableApps.com format
        install(DIRECTORY ${PROJECT_SOURCE_DIR}/build/PortableApps/App DESTINATION ${CMAKE_INSTALL_PREFIX}/../..)
        install(DIRECTORY ${PROJECT_SOURCE_DIR}/build/PortableApps/Other DESTINATION ${CMAKE_INSTALL_PREFIX}/../..)
        install(FILES ${PROJECT_SOURCE_DIR}/build/PortableApps/help.html DESTINATION ${CMAKE_INSTALL_PREFIX}/../..)
        set(MSCORE_ICON_BASE ${PROJECT_SOURCE_DIR}/assets/musescore-icon-round)
        configure_file(${PROJECT_SOURCE_DIR}/build/PortableApps/appinfo.ini.in     ${CMAKE_INSTALL_PREFIX}/../../App/AppInfo/appinfo.ini   @ONLY)
        install(FILES ${PROJECT_SOURCE_DIR}/src/main/res/mscore.ico RENAME appicon.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install(FILES ${MSCORE_ICON_BASE}-16.png RENAME appicon_16.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install(FILES ${MSCORE_ICON_BASE}-32.png RENAME appicon_32.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install(FILES ${MSCORE_ICON_BASE}-128.png RENAME appicon_128.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/mscz-icon.ico RENAME mscz.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # icon for .MSCZ files
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/mscz-icon-48.png RENAME mscz_48.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # PNG icon for .MSCZ files
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/mscx-icon.ico RENAME mscx.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # icon for .MSCX files
        install( FILES   ${PROJECT_SOURCE_DIR}/assets/mscx-icon-48.png RENAME mscx_48.png DESTINATION  ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # PNG icon for .MSCX files
        # use a custom icon for MusicXML files (there isn't a standard icon for MusicXML files)
        # install( FILES   ${PROJECT_SOURCE_DIR}/assets/mxl-icon.svg RENAME mxl.svg DESTINATION  ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # SVG icon for .MXL (compressed MusicXML) files
        # install( FILES   ${PROJECT_SOURCE_DIR}/assets/mxl-icon-48.png RENAME mxl_48.png DESTINATION  ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # PNG icon for .MXL (compressed MusicXML) files
        # install( FILES   ${PROJECT_SOURCE_DIR}/assets/xml-icon.svg RENAME xml.svg DESTINATION  ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # SVG icon for .XML (MusicXML) files
        # install( FILES   ${PROJECT_SOURCE_DIR}/assets/xml-icon-48.png RENAME xml_48.png DESTINATION  ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # PNG icon for .XML (MusicXML) files
      endif (WIN_PORTABLE)

   endif ( NOT MSVC )
endif (MINGW)

if (APPLE)
     install (FILES
      ${PROJECT_SOURCE_DIR}/fonts/gootville/GootvilleText.otf
      ${PROJECT_SOURCE_DIR}/fonts/mscore/MScoreText.ttf
      ${PROJECT_SOURCE_DIR}/fonts/mscore/MusescoreIcon.ttf
      ${PROJECT_SOURCE_DIR}/fonts/musejazz/MuseJazzText.otf
      ${PROJECT_SOURCE_DIR}/fonts/campania/Campania.otf
      ${PROJECT_SOURCE_DIR}/fonts/FreeSerif.ttf
      ${PROJECT_SOURCE_DIR}/fonts/FreeSerifBold.ttf
      ${PROJECT_SOURCE_DIR}/fonts/FreeSerifItalic.ttf
      ${PROJECT_SOURCE_DIR}/fonts/FreeSerifBoldItalic.ttf
      ${PROJECT_SOURCE_DIR}/fonts/FreeSans.ttf
      ${PROJECT_SOURCE_DIR}/fonts/mscoreTab.ttf
      ${PROJECT_SOURCE_DIR}/fonts/mscore-BC.ttf
      ${PROJECT_SOURCE_DIR}/fonts/bravura/BravuraText.otf
      ${PROJECT_SOURCE_DIR}/fonts/leland/Leland.otf
      ${PROJECT_SOURCE_DIR}/fonts/petaluma/PetalumaText.otf
      ${PROJECT_SOURCE_DIR}/fonts/petaluma/PetalumaScript.otf
     DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}fonts
     )
     install(DIRECTORY
      ${QT_INSTALL_QML}
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}
      REGEX ".*QtWebkit.*" EXCLUDE
      REGEX ".*QtTest.*" EXCLUDE
      REGEX ".*QtSensors.*" EXCLUDE
      REGEX ".*QtMultimedia.*" EXCLUDE
      REGEX ".*QtAudioEngine.*" EXCLUDE
      REGEX ".*_debug\\.dylib" EXCLUDE)
endif (APPLE)


# ====== Packaging ===============

##
##  Include packaging
##
include(SetupAppImagePackaging)
include(Packaging)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${CMAKE_INSTALL_PREFIX}/bin")
#set(CMAKE_INSTALL_RPATH "$ORIGIN")





## TEMP: Display all variables!
### message(STATUS "===========================================================")
### message(STATUS "VARIABLES:")
### message(STATUS "")
### get_cmake_property(_variableNames VARIABLES)
### list (SORT _variableNames)
### foreach (_variableName ${_variableNames})
###     message(STATUS "${_variableName}=${${_variableName}}")
### endforeach()
### message(STATUS "===========================================================")
