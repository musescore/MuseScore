/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

Item {
    id: root

    property alias navigationSection: navPanel.section

    property alias headerText: headerText.text

    property alias originShortcutText: originShortcutField.currentText
    property alias newShortcutText: newShortcutField.currentText
    property alias informationText: informationText.text
    property alias alternativesText: alternativesText.text

    function requestActive() {
        //newShortcutField.navigation.requestActive()
        root.forceActiveFocus()
    }

    signal saveRequested()
    signal cancelRequested()
    signal clearRequested()
    signal newShortcutFieldFocusChanged(bool activeFocus)
    signal currentShortcutAcceptInProgress()
    signal currentShortcutChanged(var shortcutText)

    anchors.fill: parent
    focus: true

    NavigationPanel {
        id: navPanel
        name: "EditShortcutDialog"
        enabled: root.enabled && root.visible
        order: 1
        direction: NavigationPanel.Horizontal
    }

    Column {
        anchors.fill: parent

        spacing: 20

        StyledTextLabel {
            id: headerText

            width: parent.width
            horizontalAlignment: Text.AlignLeft
            font: ui.theme.headerBoldFont
        }

        Column {
            width: parent.width

            spacing: 12

            StyledTextLabel {
                id: informationText

                width: parent.width
                horizontalAlignment: Qt.AlignLeft
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
                    id: originShortcutField
                    Layout.fillWidth: true
                    //enabled: false
                    readOnly: true
                    background.color: ui.theme.backgroundPrimaryColor
                }

                StyledTextLabel {
                    Layout.alignment: Qt.AlignVCenter
                    text: qsTrc("shortcuts", "New shortcut:")
                }

                TextInputField {
                    id: newShortcutField

                    Layout.fillWidth: true

                    background.border.color: ui.theme.accentColor

                    navigation.panel: navPanel
                    navigation.order: 1

                    hint: qsTrc("shortcuts", "Type to set shortcut")
                    //readOnly: true

                    onActiveFocusChanged: {
                        root.newShortcutFieldFocusChanged(activeFocus)
                        if (!activeFocus && navPanel.active) {
                            root.forceActiveFocus()
                        }
                    }

                    onTextEditingFinished: function(text) {
                        root.currentShortcutChanged(text)
                    }

                    onAccepted: root.currentShortcutAcceptInProgress()
                }
            }

            StyledTextLabel {
                id: alternativesText

                width: parent.width
                horizontalAlignment: Qt.AlignLeft
                wrapMode: Text.WordWrap
                maximumLineCount: 2
            }
        }

        ButtonBox {
            width: parent.width

            buttons: [ ButtonBoxModel.Cancel, ButtonBoxModel.Save ]

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            FlatButton {
                text: qsTrc("global", "Clear")
                buttonRole: ButtonBoxModel.CustomRole
                buttonId: ButtonBoxModel.Clear
                isLeftSide: true

                onClicked: {
                    root.clearRequested()
                }
            }

            onStandardButtonClicked: function(buttonId) {
                if (buttonId === ButtonBoxModel.Cancel) {
                    root.cancelRequested()
                } else if (buttonId === ButtonBoxModel.Save) {
                    if (newShortcutField.activeFocus) {
                        root.forceActiveFocus()
                    }
                    root.saveRequested()
                }
            }
        }
    }

    Keys.onShortcutOverride: function(event) {
        if(event.key === Qt.Key_Tab) {
            root.focus = false
        }

        event.accepted = event.key !== Qt.Key_Escape && event.key !== Qt.Key_Tab
    }
}
