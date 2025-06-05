# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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

declare_module(muse_diagnostics)
set(MODULE_ALIAS muse::diagnostics)

set(MODULE_QRC diagnostics.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/diagnosticsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/diagnosticsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/diagnosticutils.h
    ${CMAKE_CURRENT_LIST_DIR}/idiagnosticspathsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/idiagnosticsconfiguration.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactionscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactionscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticspathsregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticspathsregister.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/isavediagnosticfilesscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/savediagnosticfilesscenario.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/savediagnosticfilesscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticfileswriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticfileswriter.h

    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticspathsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticspathsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticaccessiblemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticaccessiblemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/actionsviewmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/actionsviewmodel.h

    ${CMAKE_CURRENT_LIST_DIR}/view/system/profilerviewmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/system/profilerviewmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/system/graphicsinfomodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/system/graphicsinfomodel.h

    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/diagnosticnavigationmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/diagnosticnavigationmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/abstractkeynavdevitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/abstractkeynavdevitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevcontrol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevcontrol.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevsubsection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevsubsection.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevsection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevsection.h

    ${CMAKE_CURRENT_LIST_DIR}/devtools/crashhandlerdevtoolsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/devtools/crashhandlerdevtoolsmodel.h
    )

# --- Crashpad ---
# Not building with MinGW, so turned off for MinGW
include(GetCompilerInfo)
if (CC_IS_MINGW)
    set(BUILD_CRASHPAD_CLIENT OFF)
endif(CC_IS_MINGW)

if (MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT)
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/crashhandler/crashhandler.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/crashhandler/crashhandler.h
    )

    set(MODULE_INCLUDE_PRIVATE ${MODULE_INCLUDE_PRIVATE}
        ${CMAKE_CURRENT_LIST_DIR}/thirdparty/google_crashpad_client
        ${CMAKE_CURRENT_LIST_DIR}/thirdparty/google_crashpad_client/third_party/mini_chromium/mini_chromium
    )

    set(MODULE_DEF ${MODULE_DEF}
        -DMUSE_MODULE_DIAGNOSTICS_CRASHREPORT_URL="${MUSE_MODULE_DIAGNOSTICS_CRASHREPORT_URL}"
    )

    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/google_crashpad_client google_crashpad_client)

    set(MODULE_LINK ${MODULE_LINK}
        gcrashpad
    )

    # Install crashpad handler
    set(CPAD_ROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/crashpad_handler)
    if (OS_IS_LIN)
        if (NOT MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH)
            if (ARCH_IS_ARMV7L)
                set(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/linux/armv7l/crashpad_handler)
            elseif (ARCH_IS_AARCH64)
                set(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/linux/aarch64/crashpad_handler)
            elseif (ARCH_IS_X86_64)
                set(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/linux/x86-64/crashpad_handler)
            else ()
                message(WARNING "Architecture could not be detected. Using x86_64 as a fallback for crashpad_handler binary.")
                set(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/linux/x86-64/crashpad_handler)
            endif()
        endif()

        install(PROGRAMS ${MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH} DESTINATION ${INSTALL_SUBDIR})
    elseif(OS_IS_WIN)
        if (NOT MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH)
            set(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/windows/x86-64/crashpad_handler.exe)
        endif()

        install(PROGRAMS ${MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH} DESTINATION ${INSTALL_SUBDIR})
    elseif(OS_IS_MAC)
        if (NOT MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH)
            set(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/macos/crashpad_handler)
        endif()

        install(PROGRAMS ${MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH} DESTINATION ${INSTALL_SUBDIR})
    endif()
endif() # MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
# ----------------

set(MODULE_USE_COVERAGE OFF)

if (QT_SUPPORT)
    list(APPEND MODULE_LINK Qt::Gui Qt::Quick)
endif()

setup_module()

if (MUSE_MODULE_DIAGNOSTICS_TESTS)
    add_subdirectory(tests)
endif()
