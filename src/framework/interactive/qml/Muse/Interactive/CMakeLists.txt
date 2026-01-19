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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_qml_module(muse_interactive_qml ALIAS muse::interactive_qml FOR muse_interactive)

qt_add_qml_module(muse_interactive_qml
    URI Muse.Interactive
    VERSION 1.0
    SOURCES
        dev/interactivetestsmodel.cpp
        dev/interactivetestsmodel.h
        interactiveqmltypes.cpp
        interactiveqmltypes.h
        errordetailsmodel.cpp
        errordetailsmodel.h
        progressdialogmodel.cpp
        progressdialogmodel.h
    QML_FILES
        dev/SampleDialog.qml
        ErrorDetailsView.qml
        FileDialog.qml
        FolderDialog.qml
        ProgressDialog.qml
        StandardDialog.qml
        StandardDialogPanel.qml
    IMPORTS
        QtQuick
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_interactive_qml)

# Necessary for the auto-generated sources
target_include_directories(muse_interactive_qml PRIVATE dev)
