/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Audio 1.0

Item {
    id: root

    property QtObject auxSendItemModel: null

    readonly property string title: root.auxSendItemModel ? root.auxSendItemModel.title : ""

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0
    readonly property int navigationRowEnd: navigationRowStart + 2
    property string navigationName: ""
    property string accessibleName: ""

    signal navigateControlIndexChanged(var index)

    height: 24
    width: 96

    RowLayout {
        id: content

        anchors.fill: parent

        spacing: 8

        KnobControl {
            id: audioSignalAmountKnob

            radius: root.height / 2 + 1.5

            from: 0
            to: 100
            stepSize: 1
            value: root.auxSendItemModel.audioSignalPercentage

            accentControl: root.auxSendItemModel.isActive

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart
            navigation.accessible.name: root.accessibleName
            navigation.onActiveChanged: {
                if (navigation.active) {
                    root.navigateControlIndexChanged({row: navigation.row, column: navigation.column})
                }
            }

            onNewValueRequested: function(newValue) {
                root.auxSendItemModel.audioSignalPercentage = newValue
            }
        }

        FlatButton {
            id: bypassBtn

            readonly property bool isHovering: bypassBtn.mouseArea.containsMouse && bypassBtn.enabled

            Layout.fillWidth: true
            Layout.preferredHeight: audioSignalAmountKnob.backgroundHeight
            Layout.alignment: Qt.AlignTop

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
            navigation.accessible.name: bypassBtn.isHovering ? root.accessibleName + " " + root.title + " " + qsTrc("playback", "Bypass") : root.title
            navigation.onActiveChanged: {
                if (navigation.active) {
                    root.navigateControlIndexChanged({row: navigation.row, column: navigation.column})
                }
            }

            text: {
                if (audioSignalAmountKnob.mouseArea.pressed && audioSignalAmountKnob.enabled) {
                    return audioSignalAmountKnob.value + "%"
                }

                return bypassBtn.isHovering ? "" : root.title
            }

            icon: bypassBtn.isHovering ? IconCode.BYPASS : IconCode.NONE

            accentButton: root.auxSendItemModel.isActive

            onClicked: {
                root.auxSendItemModel.isActive = !root.auxSendItemModel.isActive
            }
        }
    }
}
