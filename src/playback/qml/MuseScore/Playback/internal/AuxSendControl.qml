/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0

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

            radius: root.height / 2

            from: 0
            to: 100
            stepSize: 1
            value: root.auxSendItemModel.audioSignalPercentage

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
            readonly property bool isHovering: mouseArea.containsMouse

            Layout.fillWidth: true
            Layout.fillHeight: true

            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
            navigation.accessible.name: isHovering ? root.accessibleName + " " + root.title + " " + qsTrc("playback", "Bypass") : root.title
            navigation.onActiveChanged: {
                if (navigation.active) {
                    root.navigateControlIndexChanged({row: navigation.row, column: navigation.column})
                }
            }

            text: isHovering ? "" : root.title
            icon: isHovering ? IconCode.BYPASS : IconCode.NONE

            accentButton: root.auxSendItemModel.isActive

            onClicked: {
                root.auxSendItemModel.isActive = !root.auxSendItemModel.isActive
            }
        }
    }
}
