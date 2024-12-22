/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

FocusScope {
    id: root

    property var currentText: ""

    property alias hint: valueInput.placeholderText

    property bool hasText: valueInput.text.length > 0

    property bool resizeVerticallyWithText: false

    property real textSidePadding: 12

    readonly property alias inputField: valueInput
    readonly property alias background: background

    readonly property alias mouseArea: clickableArea
    property bool containsMouse: clickableArea.containsMouse

    readonly property alias navigation: navCtrl
    readonly property alias accessible: navCtrl.accessible

    signal textChanged(var newTextValue)
    signal textEditingFinished(var newTextValue)
    signal accepted()
    signal escaped()

    function selectAll() {
        valueInput.selectAll()
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

    implicitHeight: root.resizeVerticallyWithText ? Math.max(valueInput.height, 90) : 90
    implicitWidth: parent.width

    height: implicitHeight

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    FocusListener {
        item: root
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName !== "" ? root.objectName : "TextInputArea"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.EditableText
        accessible.name: valueInput.text
        accessible.visualItem: root
        accessible.text: valueInput.text
        accessible.selectedText: valueInput.selectedText
        accessible.selectionStart: valueInput.selectionStart
        accessible.selectionEnd: valueInput.selectionEnd
        accessible.cursorPosition: valueInput.cursorPosition

        onActiveChanged: {
            if (navCtrl.active) {
                root.ensureActiveFocus()
            }
        }
    }

    Rectangle {
        id: background
        anchors.fill: parent

        NavigationFocusBorder { navigationCtrl: navCtrl }

        color: ui.theme.textFieldColor
        border.color: ui.theme.strokeColor
        border.width: Math.max(ui.theme.borderWidth, 1)
        radius: 3
    }

    ScrollView {
        anchors.fill: parent

        ScrollBar.vertical: StyledScrollBar {
            anchors.margins: 8
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }

        ScrollBar.horizontal: StyledScrollBar {
            anchors.margins: 8
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
        }

        contentWidth: valueInput.width
        contentHeight: root.resizeVerticallyWithText ? -1 : valueInput.height

        // When a TextArea component is set as a direct child of a ScrollView, the result will always be scrollable in both directions. To override this,
        // we need to place the TextArea inside an Item and manually define contentWidth/Height
        Item {
            TextArea {
                id: valueInput

                objectName: "TextArea"

                padding: root.textSidePadding

                color: ui.theme.fontPrimaryColor
                font: ui.theme.bodyFont

                background: Item {}

                focus: false
                activeFocusOnPress: false
                selectByMouse: true
                selectionColor: Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.accentOpacityNormal)
                selectedTextColor: ui.theme.fontPrimaryColor
                placeholderTextColor: Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.3)
                visible: true

                text: root.currentText === undefined ? "" : root.currentText

                TextInputModel {
                    id: textInputModel
                }

                Component.onCompleted: {
                    textInputModel.init()
                }

                Keys.onShortcutOverride: function(event) {
                    if (readOnly) {
                        return
                    }

                    if (event.key === Qt.Key_Escape) {
                        event.accepted = true
                        return
                    }

                    if (textInputModel.isShortcutAllowedOverride(event.key, event.modifiers)) {
                        event.accepted = true
                    } else {
                        event.accepted = false

                        root.focus = false
                        root.textEditingFinished(valueInput.text)
                    }
                }

                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_Escape) {
                        root.focus = false
                        root.textEditingFinished(valueInput.text)
                        root.escaped()
                    }
                }

                onActiveFocusChanged: {
                    if (activeFocus) {
                        navCtrl.requestActive()
                        selectAll()
                    } else {
                        deselect()
                        root.textEditingFinished(valueInput.text)
                    }
                }

                onTextChanged: {
                    root.textChanged(text)
                }
            }
        }
    }

    states: [
        State {
            name: "HOVERED"
            when: root.containsMouse && !valueInput.activeFocus
            PropertyChanges { target: background; border.color: Utils.colorWithAlpha(ui.theme.accentColor, 0.6) }
        },

        State {
            name: "FOCUSED"
            when: valueInput.activeFocus
            PropertyChanges { target: background; border.color: ui.theme.accentColor }
        }
    ]

    MouseArea {
        id: clickableArea
        anchors.fill: parent

        enabled: root.enabled
        propagateComposedEvents: true
        hoverEnabled: true
        cursorShape: Qt.IBeamCursor

        onPressed: function(mouse) {
            root.ensureActiveFocus()
            navCtrl.requestActiveByInteraction()
            mouse.accepted = false
        }
    }
}
