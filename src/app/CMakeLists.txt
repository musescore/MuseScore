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

    set(EXECUTABLE_NAME MuseScoreStudio)

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

    # Always needed by everyone
    ${QT_LIBRARIES}
    muse::global
    muse::draw
    muse::actions

    # have stub, links real or stub
    muse::network
    muse::vst
    muse::update
    muse::accessibility
    muse::shortcuts
    muse::workspace
    muse::audio
    muse::mpe
    muse::midi
    muse::learn
    muse::tours
    muse::multiinstances
    muse::cloud
    muse::languages
    muse::extensions

    braille
    muse::diagnostics
    engraving
    context
    notation
    project
    commonscene
    palette
    inspector
    playback
    instrumentsscene
    musesounds
    )

macro(add_to_link_if_enabled opt lib)
    if (${opt})
        list(APPEND LINK_LIB ${lib})
    endif()
endmacro()

add_to_link_if_enabled(${MUSE_MODULE_AUTOBOT} muse::autobot)
add_to_link_if_enabled(${MUSE_MODULE_AUDIOPLUGINS} muse::audioplugins)
add_to_link_if_enabled(${MUSE_MODULE_MUSESAMPLER} muse::musesampler)
add_to_link_if_enabled(${MUSE_MODULE_UI} muse::ui)
add_to_link_if_enabled(${MUSE_MODULE_UI} muse::uicomponents)
add_to_link_if_enabled(${MUSE_MODULE_UI} muse::dockwindow)

add_to_link_if_enabled(${MUE_BUILD_APPSHELL_MODULE} appshell)
add_to_link_if_enabled(${MUE_BUILD_CONVERTER_MODULE} converter)
add_to_link_if_enabled(${MUE_BUILD_PRINT_MODULE} print)


if (MUE_BUILD_IMPORTEXPORT_MODULE)
    list(APPEND LINK_LIB
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

set (MSCORE_APPEND_SRC)

if (OS_IS_WIN)
    list(APPEND MSCORE_APPEND_SRC ${WINDOWS_ICONS_RC})
endif()

###########################################
# Resources
###########################################
qt_add_resources(APP_RCC_SOURCES app.qrc)

###########################################
# Executable declaration
###########################################

qt_add_executable(MuseScoreStudio
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

if (MUSE_ENABLE_UNIT_TESTS_CODE_COVERAGE)
    set(COVERAGE_FLAGS -fprofile-arcs -ftest-coverage --coverage)
    set(COVERAGE_LINK_FLAGS -lgcov --coverage)
    target_compile_options(MuseScoreStudio PRIVATE ${COVERAGE_FLAGS})
    target_link_options(MuseScoreStudio PRIVATE ${COVERAGE_LINK_FLAGS})
endif()

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

###########################################
# Includes
###########################################

target_include_directories(MuseScoreStudio PRIVATE
    ${PROJECT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}

    ${PROJECT_SOURCE_DIR}/src

    ${MUSE_FRAMEWORK_PATH}
    ${MUSE_FRAMEWORK_PATH}/framework
    ${MUSE_FRAMEWORK_PATH}/framework/global

    # compat
    ${MUSE_FRAMEWORK_PATH}/src
    ${MUSE_FRAMEWORK_PATH}/src/framework
    ${MUSE_FRAMEWORK_PATH}/src/framework/global
    # end compat
)

###########################################
# Link
###########################################

target_link_libraries(MuseScoreStudio PRIVATE ${LINK_LIB} ${COVERAGE_LINK_FLAGS})

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

    configure_file("${CMAKE_CURRENT_BINARY_DIR}/MuseScoreStudio.html" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MuseScoreStudio.html" COPYONLY)
    configure_file("${CMAKE_CURRENT_BINARY_DIR}/qtloader.js" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qtloader.js" COPYONLY)
    configure_file("${CMAKE_CURRENT_BINARY_DIR}/qtlogo.svg" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qtlogo.svg" COPYONLY)

else()
    message(FATAL_ERROR "Unsupported Platform: ${CMAKE_HOST_SYSTEM_NAME}")
endif()


#################################################
# Miscellaneous Microsoft Visual Studio settings
#################################################
if (MSVC)
    # Add a post-build event to deploy the application to the output folder. Same as building the INSTALL target.
    STRING(TOUPPER ${CMAKE_GENERATOR} UPPER_CMAKE_GENERATOR)
    if (${UPPER_CMAKE_GENERATOR} MATCHES "VISUAL STUDIO")
        # We want $(Configuration) passed verbatim. Note the round brackets.
        set(CONFIG_STR "$(Configuration)")

        add_custom_command(
            TARGET MuseScoreStudio POST_BUILD
            COMMAND ${CMAKE_COMMAND} -DBUILD_TYPE=${CONFIG_STR} -P cmake_install.cmake
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            VERBATIM
        )
    endif()

    # Force the "install" and "package" targets not to depend on the "all" target.
    set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
    set(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY true)

    # Set the startup project to "MuseScoreStudio".
    set_property(DIRECTORY "${PROJECT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT MuseScoreStudio)

    # Set the debugging properties for the "MuseScoreStudio" project.
    file(TO_NATIVE_PATH "${CMAKE_INSTALL_PREFIX}/bin" VS_DEBUGGER_WORKING_DIRECTORY)
    set_target_properties(MuseScoreStudio PROPERTIES VS_DEBUGGER_COMMAND "${VS_DEBUGGER_WORKING_DIRECTORY}\\${EXECUTABLE_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
    set_target_properties(MuseScoreStudio PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${VS_DEBUGGER_WORKING_DIRECTORY}")
    set_target_properties(MuseScoreStudio PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "--debug")
endif (MSVC)
