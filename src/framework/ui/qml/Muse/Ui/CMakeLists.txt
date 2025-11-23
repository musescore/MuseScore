# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited
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

muse_create_qml_module(muse_ui_qml ALIAS muse::ui_qml FOR muse_ui)

qt_add_qml_module(muse_ui_qml
    URI Muse.Ui
    VERSION 1.0
    SOURCES
        abstractnavigation.cpp
        abstractnavigation.h
        foreign.cpp
        foreign.h
        graphicstestobject.cpp
        graphicstestobject.h
        navigationcontrol.cpp
        navigationcontrol.h
        navigationevent.cpp
        navigationevent.h
        navigationpanel.cpp
        navigationpanel.h
        navigationpopuppanel.cpp
        navigationpopuppanel.h
        navigationsection.cpp
        navigationsection.h
        qmlaccessible.cpp
        qmlaccessible.h
        qmldrag.cpp
        qmldrag.h
        windowsmodel.cpp
        windowsmodel.h
        dev/interactivetestsmodel.cpp
        dev/interactivetestsmodel.h

    QML_FILES
        InteractiveProvider.qml
        ToolTipProvider.qml

    IMPORTS
        QtQuick
)

# Necessary for the auto-generated sources
target_include_directories(muse_ui_qml PRIVATE dev internal)

if (MUSE_MODULE_UI_TESTS)
    add_subdirectory(tests)
endif()
