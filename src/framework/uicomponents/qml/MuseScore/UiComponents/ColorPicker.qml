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
import QtQuick.Dialogs 1.2

import MuseScore.Ui 1.0

Rectangle {
    id: root

    property bool isIndeterminate: false

    property alias navigation: navCtrl

    signal newColorSelected(var newColor)

    height: 30
    width: parent.width

    opacity: enabled ? 1 : ui.theme.itemOpacityDisabled

    radius: 3
    color: "#000000"

    border.width: navCtrl.active ? 2 : 1
    border.color: navCtrl.active ? ui.theme.focusColor : "#000000"

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "ColorPicker"
        enabled: root.enabled
        onTriggered: colorDialog.open()
    }

    StyledIconLabel {
        anchors.fill: parent
        iconCode: IconCode.QUESTION_MARK
        visible: isIndeterminate
    }

    MouseArea {
        id: clickableArea

        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            navCtrl.forceActive()
            colorDialog.open()
        }
    }

    ColorDialog {
        id: colorDialog

        currentColor: root.color
        modality: Qt.ApplicationModal

        onAccepted: {
            root.newColorSelected(colorDialog.color)
        }
    }

    states: [
        State {
            name: "NORMAL"
            when: !clickableArea.containsMouse && !colorDialog.visible

            PropertyChanges { target: root; border.color: ui.theme.buttonColor }
        },

        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed && !colorDialog.visible

            PropertyChanges { target: root; border.color: ui.theme.accentColor }
        },

        State {
            name: "PRESSED"
            when: clickableArea.pressed || colorDialog.visible

            PropertyChanges { target: root; border.color: ui.theme.fontPrimaryColor }
        }
    ]
}

