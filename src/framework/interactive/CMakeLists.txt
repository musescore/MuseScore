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

muse_create_module(muse_interactive ALIAS muse::interactive)

target_sources(muse_interactive PRIVATE
    interactivemodule.cpp
    interactivemodule.h
    iinteractive.h
    iinteractiveuriregister.h
    interactivetypes.h

    api/interactiveapi.cpp
    api/interactiveapi.h

    dev/testdialog.cpp
    dev/testdialog.h
    dev/testdialog.ui

    internal/iinteractiveprovider.h
    internal/interactive.cpp
    internal/interactive.h
    internal/interactiveuriregister.cpp
    internal/interactiveuriregister.h
    internal/widgetdialogadapter.cpp
    internal/widgetdialogadapter.h
)

if (OS_IS_WASM)
    target_sources(muse_interactive PRIVATE
        internal/platform/web/webinteractive.cpp
        internal/platform/web/webinteractive.h
    )
else()
    if (OS_IS_MAC)
        target_sources(muse_interactive PRIVATE
            internal/platform/macos/macosinteractivehelper.mm
            internal/platform/macos/macosinteractivehelper.h
        )
        set_source_files_properties(
            internal/platform/macos/macosinteractivehelper.mm
            PROPERTIES
            SKIP_UNITY_BUILD_INCLUSION ON
            SKIP_PRECOMPILE_HEADERS ON
        )
        find_library(AppKit NAMES AppKit)
        target_link_libraries(muse_interactive PRIVATE ${AppKit})
    elseif(OS_IS_WIN)
        target_sources(muse_interactive PRIVATE
            internal/platform/win/wininteractivehelper.cpp
            internal/platform/win/wininteractivehelper.h
        )
    endif()
endif()

if (MUSE_QT_SUPPORT)
    target_link_libraries(muse_interactive PUBLIC Qt::Core Qt::Gui Qt::Quick Qt::Widgets)
endif()

if (MUSE_MODULE_INTERACTIVE_TESTS)
    add_subdirectory(tests)
endif()

if (MUSE_MODULE_INTERACTIVE_QML)
    add_subdirectory(qml/Muse/Interactive)
endif()
