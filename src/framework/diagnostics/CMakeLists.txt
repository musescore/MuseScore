# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(muse_diagnostics ALIAS muse::diagnostics NO_COVERAGE)

target_sources(muse_diagnostics PRIVATE
    diagnosticsmodule.cpp
    diagnosticsmodule.h
    diagnosticutils.h
    idiagnosticspathsregister.h
    idiagnosticsconfiguration.h

    internal/diagnosticsconfiguration.cpp
    internal/diagnosticsconfiguration.h
    internal/diagnosticsactions.cpp
    internal/diagnosticsactions.h
    internal/diagnosticsactionscontroller.cpp
    internal/diagnosticsactionscontroller.h
    internal/diagnosticspathsregister.cpp
    internal/diagnosticspathsregister.h

    internal/isavediagnosticfilesscenario.h
    internal/savediagnosticfilesscenario.cpp
    internal/savediagnosticfilesscenario.h
    internal/diagnosticfileswriter.cpp
    internal/diagnosticfileswriter.h
)

# --- Crashpad ---
# Not building with MinGW, so turned off for MinGW
include(GetCompilerInfo)
include(GetPaths)
if (CC_IS_MINGW)
    set(BUILD_CRASHPAD_CLIENT OFF)
endif(CC_IS_MINGW)

if (MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT)
    target_sources(muse_diagnostics PRIVATE
        internal/crashhandler/crashhandler.cpp
        internal/crashhandler/crashhandler.h
    )

    set_source_files_properties(
        internal/crashhandler/crashhandler.cpp
        PROPERTIES 
        SKIP_UNITY_BUILD_INCLUSION ON
    )

    target_include_directories(muse_diagnostics PRIVATE
        thirdparty/google_crashpad_client
        thirdparty/google_crashpad_client/third_party/mini_chromium/mini_chromium
    )

    target_compile_definitions(muse_diagnostics PRIVATE
        MUSE_MODULE_DIAGNOSTICS_CRASHREPORT_URL="${MUSE_MODULE_DIAGNOSTICS_CRASHREPORT_URL}"
    )

    add_subdirectory(thirdparty/google_crashpad_client google_crashpad_client)

    target_link_libraries(muse_diagnostics PRIVATE gcrashpad)

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

        install(PROGRAMS ${MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH} DESTINATION ${INSTALL_BIN_DIR})
    endif()
endif() # MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
# ----------------

if (MUSE_QT_SUPPORT)
    target_link_libraries(muse_diagnostics PRIVATE Qt::Gui Qt::Quick)
endif()

if (MUSE_MODULE_DIAGNOSTICS_TESTS)
    add_subdirectory(tests)
endif()

if (MUSE_MODULE_DIAGNOSTICS_QML)
    add_subdirectory(qml/Muse/Diagnostics)
endif()
