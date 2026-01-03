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

muse_create_qml_module(muse_update_qml ALIAS muse::update_qml FOR muse_update)

qt_add_qml_module(muse_update_qml
    URI Muse.Update
    VERSION 1.0
    SOURCES
        appupdatemodel.cpp
        appupdatemodel.h
    QML_FILES
        AppReleaseInfoDialog.qml
        AppUpdateProgressDialog.qml
        internal/AppReleaseInfoBottomPanel.qml
        internal/ReleaseNotesView.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_update_qml)
