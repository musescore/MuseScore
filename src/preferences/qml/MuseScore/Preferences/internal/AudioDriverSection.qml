/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick

import Muse.UiComponents

BaseSection {
    id: root

    title: qsTrc("preferences", "Audio")

    property int currentAudioDriverIndex: -1
    property var audioDrivers: null

    signal currentAudioDriverIndexChangeRequested(int newIndex)

    Row {
        spacing: 8

        ComboBoxWithTitle {
            title: qsTrc("preferences", "Audio driver")
            columnWidth: root.columnWidth

            visible: root.audioDrivers.length > 1

            currentIndex: root.currentAudioDriverIndex
            model: root.audioDrivers

            navigationName: "AudioDriverBox"
            navigationPanel: root.navigation
            navigationRow: 1

            onValueEdited: function(newIndex, newValue) {
                root.currentAudioDriverIndexChangeRequested(newIndex)
            }
        }
    }

    CommonAudioDriverConfiguration {
        columnWidth: root.columnWidth

        navigation: root.navigation
        navigationOrderStart: 2
    }
}
