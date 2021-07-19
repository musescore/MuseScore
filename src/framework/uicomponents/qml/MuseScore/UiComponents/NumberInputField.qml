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
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0

Item {
    id: root

    property int minValue: 0
    property int maxValue: 999
    property int value: 0

    property bool addLeadingZeros: true
    property int displayedNumberLength: maxValue.toString().length

    property alias font: textField.font

    property alias navigation: navCtrl
    property alias accessible: navCtrl.accessible

    signal valueEdited(var newValue)

    implicitWidth: textField.contentWidth
    implicitHeight: 30

    opacity: enabled ? 1 : ui.theme.itemOpacityDisabled

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            textField.forceActiveFocus()
        }
    }

    QtObject {
        id: privateProperties

        function pad(value) {
            var str = value.toString()

            if (!root.addLeadingZeros) {
                return str;
            }

            while (str.length < root.displayedNumberLength) {
                str = "0" + str
            }

            return str
        }
    }

    onValueChanged: {
        textField.text = privateProperties.pad(value)
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName !== "" ? root.objectName : "NumberInputField"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.EditableText
        accessible.name: textField.text
        accessible.visualItem: root

        onActiveChanged: {
            if (navCtrl.active) {
                root.ensureActiveFocus()
            }
        }
    }

    TextField {
        id: textField

        anchors.centerIn: parent
        padding: 0

        readOnly: root.maxValue === 0
        text: privateProperties.pad(root.value)

        onTextEdited: {
            var currentValue = text.length > 0 ? parseInt(text) : 0
            var str = currentValue.toString()
            var newValue = 0

            if (str.length > privateProperties.displayedNumberLength || currentValue > root.maxValue) {
                var lastDigit = str.charAt(str.length - 1)
                newValue = parseInt(lastDigit)
            } else {
                newValue = currentValue
            }

            newValue = Math.min(newValue, root.maxValue)
            text = privateProperties.pad(newValue)

            root.valueEdited(newValue)
        }

        background: Rectangle {
            id: textFieldBackground

            color: "transparent"
        }

        selectByMouse: false

        color: ui.theme.fontPrimaryColor
        font: ui.theme.tabFont

        validator: IntValidator {
            bottom: root.minValue
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        enabled: !textField.readOnly

        onClicked: {
            textField.forceActiveFocus()
            textField.cursorPosition = textField.text.length
        }
    }

    states: [
        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed && !textField.activeFocus

            PropertyChanges {
                target: textFieldBackground
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: mouseArea.pressed && !textField.activeFocus

            PropertyChanges {
                target: textFieldBackground
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "FOCUSED"
            when: textField.activeFocus && !textField.readOnly

            PropertyChanges {
                target: textFieldBackground
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityNormal
            }
        }
    ]
}
