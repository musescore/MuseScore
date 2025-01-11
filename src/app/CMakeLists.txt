# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021-2024 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

###########################################
# Setup main application
###########################################

set(EXECUTABLE_NAME mscore)

###########################################
# Platform specific
###########################################
include(GetPlatformInfo)
if (OS_IS_WIN)
    set(EXECUTABLE_NAME ${MUSE_APP_NAME}${MUSE_APP_VERSION_MAJOR})

    include(GetCompilerInfo)

    if (CC_IS_MSVC)
        # MSVC recognizes a *.rc file and will invoke the resource compiler to link it
        set(WINDOWS_ICONS_RC ${PROJECT_SOURCE_DIR}/share/icons/windows_icons.rc)
    elseif(CC_IS_MINGW)
        set(WINDOWS_ICONS_RC ${PROJECT_BINARY_DIR}/windows_icons_rc.o)
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/windows_icons_rc.o
            COMMAND ${CMAKE_RC_COMPILER} -i windows_icons.rc -o ${PROJECT_BINARY_DIR}/windows_icons_rc.o
            DEPENDS ${PROJECT_SOURCE_DIR}/share/icons/windows_icons.rc
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/share/icons
            )
        set_source_files_properties(${PROJECT_BINARY_DIR}/windows_icons_rc.o PROPERTIES generated true)
    endif()

elseif(OS_IS_LIN)

    # add ssl support on local linux machine
    # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L /usr/local/lib -lcrypto -lssl")

    if (MUSE_APP_INSTALL_SUFFIX)
        set(EXECUTABLE_NAME "${EXECUTABLE_NAME}${MUSE_APP_INSTALL_SUFFIX}")
    endif(MUSE_APP_INSTALL_SUFFIX)

    set(CMAKE_INSTALL_RPATH "${QT_INSTALL_LIBS}")
    if (BUILD_SHARED_LIBS)
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${SHARED_LIBS_INSTALL_DESTINATION}")
    endif(BUILD_SHARED_LIBS)

elseif(OS_IS_MAC)

    set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER ${MUSE_APP_GUI_IDENTIFIER})
    set(MACOSX_BUNDLE_BUNDLE_NAME ${MUSE_APP_TITLE})
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MUSE_APP_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MUSE_APP_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_BUILD_NUMBER})
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 1999-2025 MuseScore Limited. Published under the GNU General Public License version 3.")

    set(CMAKE_INSTALL_RPATH ${QT_INSTALL_LIBS})
    if (MUE_COMPILE_MACOS_PRECOMPILED_DEPS_PATH)
        list(PREPEND CMAKE_INSTALL_RPATH "${MUE_COMPILE_MACOS_PRECOMPILED_DEPS_PATH}/lib")
    endif()
    
    message("MACOSX_BUNDLE_VERSION: ${MACOSX_BUNDLE_LONG_VERSION_STRING}.${MACOSX_BUNDLE_BUNDLE_VERSION}")

