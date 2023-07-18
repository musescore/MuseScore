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

set(MODULE diagnostics)

set(MODULE_QRC diagnostics.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/diagnosticsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/diagnosticsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/diagnosticutils.h
    ${CMAKE_CURRENT_LIST_DIR}/idiagnosticspathsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/idiagnosticsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iengravingelementsprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/idiagnosticdrawprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/diagnosticserrors.h
    ${CMAKE_CURRENT_LIST_DIR}/diagnosticstypes.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactionscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactionscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticspathsregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticspathsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/engravingelementsprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/engravingelementsprovider.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/drawdata/diagnosticdrawprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/drawdata/diagnosticdrawprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/drawdata/drawdatagenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/drawdata/drawdatagenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/drawdata/drawdataconverter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/drawdata/drawdataconverter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/drawdata/drawdatacomparator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/drawdata/drawdatacomparator.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/isavediagnosticfilesscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/savediagnosticfilesscenario.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/savediagnosticfilesscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticfileswriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticfileswriter.h

    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticspathsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticspathsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticaccessiblemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticaccessiblemodel.h

    ${CMAKE_CURRENT_LIST_DIR}/view/system/profilerviewmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/system/profilerviewmodel.h

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

    ${CMAKE_CURRENT_LIST_DIR}/view/engraving/engravingelementsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/engraving/engravingelementsmodel.h

    ${CMAKE_CURRENT_LIST_DIR}/devtools/crashhandlerdevtoolsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/devtools/crashhandlerdevtoolsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/devtools/corruptscoredevtoolsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/devtools/corruptscoredevtoolsmodel.h
    )

# --- Crashpad ---
# Not building with MinGW, so turned off for MinGW
include(GetCompilerInfo)
if (CC_IS_MINGW)
    set(BUILD_CRASHPAD_CLIENT OFF)
endif(CC_IS_MINGW)

if (MUE_BUILD_CRASHPAD_CLIENT)
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/crashhandler/crashhandler.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/crashhandler/crashhandler.h
    )

    set(MODULE_INCLUDE ${MODULE_INCLUDE}
        ${PROJECT_SOURCE_DIR}/thirdparty/google_crashpad_client
        ${PROJECT_SOURCE_DIR}/thirdparty/google_crashpad_client/third_party/mini_chromium/mini_chromium
    )

    set(MODULE_DEF ${MODULE_DEF}
        -DMUE_BUILD_CRASHPAD_CLIENT
        -DMUE_CRASH_REPORT_URL="${MUE_CRASH_REPORT_URL}"
    )

    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/google_crashpad_client google_crashpad_client)

    set(MODULE_LINK ${MODULE_LINK}
        gcrashpad
    )

    # Install crashpad handler
    set(CPAD_ROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/crashpad_handler)
    if (OS_IS_LIN)
        if (NOT MUE_CRASHPAD_HANDLER_PATH)
            if (ARCH_IS_ARMV7L)
                set(MUE_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/linux/armv7l/crashpad_handler)
            elseif (ARCH_IS_AARCH64)
                set(MUE_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/linux/aarch64/crashpad_handler)
            elseif (ARCH_IS_X86_64)
                set(MUE_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/linux/x86-64/crashpad_handler)
            else ()
                message(WARNING "Architecture could not be detected. Using x86_64 as a fallback for crashpad_handler binary.")
                set(MUE_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/linux/x86-64/crashpad_handler)
            endif()
        endif()

        install(PROGRAMS ${MUE_CRASHPAD_HANDLER_PATH} DESTINATION bin)
    elseif(OS_IS_WIN)
        if (NOT MUE_CRASHPAD_HANDLER_PATH)
            set(MUE_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/windows/x86-64/crashpad_handler.exe)
        endif()

        install(PROGRAMS ${MUE_CRASHPAD_HANDLER_PATH} DESTINATION bin)
    elseif(OS_IS_MAC)
        if (NOT MUE_CRASHPAD_HANDLER_PATH)
            set(MUE_CRASHPAD_HANDLER_PATH ${CPAD_ROOT_PATH}/macos/x86-64/crashpad_handler)
        endif()

        install(PROGRAMS ${MUE_CRASHPAD_HANDLER_PATH} DESTINATION ${Mscore_SHARE_NAME}Contents/MacOS)
    endif()
endif(MUE_BUILD_CRASHPAD_CLIENT)
# ----------------

set(MODULE_INCLUDE ${MODULE_INCLUDE}
    ${PROJECT_SOURCE_DIR}/src/importexport/guitarpro
)

include(SetupModule)

if (MUE_BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()
