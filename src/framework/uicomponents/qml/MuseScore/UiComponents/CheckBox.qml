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
import QtQuick.Layouts 1.3
import MuseScore.Ui 1.0

FocusScope {
    id: root

    property bool checked: false
    property bool isIndeterminate: false

    property alias text: label.text
    property alias font: label.font
    property alias wrapMode: label.wrapMode

    property alias navigation: navCtrl

    signal clicked

    implicitHeight: contentRow.height
    implicitWidth: contentRow.width

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "CheckBox"
        enabled: root.enabled
        onActiveChanged: {
            if (!root.activeFocus) {
                root.forceActiveFocus()
            }
        }
        onTriggered: root.clicked()
    }

    RowLayout {
        id: contentRow

        height: Math.max(box.height, label.implicitHeight)

        spacing: 8

        Rectangle {
            id: box

            height: 20
            width: 20

            border.width: 1
            border.color: "#00000000"
            color: ui.theme.buttonColor

            radius: 2

            StyledIconLabel {
                anchors.fill: parent
                iconCode: root.isIndeterminate ? IconCode.MINUS : IconCode.TICK_RIGHT_ANGLE
                visible: root.checked || root.isIndeterminate
            }
        }

        StyledTextLabel {
            id: label

            Layout.preferredWidth: root.width > 0 ? Math.min(root.width, label.implicitWidth) : label.implicitWidth
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            maximumLineCount: 2

            visible: Boolean(text)
        }
    }

    MouseArea {
        id: clickableArea

        anchors.fill: contentRow
        anchors.margins: -4

        hoverEnabled: true

        onClicked: {
            root.clicked()
        }
    }

    states: [
        State {
            name: "FOCUSED"
            when: navCtrl.active

            PropertyChanges {
                target: box
                border.color: ui.theme.focusColor
                border.width: 2
            }
        },

        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed

            PropertyChanges {
                target: box
                opacity: ui.theme.buttonOpacityHover
                border.color: navCtrl.active ? ui.theme.focusColor : ui.theme.strokeColor
            }
        },

        State {
            name: "PRESSED"
            when: clickableArea.containsMouse && clickableArea.pressed

            PropertyChanges {
                target: box
                opacity: ui.theme.buttonOpacityHit
                border.color: navCtrl.active ? ui.theme.focusColor : ui.theme.strokeColor
            }
        }
    ]
}
