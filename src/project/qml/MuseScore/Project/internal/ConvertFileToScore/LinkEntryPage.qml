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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

Item {
    id: root

    property alias saveAsName: saveAsField.currentText
    property alias saveAsErrorText: saveAsField.errorText

    property string hintText: ""
    property string hintPlainText: ""
    property string audioComUrl: ""
    property int maxLinkLength: 0

    property NavigationSection navigationSection: null

    readonly property int contentPadding: 24

    signal cancelRequested()
    signal backRequested()
    signal convertRequested(string link, string convertedFileName)

    function focusOnDefault() {
        linkInputField.navigation.requestActive()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 18

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: ui.theme.backgroundSecondaryColor
            border.width: 1
            border.color: ui.theme.strokeColor
            radius: 3

            ColumnLayout {
                anchors.centerIn: parent

                width: parent.width - 2 * root.contentPadding
                spacing: 12

                StyledTextLabel {
                    id: hintLabel

                    Layout.fillWidth: true

                    text: root.hintText
                    font: ui.theme.bodyFont
                    horizontalAlignment: Text.AlignHCenter

                    NavigationControl {
                        id: audioComNavCtrl

                        name: "AudioComLink"
                        panel: linkPanel
                        order: 1
                        enabled: Boolean(root.audioComUrl) && root.enabled && root.visible

                        accessible.role: MUAccessible.Button
                        accessible.name: "Audio.com"
                        accessible.visualItem: hintLabel

                        onTriggered: {
                            Qt.openUrlExternally(root.audioComUrl)
                        }
                    }

                    NavigationFocusBorder {
                        navigationCtrl: audioComNavCtrl
                    }
                }

                TextInputField {
                    id: linkInputField

                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 320

                    readonly property string trimmedText: linkInputField.currentText.trim()

                    hint: "https://"
                    maximumLength: root.maxLinkLength > 0 ? root.maxLinkLength : 32767

                    navigation.panel: NavigationPanel {
                        id: linkPanel
                        name: "LinkEntryPage"
                        section: root.navigationSection
                        order: 1
                        enabled: root.enabled && root.visible
                    }
                    navigation.order: 0
                    navigation.accessible.name: qsTrc("project/convert", "Link") + " " + linkInputField.currentText
                    navigation.accessible.description: root.hintPlainText

                    onTextChanged: function(newTextValue) {
                        linkInputField.currentText = newTextValue
                    }
                }

                StyledTextLabel {
                    Layout.fillWidth: true

                    text: qsTrc("project/convert", "Recommended for solo arrangements only")
                    font: ui.theme.bodyFont
                    color: ui.theme.fontSecondaryColor
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        SaveAsField {
            id: saveAsField

            Layout.fillWidth: true

            navigationPanel: NavigationPanel {
                name: "LinkEntryPageSaveAs"
                section: root.navigationSection
                order: 2
                enabled: root.enabled && root.visible
            }
            navigationOrder: 0
        }

        ConvertButtonBox {
            Layout.fillWidth: true

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 3

            convertEnabled: Boolean(linkInputField.trimmedText) && Boolean(saveAsField.currentText) && !saveAsField.errorText

            onCancelRequested: root.cancelRequested()
            onBackRequested: root.backRequested()

            onConvertRequested: {
                root.convertRequested(linkInputField.trimmedText, saveAsField.currentText)
            }
        }
    }
}
