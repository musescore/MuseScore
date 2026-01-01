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

muse_create_qml_module(playback_qml STUB FOR playback)

qt_add_qml_module(playback_qml
    URI MuseScore.Playback
    VERSION 1.0
    QML_FILES
        PlaybackToolBar.qml
        PlaybackLoadingInfo.qml
        MixerPanel.qml
        SoundFlagPopup.qml
        NotationRegionsBeingProcessedView.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)
