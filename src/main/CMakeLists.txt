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

# Setup main application
include(GetPlatformInfo)

# Common
set(EXECUTABLE_NAME mscore)

# Platform specific
if (PLATFORM_IS_WINDOWS)

    set(MSCORE_OUTPUT_NAME ${MUSESCORE_NAME}${MUSESCORE_VERSION_MAJOR})

    include(GetCompilerInfo)
    if (COMPILER_IS_MSVC)
        # MSVC recognizes a *.rc file and will invoke the resource compiler to link it
        set (ICON_RES_FILE ${CMAKE_CURRENT_LIST_DIR}/res/mscore.rc)

    endif(COMPILER_IS_MSVC)

    if (COMPILER_IS_MINGW)
        set(ICON_RES_FILE ${PROJECT_BINARY_DIR}/resfile.o)
        add_custom_command(
           OUTPUT ${PROJECT_BINARY_DIR}/resfile.o
           COMMAND ${CMAKE_RC_COMPILER} -i mscore.rc -o ${PROJECT_BINARY_DIR}/resfile.o
           DEPENDS ${PROJECT_SOURCE_DIR}/src/main/res/mscore.rc
           WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/main/res
           )
        set_source_files_properties(${PROJECT_BINARY_DIR}/resfile.o PROPERTIES generated true)

    endif(COMPILER_IS_MINGW)

elseif(PLATFORM_IS_LINUX)

    if (MSCORE_INSTALL_SUFFIX)
        set(MSCORE_OUTPUT_NAME "${EXECUTABLE_NAME}${MSCORE_INSTALL_SUFFIX}")
    endif(MSCORE_INSTALL_SUFFIX)

elseif(PLATFORM_IS_MACOS)

    set(EXECUTABLE_NAME mscore MACOSX_BUNDLE)
    set(MACOSX_BUNDLE_ICON_FILE mscore.icns)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER org.musescore.${MUSESCORE_NAME})
    set(MACOSX_BUNDLE_BUNDLE_NAME ${MUSESCORE_NAME})
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MUSESCORE_VERSION_FULL})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MUSESCORE_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${MUSESCORE_VERSION_FULL})
    set(MACOSX_BUNDLE_COPYRIGHT musescore.org)

else()
    message(FATAL_ERROR "Unsopported Platform: ${CMAKE_HOST_SYSTEM_NAME}")
endif()



if (APPLE)


#      if(CMAKE_BUILD_NUMBER)
#            set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${MUSESCORE_VERSION_FULL}.${CMAKE_BUILD_NUMBER})
#            set (MACOSX_BUNDLE_BUNDLE_VERSION ${MUSESCORE_VERSION_FULL}.${CMAKE_BUILD_NUMBER})
#      endif(CMAKE_BUILD_NUMBER)

endif (APPLE)

add_executable(${EXECUTABLE_NAME}
      ${ICON_RES_FILE}
      main.cpp
      modulessetup.cpp
      modulessetup.h
      )

# If MSCORE_OUTPUT_NAME is set (e.g, when cmake is called by the user), the output executable will be
# called MSCORE_OUTPUT_NAME instead of 'mscore'. This can be used to have MuseScore stable and unstable
# both installed in the same prefix on a Linux system.
if (MSCORE_OUTPUT_NAME)
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME ${MSCORE_OUTPUT_NAME})
endif (MSCORE_OUTPUT_NAME)

if (PLATFORM_IS_MACOS)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/build/MacOSXBundleInfo.plist.in)
endif (PLATFORM_IS_MACOS)



set(FREETYPE_LIB )
if (USE_SYSTEM_FREETYPE)
     set(FREETYPE_LIB ${FREETYPE_LIBRARIES})
else (USE_SYSTEM_FREETYPE)
     set(FREETYPE_LIB mscore_freetype)
endif (USE_SYSTEM_FREETYPE)

set(LINK_LIB
    ${FREETYPE_LIB}
    ${QT_LIBRARIES}
    global
    ui
    uicomponents
    fonts
    system
    network
    )

if (BUILD_TELEMETRY_MODULE)
    set(LINK_LIB ${LINK_LIB} telemetry)
endif(BUILD_TELEMETRY_MODULE)

set(LINK_LIB ${LINK_LIB}
    libmscore
    actions
    appshell
    cloud
    context
    shortcuts
    workspace
    audio
    midi
    midi_old
    userscores
    extensions
    languages
    importexport
    notation
    commonscene
    palette
    inspector
    playback
    instruments
    plugins
    )

if (BUILD_VST)
    set(LINK_LIB ${LINK_LIB} vst)
endif(BUILD_VST)

add_dependencies(${EXECUTABLE_NAME} workspaces)

target_link_libraries(${EXECUTABLE_NAME} ${LINK_LIB} )


if (MSCORE_OUTPUT_NAME)
    set(MSCORE_EXECUTABLE_NAME ${MSCORE_OUTPUT_NAME})
else (MSCORE_OUTPUT_NAME)
    set(MSCORE_EXECUTABLE_NAME ${EXECUTABLE_NAME})
endif (MSCORE_OUTPUT_NAME)
set(MSCORE_EXECUTABLE_NAME "${MSCORE_EXECUTABLE_NAME}" PARENT_SCOPE)


##
## Miscellaneous Microsoft Visual Studio settings
##
if (MSVC)

   # Force the "install" and "package" targets not to depend on the "all" target.
   set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
   set(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY true)

   # Set the startup project to "mscore".
   if (NOT ${CMAKE_VERSION} VERSION_LESS "3.6.0")
      set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mscore)
   endif ()

   # Set the debugging properties for the "mscore" project.
   file(TO_NATIVE_PATH "${CMAKE_INSTALL_PREFIX}/bin" VS_DEBUGGER_WORKING_DIRECTORY)
   if (NOT ${CMAKE_VERSION} VERSION_LESS "3.12.0")
      set_target_properties(${EXECUTABLE_NAME} PROPERTIES VS_DEBUGGER_COMMAND "${VS_DEBUGGER_WORKING_DIRECTORY}\\${MSCORE_EXECUTABLE_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
   endif ()
   if (NOT ${CMAKE_VERSION} VERSION_LESS "3.8.0")
      set_target_properties(${EXECUTABLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${VS_DEBUGGER_WORKING_DIRECTORY}")
   endif ()
   if (NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0")
      set_target_properties(${EXECUTABLE_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "--debug")
   endif ()

endif (MSVC)
