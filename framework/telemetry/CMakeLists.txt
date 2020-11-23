#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2019 MuseScore BVBA and others
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

set(MODULE telemetry)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/telemetrysetup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/telemetrysetup.h
    ${CMAKE_CURRENT_LIST_DIR}/itelemetryservice.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/actioneventobserver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/actioneventobserver.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/telemetryservice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/telemetryservice.h
    ${CMAKE_CURRENT_LIST_DIR}/view/telemetrypermissiondialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/telemetrypermissiondialog.h
    ${CMAKE_CURRENT_LIST_DIR}/view/telemetrypermissionmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/telemetrypermissionmodel.h
    )

set(MODULE_QRC telemetry.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_INCLUDE
    ${PROJECT_SOURCE_DIR}/thirdparty/google_crashpad_client
    ${PROJECT_SOURCE_DIR}/thirdparty/google_crashpad_client/third_party/mini_chromium/mini_chromium
)

add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/qt-google-analytics qt-google-analytics)


set(MODULE_LINK
    qt-google-analytics
)

# --- Crashpad ---
# Not building with MinGW, so turned off for MinGW
include(${PROJECT_SOURCE_DIR}/build/compiler.cmake)
set(BUILD_CRASHPAD_CLIENT ON)
if (COMPILER_IS_MINGW)
    set(BUILD_CRASHPAD_CLIENT OFF)
endif(COMPILER_IS_MINGW)

if (BUILD_CRASHPAD_CLIENT)

    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/dump/crashhandler.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/dump/crashhandler.h
    )

    set(MODULE_INCLUDE ${MODULE_INCLUDE}
        ${PROJECT_SOURCE_DIR}/thirdparty/google_crashpad_client
        ${PROJECT_SOURCE_DIR}/thirdparty/google_crashpad_client/third_party/mini_chromium/mini_chromium
    )

    set(MODULE_DEF ${MODULE_DEF}
        -DBUILD_CRASHPAD_CLIENT
    )

    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/google_crashpad_client google_crashpad_client)

    set(MODULE_LINK ${MODULE_LINK}
        gcrashpad
    )

endif(BUILD_CRASHPAD_CLIENT)
# ----------------


include(${PROJECT_SOURCE_DIR}/build/module.cmake)

# === Install crashpad handler ===
set(CPAD_ROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/crashpad_handler)
if (PLATFORM_IS_LINUX)
    install(PROGRAMS ${CPAD_ROOT_PATH}/linux/x86-64/crashpad_handler DESTINATION bin)
elseif(PLATFORM_IS_WINDOWS)
    install(PROGRAMS ${CPAD_ROOT_PATH}/windows/x86-64/crashpad_handler.exe DESTINATION bin)
elseif(PLATFORM_IS_MACOS)
    install(PROGRAMS ${CPAD_ROOT_PATH}/macos/x86-64/crashpad_handler DESTINATION ${Mscore_SHARE_NAME}Contents/MacOS)
endif()

