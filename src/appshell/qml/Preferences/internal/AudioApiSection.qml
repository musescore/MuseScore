/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import QtQuick 2.15

import Muse.UiComponents 1.0

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Audio")

    property int currentAudioApiIndex: -1
    property var audioApiList: null

    signal currentAudioApiIndexChangeRequested(int newIndex)

    Row {
        spacing: 8

        ComboBoxWithTitle {
            id: apiComboBox

            property int initialIndex: -1

            title: qsTrc("appshell/preferences", "Audio API")
            columnWidth: root.columnWidth

            visible: root.audioApiList.length > 1

            currentIndex: root.currentAudioApiIndex
            model: root.audioApiList

            navigation.name: "AudioApiBox"
            navigation.panel: root.navigation
            navigation.row: 1

            onValueEdited: function(newIndex, newValue) {
                root.currentAudioApiIndexChangeRequested(newIndex)
            }

            onCurrentIndexChanged: {
                if (apiComboBox.initialIndex !== -1) {
                    restartRequiredLabel.visible = apiComboBox.currentIndex !== apiComboBox.initialIndex
                }
            }

            Component.onCompleted: {
                apiComboBox.initialIndex = apiComboBox.currentIndex
            }
        }

        StyledTextLabel {
            id: restartRequiredLabel

            anchors.verticalCenter: parent.verticalCenter

            text: qsTrc("appshell/preferences", "Restart required")
            visible: false
        }
    }

    CommonAudioApiConfiguration {
        columnWidth: root.columnWidth

        navigation: root.navigation
        navigationOrderStart: 2
    }
}
