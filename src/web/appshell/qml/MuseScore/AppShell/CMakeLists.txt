# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited and others
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

muse_create_qml_module(appshell_qml FOR appshell)

qt_add_qml_module(appshell_qml
    URI MuseScore.AppShell
    VERSION 1.0
    SOURCES
        appmenumodel.cpp
        appmenumodel.h
        navigableappmenumodel.cpp
        navigableappmenumodel.h
    QML_FILES
        AppButtonBackground.qml
        AppMenuBar.qml
        AppWindow.qml
        DevFrame.qml
        Main.qml
        NotationFrame.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
        TARGET muse_interactive_qml
        TARGET muse_shortcuts_qml
        TARGET instrumentsscene_qml
        TARGET notationscene_qml
        TARGET palette_qml
        TARGET playback_qml
        TARGET propertiespanel_qml
)

fixup_qml_module_dependencies(appshell_qml)

target_link_libraries(appshell_qml PRIVATE muse::uicomponents_qml)

# we use QKeyMapper to fix https://github.com/musescore/MuseScore/issues/10181
find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
target_link_libraries(appshell_qml PRIVATE Qt6::GuiPrivate)
