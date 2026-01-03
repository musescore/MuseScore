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

muse_create_qml_module(muse_cloud_qml ALIAS muse::cloud_qml FOR muse_cloud)

qt_add_qml_module(muse_cloud_qml
    URI Muse.Cloud
    VERSION 1.0
    SOURCES
        cloudsmodel.cpp
        cloudsmodel.h
        enums.cpp
        enums.h
        musescorecomauthorizationmodel.cpp
        musescorecomauthorizationmodel.h
    QML_FILES
        AccountAvatar.qml
        AccountInfoButton.qml
        AccountPage.qml
        CloudScoresView.qml
        internal/CloudItem.qml
        internal/CloudScoresGridView.qml
        internal/CloudScoresListView.qml
        internal/CloudsListView.qml
        RequireAuthorizationDialog.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_cloud_qml)
