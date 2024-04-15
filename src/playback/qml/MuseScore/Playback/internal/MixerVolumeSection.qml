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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Audio 1.0

MixerPanelSection {
    id: root

    headerTitle: qsTrc("playback", "Volume")

    Item {
        id: content

        height: childrenRect.height
        width: root.channelItemWidth

        property string accessibleName: (Boolean(root.needReadChannelName) ? channelItem.title + " " : "") + root.headerTitle

        TextInputField {
            id: volumeTextInputField

            anchors.horizontalCenter: parent.horizontalCenter

            height: 24
            width: 46

            textHorizontalAlignment: Qt.AlignHCenter
            textSidePadding: 0
            background.radius: 2

            navigation.name: "VolumeInputField"
            navigation.panel: channelItem.panel
            navigation.row: root.navigationRowStart
            navigation.accessible.name: content.accessibleName + " " + currentText
            navigation.onActiveChanged: {
                if (navigation.active) {
                    root.navigateControlIndexChanged({row: navigation.row, column: navigation.column})
                }
            }

            validator: DoubleInputValidator {
                id: doubleInputValidator
                top: 12.0
                bottom: -60.0
                decimal: 1
            }

            currentText: Math.round(channelItem.volumeLevel * 10) / 10

            onTextChanged: function(newTextValue) {
                if (channelItem.volumeLevel !== Number(newTextValue)) {
                    channelItem.volumeLevel = Number(newTextValue)
                }
            }
        }
    }
}
