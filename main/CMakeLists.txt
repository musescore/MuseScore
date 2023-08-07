#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2019 MuseScore BVBA
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

if (MINGW)
      set (resource_file ${PROJECT_BINARY_DIR}/resfile.o)
endif (MINGW)
if (MSVC)
      # MSVC recognizes a *.rc file and will invoke the resource compiler to link it
      set (resource_file ${PROJECT_SOURCE_DIR}/mscore/data/mscore.rc)
endif ( MSVC )

if (APPLE)
      set (ExecutableName mscore MACOSX_BUNDLE)
      set (MACOSX_BUNDLE_ICON_FILE      mscore.icns)
      set (MACOSX_BUNDLE_GUI_IDENTIFIER org.musescore.MuseScore)
      set (MACOSX_BUNDLE_LONG_VERSION_STRING ${MUSESCORE_VERSION_FULL})
      set (MACOSX_BUNDLE_BUNDLE_NAME MuseScore)
      set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${MUSESCORE_VERSION})
      set (MACOSX_BUNDLE_BUNDLE_VERSION ${MUSESCORE_VERSION_FULL})
      if(CMAKE_BUILD_NUMBER)
            set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${MUSESCORE_VERSION_FULL}.${CMAKE_BUILD_NUMBER})
            set (MACOSX_BUNDLE_BUNDLE_VERSION ${MUSESCORE_VERSION_FULL}.${CMAKE_BUILD_NUMBER})
      endif(CMAKE_BUILD_NUMBER)
      set (MACOSX_BUNDLE_COPYRIGHT "Copyright © 1999-2023 MuseScore BVBA and others. Published under the GNU General Public License.")
else (APPLE)
      set (ExecutableName mscore)
endif (APPLE)

include_directories(
      ${PROJECT_SOURCE_DIR}/global
      ${PROJECT_BINARY_DIR}
      ${QTSINGLEAPPLICATION_INCLUDE_DIRS}
      )

add_executable(${ExecutableName}
      ${resource_file}
      main.cpp
      modulessetup.cpp
      modulessetup.h
      )

if (MINGW OR MSVC)
      set(MSCORE_OUTPUT_NAME ${MUSESCORE_NAME}${MUSESCORE_VERSION_MAJOR})
elseif (MSCORE_INSTALL_SUFFIX)
      set(MSCORE_OUTPUT_NAME "${ExecutableName}${MSCORE_INSTALL_SUFFIX}")
endif (MINGW OR MSVC)

# If MSCORE_OUTPUT_NAME is set (e.g, when cmake is called by the user), the output executable will be
# called MSCORE_OUTPUT_NAME instead of 'mscore'. This can be used to have MuseScore stable and unstable
# both installed in the same prefix on a Linux system.
if (MSCORE_OUTPUT_NAME)
  set_target_properties(
      ${ExecutableName}
      PROPERTIES
      OUTPUT_NAME ${MSCORE_OUTPUT_NAME}
      )
endif (MSCORE_OUTPUT_NAME)

target_include_directories(mscore PRIVATE
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_SOURCE_DIR}/thirdparty
      )

target_link_libraries(mscore mscoreapp global)


if (APPLE)
      set_target_properties (mscore
          PROPERTIES
          MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/build/MacOSXBundleInfo.plist.in)

      set_target_properties (mscore
          PROPERTIES
          XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET 10.10
          XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${MACOSX_BUNDLE_GUI_IDENTIFIER})

      # Enable dSym generation
      #set_target_properties (mscore
      #    PROPERTIES
      #       XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
endif (APPLE)


