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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

RowLayout {
    id: root

    property string type: "INFO" // "QUESTION", "INFO", "WARNING", "ERROR"

    property alias title: titleLabel.text
    property alias text: textLabel.text
    property alias textFormat: textLabel.textFormat

    property bool withIcon: false
    property alias iconCode: icon.iconCode

    property alias withDontShowAgainCheckBox: dontShowAgainCheckBox.visible

    property int buttons: 0
    property var customButtons
    property int defaultButtonId: 0

    property alias navigation: navPanel

    signal clicked(int buttonId, bool showAgain)

    function focusOnFirst() {
        var btn = buttons.accentButton()
        btn.navigation.requestActive()
    }

    function readInfo() {
        accessibleInfo.readInfo()
    }

    function standardIcon(type) {
        switch (type) {
        case "QUESTION": return IconCode.QUESTION
        case "INFO": return IconCode.INFO
        case "WARNING": return IconCode.WARNING
        case "ERROR": return IconCode.ERROR
        }

        return IconCode.NONE
    }

    function standardName(type) {
        switch (type) {
        case "QUESTION": return qsTrc("global", "Question")
        case "INFO": return qsTrc("global", "Information")
        case "WARNING": return qsTrc("global", "Warning")
        case "ERROR": return qsTrc("global", "Error")
        }

        return qsTrc("global", "Information")
    }

    NavigationPanel {
        id: navPanel
        name: "StandardDialog"
        order: 1
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Horizontal
        accessible.role: MUAccessible.Dialog
        accessible.name: root.standardName(root.type)
    }

    //! NOTE By default accessibility for buttons ignored.
    // On dialog open, set focus on accessibleInfo item, so Screen Reader reads completed info for dialog.
    // On button navigation active turned on accessibility for button.
    AccessibleItem {
        id: accessibleInfo
        accessibleParent: navPanel.accessible
        visualItem: root
        role: MUAccessible.Button
        name: root.title + " " + root.text + " " + root.firstFocusBtn().text

        function readInfo() {
            accessibleInfo.ignored = false
            accessibleInfo.focused = true
        }

        function resetFocus() {
            accessibleInfo.ignored = true
            accessibleInfo.focused = false
        }
    }

    spacing: 27

    StyledIconLabel {
        id: icon

        Layout.alignment: Qt.AlignTop
        Layout.preferredWidth: 48
        Layout.preferredHeight: 48

        font.pixelSize: 48
        iconCode: root.standardIcon(root.type)

        visible: root.withIcon && !isEmpty
    }

    Column {
        Layout.fillWidth: true

        // Unclear why "+ 1" is needed; apparently implicitWidth is incorrect
        readonly property real textsImplicitWidth: Math.max(titleLabel.implicitWidth,
                                                            textLabel.implicitWidth,
                                                            dontShowAgainCheckBox.implicitWidth) + 1

        // Allow the text to make the dialog wider, but not too much wider
        readonly property real textsImplicitWidthBounded: Math.min(420, textsImplicitWidth)

        // But if the buttons need more space, then the dialog becomes as wide as necessary
        Layout.preferredWidth: Math.max(buttons.width, textsImplicitWidthBounded)

        spacing: 18

        Column {
            width: parent.width
            height: Math.max(implicitHeight, 32)
            spacing: 18

            StyledTextLabel {
                id: titleLabel
                width: parent.width

                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap

                visible: !isEmpty
            }

            StyledTextLabel {
                id: textLabel
                width: parent.width

                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap

                visible: !isEmpty
            }

            CheckBox {
                id: dontShowAgainCheckBox
                width: parent.width
                visible: false

                text: qsTrc("global", "Don't show this message again")
                checked: false

                navigation.panel: navPanel
                navigation.column: buttons.count + 1

                onClicked: {
                    checked = !checked
                }
            }
        }

        ButtonBox {
            id: buttons

            anchors.right: parent.right

            buttons: root.buttons
            customButtons: {
                if (!root.customButtons) {
                    return
                }

                var result = []
                for (var i = 0; i < root.customButtons.length; i++) {
                    var customButton = root.customButtons[i]

                    const button = Qt.createQmlObject('
                                    import MuseScore.UiComponents 1.0

                                    ButtonBoxItem {
                                    }', buttons)
                    button.text = customButton.text
                    button.buttonRole = customButton.role
                    button.isAccent = customButton.isAccent
                    button.isLeftSide = customButton.isLeftSide

                    const buttonId = customButton.buttonId;
                    button.clicked.connect(function() {
                        root.clicked(buttonId, !dontShowAgainCheckBox.checked)
                    })

                    result.push(button)
                }
                return result
            }

            separationGap: false

            delegateProperties: { //! NOTE See description about AccessibleItem { id: accessibleInfo }
                "accessible.ignored" : true,
                "navigation.onActiveChanged": function(item) {
                    if (!item.navigation.active) {
                        item.accessible.ignored = false
                        accessibleInfo.ignored = true
                    }
                }
            }
            navigationPanel: navPanel

            onStandardButtonClicked: function(type) {
                root.clicked(type, !dontShowAgainCheckBox.checked)
            }
        }
    }
}
