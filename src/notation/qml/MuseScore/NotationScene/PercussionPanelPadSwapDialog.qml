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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

StyledDialogView {
    id: root

    property bool moveMidiNotesAndShortcuts: true

    contentWidth: 500
    contentHeight: contentRow.implicitHeight + contentColumn.spacing + buttonBox.implicitHeight

    margins: 16

    //! NOTE: Actually making this empty will display "MuseScore Studio"
    title: " "

    function done(data = {}) {
        let value = Object.assign(data)

        root.ret = {
            errcode: 0,
            value: value
        }

        root.hide()
    }

    onNavigationActivateRequested: {
        var btn = buttonBox.firstFocusBtn
        if (btn) {
            btn.navigation.requestActive()
        }
    }

    onAccessibilityActivateRequested: {
        accessibleInfo.readInfo()
    }

    AccessibleItem {
        id: accessibleInfo

        accessibleParent: radioButtons.accessible
        visualItem: titleLabel
        role: MUAccessible.StaticText
        name: titleLabel.text

        function readInfo() {
            accessibleInfo.ignored = false
            accessibleInfo.focused = true
        }

        function resetFocus() {
            accessibleInfo.ignored = true
            accessibleInfo.focused = false
        }
    }

    NavigationPanel {
        id: contentNavigationPanel

        name: "contentNavigationPanel"
        section: root.navigationSection
        order: 0
    }

    Row {
        id: contentRow

        width: parent.width
        spacing: 28

        StyledIconLabel {
            id: icon

            font.pixelSize: 48
            iconCode: IconCode.QUESTION
        }

        Column {
            id: contentColumn

            width: contentRow.width - contentRow.spacing - icon.width
            spacing: 16

            StyledTextLabel {
                id: titleLabel

                width: parent.width

                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap

                text: qsTrc("notation/percussion", "Do you also want to move the MIDI notes and keyboard shortcuts that trigger these sounds?")
            }

            RadioButtonGroup {
                id: radioButtons

                width: parent.width
                spacing: contentColumn.spacing

                orientation: ListView.Vertical

                model: [
                    { text: qsTrc("notation/percussion", "Move MIDI notes and keyboard shortcuts with their sounds"), value: true },
                    { text: qsTrc("notation/percussion", "Leave MIDI notes and keyboard shortcuts fixed to original pad positions"), value: false }
                ]

                delegate: Row {
                    width: parent.width
                    spacing: 6

                    RoundedRadioButton {
                        id: radioButton

                        anchors.verticalCenter: parent.verticalCenter

                        navigation.name: modelData.text
                        navigation.panel: contentNavigationPanel
                        navigation.row: model.index

                        checked: modelData.value === root.moveMidiNotesAndShortcuts

                        onToggled: {
                            // TODO: Live update the pads when this value is changed...
                            root.moveMidiNotesAndShortcuts = modelData.value
                        }
                    }

                    //! NOTE: Can't use radioButton.text because it won't wrap
                    StyledTextLabel {
                        width: parent.width - parent.spacing - radioButton.width

                        anchors.verticalCenter: parent.verticalCenter

                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.Wrap
                        text: modelData.text

                        MouseArea {
                            id: mouseArea

                            anchors.fill: parent

                            onClicked: {
                                // TODO: Live update the pads when this value is changed...
                                root.moveMidiNotesAndShortcuts = modelData.value
                            }
                        }
                    }
                }
            }

            CheckBox {
                id: rememberMyChoice

                width: parent.width

                navigation.panel: contentNavigationPanel
                navigation.row: radioButtons.model.length
                navigation.accessible.name: rememberMyChoice.text

                text: qsTrc("global", "Remember my choice")
                checked: false
                onClicked: {
                    rememberMyChoice.checked = !rememberMyChoice.checked
                }
            }


            StyledTextLabel {
                id: preferenceInfo

                width: parent.width

                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap

                text: qsTrc("global", "This setting can be changed at any time in Preferences")
            }
        }
    }

    ButtonBox {
        id: buttonBox

        anchors {
            top: contentRow.bottom
            topMargin: contentColumn.spacing
            right: parent.right
        }

        navigationPanel.section: root.navigationSection
        navigationPanel.order: contentNavigationPanel.order + 1

        buttons: [ ButtonBoxModel.Cancel, ButtonBoxModel.Done ]

        isAccessibilityDisabledWhenInit: true

        onStandardButtonClicked: function(buttonId) {
            if (buttonId === ButtonBoxModel.Cancel) {
                root.reject()
                return
            }

            root.done({ moveMidiNotesAndShortcuts: root.moveMidiNotesAndShortcuts, rememberMyChoice: rememberMyChoice.checked })
        }
    }
}
