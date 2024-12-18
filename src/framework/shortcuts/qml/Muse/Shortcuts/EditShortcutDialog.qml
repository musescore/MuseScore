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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Shortcuts 1.0

StyledDialogView {
    id: root

    title: qsTrc("shortcuts", "Enter shortcut sequence")

    contentWidth: 538
    contentHeight: 200

    margins: 20

    signal applySequenceRequested(string newSequence, int conflictShortcutIndex)

    function startEdit(shortcut, allShortcuts) {
        model.load(shortcut, allShortcuts)
        open()
    }

    onNavigationActivateRequested: {
        newSequenceField.navigation.requestActive()
    }

    Item {
        id: content
        anchors.fill: parent

        focus: true

        EditShortcutModel {
            id: model

            onApplyNewSequenceRequested: function(newSequence, conflictShortcutIndex) {
                root.applySequenceRequested(newSequence, conflictShortcutIndex)
            }
        }

        NavigationPanel {
            id: navPanel
            name: "EditShortcutSequenceDialog"
            section: root.navigationSection
            enabled: content.enabled && content.visible
            order: 1
            direction: NavigationPanel.Horizontal
        }

        Column {
            anchors.fill: parent

            spacing: 20

            StyledTextLabel {
                width: parent.width
                text: qsTrc("shortcuts", "Define keyboard shortcut")
                horizontalAlignment: Text.AlignLeft
                font: ui.theme.headerBoldFont
            }

            Column {
                width: parent.width

                spacing: 12

                StyledTextLabel {
                    width: parent.width
                    horizontalAlignment: Qt.AlignLeft

                    text: model.conflictWarning
                }

                GridLayout {
                    width: parent.width
                    height: childrenRect.height

                    columnSpacing: 12
                    rowSpacing: 12

                    columns: 2
                    rows: 2

                    StyledTextLabel {
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTrc("shortcuts", "Old shortcut:")
                    }

                    TextInputField {
                        Layout.fillWidth: true

                        enabled: false
                        currentText: model.originSequence
                    }

                    StyledTextLabel {
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTrc("shortcuts", "New shortcut:")
                    }

                    TextInputField {
                        id: newSequenceField

                        Layout.fillWidth: true

                        background.border.color: ui.theme.accentColor

                        navigation.panel: navPanel
                        navigation.order: 1

                        hint: qsTrc("shortcuts", "Type to set shortcut")
                        readOnly: true
                        currentText: model.newSequence

                        onActiveFocusChanged: {
                            if (activeFocus) {
                                content.forceActiveFocus()
                            }
                        }
                    }
                }
            }

            ButtonBox {
                width: parent.width

                buttons: [ ButtonBoxModel.Cancel, ButtonBoxModel.Save ]

                navigationPanel.section: root.navigationSection
                navigationPanel.order: 2

                onStandardButtonClicked: function(buttonId) {
                    if (buttonId === ButtonBoxModel.Cancel) {
                        root.reject()
                    } else if (buttonId === ButtonBoxModel.Save) {
                        model.applyNewSequence()
                        root.accept()
                    }
                }
            }
        }

        Keys.onShortcutOverride: function(event) {
            if(event.key === Qt.Key_Tab) {
                content.focus = false
            }
            event.accepted = event.key !== Qt.Key_Escape && event.key !== Qt.Key_Tab 
        }

        Keys.onPressed: function(event) {
            model.inputKey(event.key, event.modifiers)
        }
    }
}