if (MINGW)
   add_custom_command(
      OUTPUT ${PROJECT_BINARY_DIR}/resfile.o
      COMMAND ${CMAKE_RC_COMPILER} -i mscore.rc -o ${PROJECT_BINARY_DIR}/resfile.o
      DEPENDS ${PROJECT_SOURCE_DIR}/mscore/data/mscore.rc
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/mscore/data
      )
   set_source_files_properties(
      ${PROJECT_BINARY_DIR}/resfile.o
      PROPERTIES generated true
      )
   string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
   # Windows: Add -mconsole to LINK_FLAGS to get a console window for debug output
   if(CMAKE_BUILD_TYPE MATCHES "DEBUG")
     set_target_properties( mscore
        PROPERTIES
           COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
           LINK_FLAGS "-mwindows -mconsole -L ${QT_INSTALL_LIBS}"
        )
   else(CMAKE_BUILD_TYPE MATCHES "DEBUG")
     set_target_properties( mscore
          PROPERTIES
             COMPILE_FLAGS "${PCH_INCLUDE} -Wall -Wextra -Winvalid-pch ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
             LINK_FLAGS "-Wl,-S -mwindows -L ${QT_INSTALL_LIBS}"
          )
   endif(CMAKE_BUILD_TYPE MATCHES "DEBUG")

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
         DESTINATION bin)
      if (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.12.4")
         install( FILES
            ${DEPENDENCIES_DIR}/libcrypto-1_1-x64.dll
            ${DEPENDENCIES_DIR}/libssl-1_1-x64.dll
            DESTINATION bin)
      endif (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.12.4")
   else (BUILD_64)
      install( FILES
         ${MINGW_ROOT}/bin/libgcc_s_dw2-1.dll
         DESTINATION bin)
      if (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.12.4")
         install( FILES
            ${DEPENDENCIES_DIR}/libcrypto-1_1.dll
            ${DEPENDENCIES_DIR}/libssl-1_1.dll
            DESTINATION bin)
      endif (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.12.4")
   endif (BUILD_64)

   install( FILES
      ${MINGW_ROOT}/bin/libstdc++-6.dll
      ${MINGW_ROOT}/bin/libwinpthread-1.dll
      ${OGG_LIBRARY}
      ${SNDFILE_LIBRARY}
      ${VORBIS_LIBRARY}
      ${VORBISFILE_LIBRARY}
      ${PORTAUDIO_LIBRARY}
      ${QT_INSTALL_BINS}/libEGL.dll
      ${QT_INSTALL_BINS}/libGLESv2.dll
      ${QT_INSTALL_BINS}/opengl32sw.dll
      ${QT_INSTALL_BINS}/d3dcompiler_47.dll
      ${QtInstallLibraries}
      ${PROJECT_SOURCE_DIR}/build/qt.conf
      DESTINATION bin)
  if (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.14.1")
     install( FILES
        ${QT_INSTALL_BINS}/Qt5QmlModels.dll
        ${QT_INSTALL_BINS}/Qt5QmlWorkerScript.dll
        DESTINATION bin)
   endif (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.14.1")
   if (Qt5Widgets_VERSION VERSION_LESS "5.12.4")
      install( FILES
         ${MINGW_ROOT}/opt/bin/libeay32.dll
         ${MINGW_ROOT}/opt/bin/ssleay32.dll
         DESTINATION bin)
   endif (Qt5Widgets_VERSION VERSION_LESS "5.12.4")

   install (FILES
      ${DEPENDENCIES_DIR}/lame_enc.dll
      DESTINATION bin
      OPTIONAL)

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
        xcode_pch(mscore all)
        install (TARGETS mscore BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})
        install (FILES "${PROJECT_SOURCE_DIR}/mscore/data/mscore.icns" DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})
        install (FILES "${PROJECT_SOURCE_DIR}/mscore/data/musescoreDocument.icns" DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})
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
           install( FILES ${PROJECT_BINARY_DIR}/main/musescore${MSCORE_INSTALL_SUFFIX} DESTINATION bin)
        else (LN_EXECUTABLE)
           add_custom_target(mscore_alias ALL
               COMMAND echo "No symlink aliases will be created."
               VERBATIM
               )
        endif (LN_EXECUTABLE)
        # Install MuseScore icons (use SVGs where possible, but install PNGs as backup for systems that don't support SVG)
        set(MSCORE_ICON_BASE ../assets/musescore-icon-round)
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
        install( FILES   ../assets/mscz-icon.svg RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}.svg
           DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .MSCZ files
        install( FILES   ../assets/mscz-icon-48.png RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}.png
           DESTINATION share/icons/hicolor/48x48/mimetypes) # PNG icon for .MSCZ files
        install( FILES   ../assets/mscx-icon.svg RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}+xml.svg
           DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .MSCX files
        install( FILES   ../assets/mscx-icon-48.png RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}+xml.png
           DESTINATION share/icons/hicolor/48x48/mimetypes) # PNG icon for .MSCX files
        # use a custom icon for MusicXML files (there isn't a standard icon for MusicXML files)
        install( FILES   ../assets/mxl-icon.svg RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}.svg
           DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .MXL (compressed MusicXML) files
        install( FILES   ../assets/mxl-icon-48.png RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}.png
           DESTINATION share/icons/hicolor/48x48/mimetypes) # PNG icon for .MXL (compressed MusicXML) files
        install( FILES   ../assets/xml-icon.svg RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}+xml.svg
           DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .XML (MusicXML) files
        install( FILES   ../assets/xml-icon-48.png RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}+xml.png
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
                  )
      if (USE_WEBENGINE)
         list(APPEND dlls_to_copy "${QT_INSTALL_BINS}/Qt5WebEngineWidgets.dll" "${QT_INSTALL_BINS}/Qt5WebEngineCore.dll")
      endif(USE_WEBENGINE)
      if (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.14.1")
         list(APPEND dlls_to_copy "${QT_INSTALL_BINS}/Qt5QmlModels.dll")
         list(APPEND dlls_to_copy "${QT_INSTALL_BINS}/Qt5QmlWorkerScript.dll")
      endif(Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.14.1")

      set(CMAKE_FIND_LIBRARY_PREFIX "")
      set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")

      if (WIN_SPARKLE_ENABLED)
            find_library( dll_winsparkle  NAMES "WinSparkle" PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
            list(APPEND dlls_to_copy ${dll_winsparkle})
      endif (WIN_SPARKLE_ENABLED)

      find_library( dll_lame        NAMES "lame_enc"       PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_ogg         NAMES "libogg"         PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_sndfile     NAMES "sndfile" "libsndfile-1"   PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_vorbis      NAMES "libvorbis"      PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_vorbisfile  NAMES "libvorbisfile"  PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_portaudio   NAMES "portaudio"      PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      if (Qt5Widgets_VERSION VERSION_LESS "5.12.4")
         find_library( dll_ssl1        NAMES "libeay32"       PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
         find_library( dll_ssl2        NAMES "ssleay32"       PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      else (Qt5Widgets_VERSION VERSION_LESS "5.12.4")
         find_library( dll_ssl1        NAMES "libcrypto-1_1-x64" "libcrypto-1_1"  PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
         find_library( dll_ssl2        NAMES "libssl-1_1-x64" "libssl-1_1"        PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      endif (Qt5Widgets_VERSION VERSION_LESS "5.12.4")

      list(APPEND dlls_to_copy ${dll_lame} ${dll_ogg} ${dll_sndfile} ${dll_vorbis} ${dll_vorbisfile} ${dll_portaudio} ${dll_ssl1} ${dll_ssl2} "$<TARGET_FILE_DIR:mscore>/${MSCORE_OUTPUT_NAME}.exe")
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
            ${dll_lame}
            ${dll_sndfile}
            ${dll_portaudio}
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
        install(DIRECTORY ../build/PortableApps/App DESTINATION ${CMAKE_INSTALL_PREFIX}/../..)
        install(DIRECTORY ../build/PortableApps/Other DESTINATION ${CMAKE_INSTALL_PREFIX}/../..)
        install(FILES ../build/PortableApps/help.html DESTINATION ${CMAKE_INSTALL_PREFIX}/../..)
        set(MSCORE_ICON_BASE ../assets/musescore-icon-round)
        configure_file(../build/PortableApps/appinfo.ini.in     ${CMAKE_INSTALL_PREFIX}/../../App/AppInfo/appinfo.ini   @ONLY)
        install(FILES ../mscore/data/mscore.ico RENAME appicon.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install(FILES ${MSCORE_ICON_BASE}-16.png RENAME appicon_16.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install(FILES ${MSCORE_ICON_BASE}-32.png RENAME appicon_32.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install(FILES ${MSCORE_ICON_BASE}-128.png RENAME appicon_128.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install( FILES   ../assets/mscz-icon.ico RENAME mscz.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # icon for .MSCZ files
        install( FILES   ../assets/mscz-icon-48.png RENAME mscz_48.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # PNG icon for .MSCZ files
        install( FILES   ../assets/mscx-icon.ico RENAME mscx.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # icon for .MSCX files
        install( FILES   ../assets/mscx-icon-48.png RENAME mscx_48.png DESTINATION  ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # PNG icon for .MSCX files
        # use a custom icon for MusicXML files (there isn't a standard icon for MusicXML files)
        # install( FILES   ../assets/mxl-icon.svg RENAME mxl.svg DESTINATION  ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # SVG icon for .MXL (compressed MusicXML) files
        # install( FILES   ../assets/mxl-icon-48.png RENAME mxl_48.png DESTINATION  ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # PNG icon for .MXL (compressed MusicXML) files
        # install( FILES   ../assets/xml-icon.svg RENAME xml.svg DESTINATION  ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # SVG icon for .XML (MusicXML) files
        # install( FILES   ../assets/xml-icon-48.png RENAME xml_48.png DESTINATION  ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons) # PNG icon for .XML (MusicXML) files
      endif (WIN_PORTABLE)

   endif ( NOT MSVC )
endif (MINGW)

if (APPLE)
     install (FILES
      ../fonts/gootville/GootvilleText.otf
      ../fonts/mscore/MScoreText.ttf
      ../fonts/musejazz/MuseJazzText.otf
      ../fonts/campania/Campania.otf
      ../fonts/edwin/Edwin-Roman.otf
      ../fonts/edwin/Edwin-Bold.otf
      ../fonts/edwin/Edwin-Italic.otf
      ../fonts/edwin/Edwin-BdIta.otf
      ../fonts/FreeSerif.ttf
      ../fonts/FreeSerifBold.ttf
      ../fonts/FreeSerifItalic.ttf
      ../fonts/FreeSerifBoldItalic.ttf
      ../fonts/FreeSans.ttf
      ../fonts/mscoreTab.ttf
      ../fonts/mscore-BC.ttf
      ../fonts/bravura/BravuraText.otf
      ../fonts/leland/LelandText.otf
      ../fonts/petaluma/PetalumaText.otf
      ../fonts/petaluma/PetalumaScript.otf
      ../fonts/finalemaestro/FinaleMaestroText-Regular.otf
      ../fonts/finalebroadway/FinaleBroadwayText.otf
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


if (MSCORE_OUTPUT_NAME)
      set (MSCORE_EXECUTABLE_NAME ${MSCORE_OUTPUT_NAME})
else (MSCORE_OUTPUT_NAME)
      set (MSCORE_EXECUTABLE_NAME ${ExecutableName})
endif (MSCORE_OUTPUT_NAME)
set (MSCORE_EXECUTABLE_NAME "${MSCORE_EXECUTABLE_NAME}" PARENT_SCOPE)
