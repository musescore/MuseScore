/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

Item {
    id: root

    property NavigationPanel navigationPanel: null

    readonly property string accessibleName: qsTrc("project/convert", "Help us improve score processing with your thoughts on what went wrong")
    readonly property string accessibleDescription: ""

    readonly property int maxFeedbackLength: 3000

    signal submitRequested(string comment)
    signal closeRequested()

    implicitWidth: 480
    implicitHeight: Math.max(content.implicitHeight + content.anchors.margins * 2, 214)

    RowLayout {
        id: content

        anchors.fill: parent
        anchors.margins: 16

        spacing: 12

        StyledIconLabel {
            Layout.alignment: Qt.AlignTop

            iconCode: IconCode.INFO_FILLED
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 12

            StyledTextLabel {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap

                font.bold: true

                text: qsTrc("project/convert", "Help us improve score processing with your thoughts on what went wrong")
            }

            TextInputArea {
                id: feedbackInput

                Layout.fillWidth: true
                Layout.preferredHeight: 100

                hint: qsTrc("project/convert", "Tell us how your score could look better")

                verticalScrollBarPolicy: ScrollBar.AlwaysOn

                navigation.panel: root.navigationPanel
                navigation.order: 1
                navigation.accessible.name: qsTrc("project/convert", "Feedback")

                onTextChanged: function(newTextValue) {
                    if (newTextValue.length > root.maxFeedbackLength) {
                        var cursorPosition = inputField.cursorPosition
                        inputField.text = newTextValue.slice(0, root.maxFeedbackLength)
                        inputField.cursorPosition = Math.min(cursorPosition, inputField.text.length)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                StyledTextLabel {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    horizontalAlignment: Text.AlignLeft

                    opacity: 0.7

                    readonly property int remainingChars: root.maxFeedbackLength - feedbackInput.inputField.text.length

                    //: %1 is the current number of characters typed, %2 is the maximum allowed
                    text: remainingChars <= 100
                          ? qsTrc("project/convert", "%1/%2 characters").arg(feedbackInput.inputField.text.length).arg(root.maxFeedbackLength)
                          : ""
                }

                FlatButton {
                    text: qsTrc("project/convert", "Submit feedback")
                    accentButton: true
                    enabled: feedbackInput.hasText

                    navigation.panel: root.navigationPanel
                    navigation.order: 2

                    onClicked: root.submitRequested(feedbackInput.inputField.text.trim())
                }
            }
        }

        FlatButton {
            Layout.alignment: Qt.AlignTop

            Layout.preferredWidth: 16
            Layout.preferredHeight: 16

            icon: IconCode.CLOSE_X_ROUNDED
            transparent: true

            navigation.panel: root.navigationPanel
            navigation.order: 3
            navigation.accessible.name: qsTrc("global", "Close")

            onClicked: root.closeRequested()
        }
    }
}
