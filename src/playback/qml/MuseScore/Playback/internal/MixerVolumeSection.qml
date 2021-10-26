/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0

MixerPanelSection {
    id: root

    headerTitle: qsTrc("playback", "Volume")

    Item {
        id: content

        height: childrenRect.height
        width: root.delegateDefaultWidth

        property string accessibleName: (Boolean(root.needReadChannelName) ? item.title + " " : "") + root.headerTitle

        Component.onCompleted: {
            root.navigationRowEnd = root.navigationRowStart + 1
        }

        TextInputField {
            id: volumeTextInputField

            anchors.horizontalCenter: parent.horizontalCenter

            height: 24
            width: 46

            textHorizontalAlignment: Qt.AlignHCenter

            navigation.name: "VolumeInputField"
            navigation.panel: item.panel
            navigation.row: root.navigationRowStart
            navigation.accessible.name: content.accessibleName + " " + currentText
            navigation.onActiveChanged: {
                if (navigation.active) {
                    root.navigateControlNameChanged(navigation.name)
                }
            }

            validator: DoubleInputValidator {
                id: doubleInputValidator
                top: 12.0
                bottom: -60.0
                decimal: 1
            }

            currentText: Math.round(item.volumeLevel * 10) / 10

            onCurrentTextEdited: {
                if (item.volumeLevel !== Number(newTextValue)) {
                    item.volumeLevel = Number(newTextValue)
                }
            }
        }
    }
}