elseif(OS_IS_WASM)

    include_directories(${MODULE} PUBLIC
        ${PROJECT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/framework
        ${PROJECT_SOURCE_DIR}/src/framework/global
    )
    set(EMCC_LINKER_FLAGS "\
        -s WASM=1 \
        -s FULL_ES2=1 \
        -s FULL_ES3=1 \
        -s USE_WEBGL2=1 \
        -s USE_ZLIB=1 \
        -s EXIT_RUNTIME=1 \
        -s \"EXTRA_EXPORTED_RUNTIME_METHODS=['UTF16ToString', 'stringToUTF16']\" \
        --bind \
        -s FETCH=1 \
        -s FORCE_FILESYSTEM=1 \
        -g \
        -s ASSERTIONS=2 \
        -s DEMANGLE_SUPPORT=1 \
        -s GL_DEBUG=0 \
        -s ALLOW_MEMORY_GROWTH=1 \
        -s TOTAL_MEMORY=33554432 \
        -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
        -O3 \
    ")
elseif(OS_IS_FBSD)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L /usr/local/lib -lasound -lpthread")

    set(CMAKE_INSTALL_RPATH "${QT_INSTALL_LIBS}")
    if (BUILD_SHARED_LIBS)
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${SHARED_LIBS_INSTALL_DESTINATION}")
    endif(BUILD_SHARED_LIBS)
else()
    message(FATAL_ERROR "Unsupported Platform: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

###########################################
# Dependency library declaration
###########################################
set(LINK_LIB
    ${QT_LIBRARIES}
    muse::global
    muse::draw
    muse::network

    braille
    muse::diagnostics
    engraving
    muse::actions
    muse::accessibility
    context
    muse::shortcuts
    muse::workspace
    workspacescene
    muse::audio
    muse::mpe
    muse::midi
    muse::learn
    notation
    project
    print
    commonscene
    palette
    inspector
    playback
    instrumentsscene
    muse::vst
    muse::update
    )

if (MUSE_MODULE_UI)
    list(APPEND LINK_LIB muse::ui)
    list(APPEND LINK_LIB muse::uicomponents)
    list(APPEND LINK_LIB muse::dockwindow)
endif()

if (MUSE_MODULE_AUDIOPLUGINS)
    list(APPEND LINK_LIB muse::audioplugins)
endif()

if (MUE_BUILD_CONVERTER_MODULE)
    list(APPEND LINK_LIB converter)
endif()

if (MUE_BUILD_APPSHELL_MODULE)
    list(APPEND LINK_LIB appshell)
endif()

if (MUE_BUILD_MUSESOUNDS_MODULE)
    list(APPEND LINK_LIB musesounds)
endif()

if (NOT CC_IS_EMSCRIPTEN)
    list(APPEND LINK_LIB muse::cloud)
    list(APPEND LINK_LIB muse::languages)
    list(APPEND LINK_LIB muse::multiinstances)

    if (MUSE_MODULE_EXTENSIONS)
        list(APPEND LINK_LIB muse::extensions)
    endif()

    if (MUE_BUILD_IMPORTEXPORT_MODULE)
        set(LINK_LIB ${LINK_LIB}
            iex_musicxml
            iex_bb
            iex_bww
            iex_capella
            iex_guitarpro
            iex_midi
            iex_musedata
            iex_ove
            iex_audioexport
            iex_imagesexport
            iex_mei
        )

        if (MUE_BUILD_VIDEOEXPORT_MODULE)
            list(APPEND LINK_LIB iex_videoexport)
        endif()
    else()
        if (MUE_BUILD_IMAGESEXPORT_MODULE)
            list(APPEND LINK_LIB iex_imagesexport)
        endif()
    endif()

endif()


if (MUSE_MODULE_AUTOBOT)
    list(APPEND LINK_LIB muse::autobot)
endif()

if (MUSE_MODULE_MUSESAMPLER)
    list(APPEND LINK_LIB muse::musesampler)
endif()

set (MSCORE_APPEND_SRC)

if (OS_IS_WIN)
    list(APPEND MSCORE_APPEND_SRC ${WINDOWS_ICONS_RC})
endif()

if (CC_IS_EMSCRIPTEN)
    list(APPEND LINK_LIB wasmtest)

    foreach(link ${LINK_LIB} )
        list(APPEND MSCORE_APPEND_SRC $<TARGET_OBJECTS:${link}>)
    endforeach()

    list(APPEND MSCORE_APPEND_SRC $<TARGET_OBJECTS:sndfile>)
    list(APPEND MSCORE_APPEND_SRC $<TARGET_OBJECTS:fluidsynth>)
endif()

###########################################
# Resources
###########################################
qt_add_resources(APP_RCC_SOURCES app.qrc)
if (OS_IS_WIN)
    qt_add_resources(APP_RCC_SOURCES app_win.qrc)
endif()

###########################################
# Executable declaration
###########################################

add_executable(MuseScoreStudio
    WIN32 MACOSX_BUNDLE
    ${APP_RCC_SOURCES}
    ${MSCORE_APPEND_SRC}
    main.cpp
    cmdoptions.h
    appfactory.cpp
    appfactory.h

    internal/commandlineparser.cpp
    internal/commandlineparser.h
    internal/consoleapp.cpp
    internal/consoleapp.h
    internal/guiapp.cpp
    internal/guiapp.h
)

###########################################
# Setting target properties
###########################################

set_target_properties(MuseScoreStudio PROPERTIES OUTPUT_NAME ${EXECUTABLE_NAME})

if (OS_IS_MAC)
    set_target_properties(MuseScoreStudio PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/buildscripts/packaging/macOS/Info.plist.in
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${MACOSX_BUNDLE_GUI_IDENTIFIER})
endif (OS_IS_MAC)

if (CC_IS_MINGW)
    if (CMAKE_BUILD_TYPE MATCHES "DEBUG")
        set_target_properties(MuseScoreStudio PROPERTIES LINK_FLAGS "-mwindows -mconsole")
    else (CMAKE_BUILD_TYPE MATCHES "DEBUG")
        set_target_properties(MuseScoreStudio PROPERTIES LINK_FLAGS "-Wl,-S -mwindows")
    endif (CMAKE_BUILD_TYPE MATCHES "DEBUG")
endif(CC_IS_MINGW)

if (CC_IS_MSVC)
    target_link_options(MuseScoreStudio PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
endif(CC_IS_MSVC)

if (OS_IS_WASM)
    set_target_properties(MuseScoreStudio PROPERTIES LINK_FLAGS "${EMCC_LINKER_FLAGS}")
endif(OS_IS_WASM)


###########################################
# Link
###########################################

if (NOT CC_IS_EMSCRIPTEN)
    target_link_libraries(MuseScoreStudio ${LINK_LIB} )
else()
    target_link_libraries(MuseScoreStudio PRIVATE ${QT_LIBRARIES})

    find_package(Qt6QmlImportScanner REQUIRED)
    qt_import_qml_plugins(MuseScoreStudio PATH_TO_SCAN ${CMAKE_SOURCE_DIR})
endif()


###########################################
# INSTALL
###########################################

install(TARGETS MuseScoreStudio
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION bin
)

###########################################
# Windows
###########################################
if (OS_IS_WIN)
    include(GetCompilerInfo)

    if (CC_IS_MINGW)
        get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
        get_filename_component (MINGW_ROOT ${COMPILER_DIR} DIRECTORY)

        install( FILES
               ${MINGW_ROOT}/bin/libgcc_s_seh-1.dll
               ${MINGW_ROOT}/bin/libstdc++-6.dll
               ${MINGW_ROOT}/bin/libwinpthread-1.dll
               DESTINATION bin)

    endif(CC_IS_MINGW)

    # Install Qt
    foreach (QtLibrary ${QT_LIBRARIES})
        #message(STATUS "Library ${QtLibrary}")
        # always use release libs
        # TODO: remove this
        # That's not trivial, because for the libraries that are installed manually
        # below, we would need to manually specify the correct debug/release names.
        # The real solution would be to use windeployqt.
        set_target_properties(${QtLibrary} PROPERTIES MAP_IMPORTED_CONFIG_DEBUG "RELEASE")
        get_target_property(QtSharedLibrary ${QtLibrary} LOCATION_RELEASE)
        if (EXISTS ${QtSharedLibrary})
            list (APPEND QtInstallLibraries ${QtSharedLibrary})
        endif (EXISTS ${QtSharedLibrary})
    endforeach (QtLibrary ${QT_LIBRARIES})
    list(REMOVE_DUPLICATES QtInstallLibraries)

    install(FILES
            ${QtInstallLibraries}
            #${QT_INSTALL_BINS}/libEGL.dll
            #${QT_INSTALL_BINS}/libGLESv2.dll
            ${QT_INSTALL_BINS}/opengl32sw.dll
            ${QT_INSTALL_BINS}/d3dcompiler_47.dll
            ${QT_INSTALL_BINS}/Qt6QuickControls2Impl.dll
            ${QT_INSTALL_BINS}/Qt6QuickLayouts.dll
            ${QT_INSTALL_BINS}/Qt6QmlModels.dll
            ${QT_INSTALL_BINS}/Qt6QmlWorkerScript.dll
            DESTINATION bin)

    install(FILES
            ${QT_INSTALL_PLUGINS}/iconengines/qsvgicon.dll
            DESTINATION bin/iconengines)

    install(FILES
            ${QT_INSTALL_PLUGINS}/imageformats/qjpeg.dll
            ${QT_INSTALL_PLUGINS}/imageformats/qsvg.dll
            DESTINATION bin/imageformats)

    install(FILES
            ${QT_INSTALL_PLUGINS}/platforms/qwindows.dll
            ${QT_INSTALL_PLUGINS}/platforms/qoffscreen.dll
            DESTINATION bin/platforms)

    install(FILES
            ${QT_INSTALL_PLUGINS}/tls/qcertonlybackend.dll
            ${QT_INSTALL_PLUGINS}/tls/qopensslbackend.dll
            ${QT_INSTALL_PLUGINS}/tls/qschannelbackend.dll
            DESTINATION bin/tls)

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

    if (SNDFILE_DLL)
        install(FILES ${SNDFILE_DLL} DESTINATION bin)
    endif()

    # Install ssl
    install(FILES
            ${DEPENDENCIES_LIB_DIR}/libcrypto-1_1-x64.dll
            ${DEPENDENCIES_LIB_DIR}/libssl-1_1-x64.dll
            DESTINATION bin)

    if (WIN_PORTABLE)
        # deploy the files and directory structure needed for the PortableApps.com format
        install(DIRECTORY ${PROJECT_SOURCE_DIR}/buildscripts/packaging/Windows/PortableApps/App DESTINATION ${CMAKE_INSTALL_PREFIX}/../..)
        install(DIRECTORY ${PROJECT_SOURCE_DIR}/buildscripts/packaging/Windows/PortableApps/Other DESTINATION ${CMAKE_INSTALL_PREFIX}/../..)
        install(FILES ${PROJECT_SOURCE_DIR}/buildscripts/packaging/Windows/PortableApps/help.html DESTINATION ${CMAKE_INSTALL_PREFIX}/../..)
        configure_file(${PROJECT_SOURCE_DIR}/buildscripts/packaging/Windows/PortableApps/appinfo.ini.in     ${CMAKE_INSTALL_PREFIX}/../../App/AppInfo/appinfo.ini   @ONLY)
    endif (WIN_PORTABLE)

###########################################
# Linux
###########################################
elseif(OS_IS_LIN)
    # Install mscore executable (package maintainers may add "MuseScore" and/or "musescore" aliases that symlink to mscore)
    if (LN_EXECUTABLE)
       add_custom_target(mscore_alias ALL
           COMMAND echo "Creating symlink alias for mscore executable."
           COMMAND ${LN_EXECUTABLE} -sf "mscore${MUSE_APP_INSTALL_SUFFIX}" "musescore${MUSE_APP_INSTALL_SUFFIX}"
           COMMAND echo 'Symlink alias: musescore${MUSE_APP_INSTALL_SUFFIX} -> mscore${MUSE_APP_INSTALL_SUFFIX}'
           )
       install(FILES ${PROJECT_BINARY_DIR}/src/app/musescore${MUSE_APP_INSTALL_SUFFIX} DESTINATION bin)
    else (LN_EXECUTABLE)
       add_custom_target(mscore_alias ALL COMMAND echo "No symlink aliases will be created." VERBATIM )
    endif (LN_EXECUTABLE)

elseif(OS_IS_FBSD)
###########################################
# FreeBSD
###########################################
    add_custom_target(mscore_alias ALL
           COMMAND echo "Creating symlink alias for mscore executable."
           COMMAND ln -sfv "mscore" "musescore"
           )
   install(FILES ${PROJECT_BINARY_DIR}/src/app/musescore DESTINATION bin)

###########################################
# macOS
###########################################
elseif(OS_IS_MAC)
    install(FILES
            ${PROJECT_SOURCE_DIR}/fonts/bravura/BravuraText.otf
            ${PROJECT_SOURCE_DIR}/fonts/campania/Campania.otf
            ${PROJECT_SOURCE_DIR}/fonts/edwin/Edwin-BdIta.otf
            ${PROJECT_SOURCE_DIR}/fonts/edwin/Edwin-Bold.otf
            ${PROJECT_SOURCE_DIR}/fonts/edwin/Edwin-Italic.otf
            ${PROJECT_SOURCE_DIR}/fonts/edwin/Edwin-Roman.otf
            ${PROJECT_SOURCE_DIR}/fonts/gootville/GootvilleText.otf
            ${PROJECT_SOURCE_DIR}/fonts/FreeSans.ttf
            ${PROJECT_SOURCE_DIR}/fonts/FreeSerif.ttf
            ${PROJECT_SOURCE_DIR}/fonts/FreeSerifBold.ttf
            ${PROJECT_SOURCE_DIR}/fonts/FreeSerifItalic.ttf
            ${PROJECT_SOURCE_DIR}/fonts/FreeSerifBoldItalic.ttf
            ${PROJECT_SOURCE_DIR}/fonts/leland/Leland.otf
            ${PROJECT_SOURCE_DIR}/fonts/leland/LelandText.otf
            ${PROJECT_SOURCE_DIR}/fonts/mscore-BC.ttf
            ${PROJECT_SOURCE_DIR}/fonts/mscoreTab.ttf
            ${PROJECT_SOURCE_DIR}/fonts/mscore/MScoreText.ttf
            ${PROJECT_SOURCE_DIR}/fonts/musejazz/MuseJazzText.otf
            ${PROJECT_SOURCE_DIR}/fonts/petaluma/PetalumaScript.otf
            ${PROJECT_SOURCE_DIR}/fonts/petaluma/PetalumaText.otf
            ${PROJECT_SOURCE_DIR}/fonts/finalemaestro/FinaleMaestroText-Regular.otf
            ${PROJECT_SOURCE_DIR}/fonts/finalebroadway/FinaleBroadwayText.otf
            DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}fonts
    )

###########################################
# Wasm
###########################################
elseif(OS_IS_WASM)

    function(copy_html_js_launch_files target)
        set(APPNAME ${target})
        configure_file("${_qt6Core_install_prefix}/plugins/platforms/wasm_shell.html" "public_html/${target}.qt.html")
        configure_file("${_qt6Core_install_prefix}/plugins/platforms/qtloader.js" public_html/qtloader.js COPYONLY)
        configure_file("${_qt6Core_install_prefix}/plugins/platforms/qtlogo.svg" public_html/qtlogo.svg COPYONLY)
    endfunction()

    copy_html_js_launch_files(MuseScoreStudio)

else()
    message(FATAL_ERROR "Unsupported Platform: ${CMAKE_HOST_SYSTEM_NAME}")
endif()


#################################################
# Miscellaneous Microsoft Visual Studio settings
#################################################
if (MSVC)
    # Force the "install" and "package" targets not to depend on the "all" target.
    set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
    set(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY true)

    # Set the startup project to "MuseScoreStudio".
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MuseScoreStudio)

    # Set the debugging properties for the "MuseScoreStudio" project.
    file(TO_NATIVE_PATH "${CMAKE_INSTALL_PREFIX}/bin" VS_DEBUGGER_WORKING_DIRECTORY)
    set_target_properties(MuseScoreStudio PROPERTIES VS_DEBUGGER_COMMAND "${VS_DEBUGGER_WORKING_DIRECTORY}\\${MUSESCORE_EXECUTABLE_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
    set_target_properties(MuseScoreStudio PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${VS_DEBUGGER_WORKING_DIRECTORY}")
    set_target_properties(MuseScoreStudio PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "--debug")
endif (MSVC)
