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

FocusScope {
    id: root

    property alias icon: buttonIcon.iconCode
    property string text: ""
    property string progressTitle: ""

    property real from: 0.0
    property real to: 1.0
    property real value: 0.0

    property bool indeterminate: false

    property alias navigation: navCtrl

    signal clicked()

    function setProgress(status, indeterminate, current, total) {
        root.progressTitle = status
        root.indeterminate = indeterminate
        root.value = 0.0
        if (!indeterminate) {
            root.value = current
            root.to = total
        }
    }

    function resetProgress() {
        value = 0.0
        indeterminate = false
    }

    height: contentWrapper.implicitHeight + 16
    width: 132

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "ProgressButton"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.Button
        accessible.name: root.text
        accessible.visualItem: root

        onActiveChanged: {
            if (active) {
                root.ensureActiveFocus()
            }
        }
        onTriggered: root.clicked()
    }

    QtObject {
        id: prv

        property bool inProgress: (from < value && value < to) || indeterminate
    }

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: prv.inProgress ? ui.theme.backgroundPrimaryColor : ui.theme.accentColor
        opacity: ui.theme.buttonOpacityNormal

        border.color: ui.theme.focusColor
        border.width: navCtrl.active ? 2 : 0

        radius: 3
    }

    Rectangle {
        id: progressRect

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        width: prv.inProgress ? parent.width * (value / to) : 0

        color: ui.theme.accentColor
    }

    Column {
        id: contentWrapper

        anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

        height: implicitHeight

        spacing: 4

        StyledIconLabel {
            id: buttonIcon

            anchors.horizontalCenter: parent.horizontalCenter
        }

        StyledTextLabel {
            id: textLabel

            anchors.horizontalCenter: parent.horizontalCenter
            height: text === "" ? 0 : implicitHeight

            horizontalAlignment: Text.AlignHCenter

            text: prv.inProgress ? progressTitle : root.text
        }
    }

    MouseArea {
        id: clickableArea

        anchors.fill: parent

        hoverEnabled: true

        enabled: !prv.inProgress

        onReleased: {
            root.ensureActiveFocus()
            root.clicked()
        }
    }

    states: [
        State {
            name: "PRESSED"
            when: clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                opacity: ui.theme.buttonOpacityHit
                border.color: ui.theme.strokeColor
            }
        },

        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                opacity: ui.theme.buttonOpacityHover
                border.color: ui.theme.strokeColor
            }
        }
    ]
}
