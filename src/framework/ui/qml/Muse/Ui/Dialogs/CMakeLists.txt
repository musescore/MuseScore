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

muse_create_qml_module(muse_ui_dialogs_qml ALIAS muse::ui_dialogs_qml FOR muse_ui)

qt_add_qml_module(muse_ui_dialogs_qml
    URI Muse.Ui.Dialogs
    VERSION 1.0
    SOURCES
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

fixup_qml_module_dependencies(muse_ui_dialogs_qml)
