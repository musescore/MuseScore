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
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

FocusScope {
    id: root

    property bool isIndeterminate: false
    readonly property string indeterminateText: "--"
    property var currentText: ""
    property alias validator: valueInput.validator
    property alias measureUnitsSymbol: measureUnitsLabel.text

    property alias hint: valueInput.placeholderText
    property alias hintIcon: hintIcon.iconCode
    property bool clearTextButtonVisible: false

    property bool hasText: valueInput.text.length > 0
    property alias readOnly: valueInput.readOnly

    property alias navigation: navCtrl
    property alias accessible: navCtrl.accessible

    property alias clearTextButton: clearTextButtonItem

    signal currentTextEdited(var newTextValue)
    signal textCleared()
    signal textEditingFinished()

    function selectAll() {
        valueInput.selectAll()
    }

    function clear() {
        valueInput.text = ""
        currentText = ""
        textCleared()
    }

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            valueInput.forceActiveFocus()
        }
    }

    implicitHeight: 30
    implicitWidth: parent.width

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    FocusListener {
        item: root
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName !== "" ? root.objectName : "TextInputField"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.EditableText
        accessible.name: Boolean(valueInput.text) ? valueInput.text + " " + measureUnitsLabel.text : valueInput.placeholderText
        accessible.visualItem: root

        onActiveChanged: {
            if (navCtrl.active) {
                root.ensureActiveFocus()
            }
        }
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: ui.theme.textFieldColor
        radius: 4

        NavigationFocusBorder { navigationCtrl: navCtrl }

        border.color: ui.theme.strokeColor
        border.width: ui.theme.borderWidth > 0 ? ui.theme.borderWidth : 1 //borderWidth of >0 suggests that an HC theme is active, in which case we don't want to make a change
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: hintIcon.visible ? 0 : 12
        anchors.rightMargin: 4

        spacing: 0

        StyledIconLabel {
            id: hintIcon

            Layout.fillHeight: true
            Layout.preferredWidth: 30

            visible: Boolean(!hintIcon.isEmpty)
        }

        TextField {
            id: valueInput

            objectName: "TextField"

            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: !measureUnitsLabel.visible

            //! NOTE Disabled default Qt Accessible
            Accessible.role: Accessible.NoRole

            color: ui.theme.fontPrimaryColor
            font: ui.theme.bodyFont

            background: Item {}

            focus: false
            activeFocusOnPress: false
            selectByMouse: true
            selectionColor: Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.accentOpacityNormal)
            selectedTextColor: ui.theme.fontPrimaryColor
            visible: !root.isIndeterminate || activeFocus

            text: root.currentText === undefined ? "" : root.currentText

            Keys.onShortcutOverride: {
                event.accepted = true
            }

            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return
                        || event.key === Qt.Key_Escape) {
                    root.focus = false
                    root.textEditingFinished()
                }
            }

            onActiveFocusChanged: {
                if (activeFocus) {
                    selectAll()
                } else {
                    deselect()
                    root.textEditingFinished()
                }
            }

            onTextChanged: {
                if (!acceptableInput) {
                    return
                }

                root.currentTextEdited(text)
            }
        }

        StyledTextLabel {
            id: measureUnitsLabel

            Layout.alignment: Qt.AlignVCenter

            color: ui.theme.fontPrimaryColor
            visible: !root.isIndeterminate && Boolean(text)
        }

        FlatButton {
            id: clearTextButtonItem

            Layout.fillHeight: true
            Layout.preferredWidth: height

            readonly property int margin: 4

            Layout.topMargin: margin
            Layout.bottomMargin: margin

            icon: IconCode.CLOSE_X_ROUNDED
            visible: root.clearTextButtonVisible

            normalStateColor: background.color
            hoveredStateColor: ui.theme.accentColor
            pressedStateColor: ui.theme.accentColor

            navigation.panel: navCtrl.panel
            navigation.order: navCtrl.order + 1

            onClicked: {
                root.clear()
            }
        }

        Item {
            Layout.fillWidth: measureUnitsLabel.visible
        }
    }

    StyledTextLabel {
        id: undefinedValueLabel

        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 12

        text: root.indeterminateText
        color: ui.theme.fontPrimaryColor
        visible: root.isIndeterminate && valueInput.activeFocus === false
    }

    states: [
        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !valueInput.activeFocus
            PropertyChanges { target: background; opacity: 0.6 }
        },

        State {
            name: "FOCUSED"
            when: valueInput.activeFocus
            PropertyChanges { target: background; border.color: ui.theme.accentColor; border.width: 1; opacity: 1 }
        }
    ]

    MouseArea {
        id: clickableArea

        anchors.top: parent.top
        anchors.left: parent.left

        height: parent.height
        width: clearTextButtonItem.visible ? parent.width - clearTextButtonItem.width : parent.width

        propagateComposedEvents: true
        hoverEnabled: true

        onPressed: {
            root.ensureActiveFocus()
            mouse.accepted = false
        }
    }
}
