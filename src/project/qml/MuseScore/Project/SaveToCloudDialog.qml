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
import MuseScore.Project 1.0
import MuseScore.Cloud 1.0

StyledDialogView {
    id: root

    property bool isPublish: false
    property string name
    property int visibility: CloudVisibility.Private
    property string existingOnlineScoreUrl
    property bool replaceExistingOnlineScore: true
    property string cloudCode: ""

    contentWidth: contentItem.implicitWidth
    contentHeight: contentItem.implicitHeight

    margins: 20

    function done(response, data = {}) {
        let value = Object.assign(({ response: response }), data)

        root.ret = {
            errcode: 0,
            value: value
        }

        root.hide()
    }

    Item {
        id: contentItem

        property var cloudInfo: null

        implicitWidth: Math.max(420, contentColumn.implicitWidth)
        implicitHeight: contentColumn.implicitHeight

        AccountAvatar {
            id: avatar

            anchors.top: parent.top
            anchors.right: parent.right

            side: 38
            url: Boolean(contentItem.cloudInfo) ? contentItem.cloudInfo.userAvatarUrl : null

            CloudsModel {
                id: cloudsModel

                Component.onCompleted: {
                    load()

                    contentItem.cloudInfo = cloudsModel.cloudInfo(root.cloudCode)
                }
            }
        }

        ColumnLayout {
            id: contentColumn
            anchors.fill: parent
            spacing: 20

            StyledTextLabel {
                id: titleLabel
                text: root.isPublish
                      ? qsTrc("project/save", "Publish to %1").arg(Boolean(contentItem.cloudInfo) ? contentItem.cloudInfo.cloudTitle : "")
                      : qsTrc("project/save", "Save to cloud")
                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            ColumnLayout {
                id: optionsColumn
                spacing: 16

                NavigationPanel {
                    id: optionsNavPanel
                    name: "SaveToCloudOptions"
                    enabled: optionsColumn.enabled && optionsColumn.visible
                    direction: NavigationPanel.Vertical
                    section: root.navigationSection
                    order: 1
                    accessible.name: qsTrc("project/save", "Options")
                }

                ColumnLayout {
                    spacing: 8

                    StyledTextLabel {
                        Layout.fillWidth: true
                        text: qsTrc("project/save", "Name")
                        horizontalAlignment: Text.AlignLeft
                    }

                    TextInputField {
                        Layout.fillWidth: true
                        currentText: root.name

                        navigation.panel: optionsNavPanel
                        navigation.row: 1
                        accessible.name: titleLabel.text + ". " + qsTrc("project/save", "Name") + ": " + currentText

                        onTextChanged: function(newTextValue) {
                            root.name = newTextValue
                        }
                    }
                }

                ColumnLayout {
                    spacing: 8

                    StyledTextLabel {
                        Layout.fillWidth: true
                        //: visibility of a score on MuseScore.com: private, public or unlisted
                        text: qsTrc("project/save", "Visibility")
                        horizontalAlignment: Text.AlignLeft
                    }

                    StyledDropdown {
                        Layout.fillWidth: true

                        model: [
                            { value: CloudVisibility.Public, text: qsTrc("project/save", "Public") },
                            { value: CloudVisibility.Unlisted, text: qsTrc("project/save", "Unlisted") },
                            { value: CloudVisibility.Private, text: qsTrc("project/save", "Private") }
                        ]

                        currentIndex: indexOfValue(root.visibility)

                        navigation.panel: optionsNavPanel
                        navigation.row: 2
                        navigation.accessible.name: qsTrc("project/save", "Visibility") + ": " + currentText

                        onActivated: function(index, value) {
                            root.visibility = value
                        }
                    }
                }

                RadioButtonGroup {
                    Layout.fillWidth: true

                    orientation: ListView.Vertical
                    spacing: 8

                    visible: root.isPublish && Boolean(root.existingOnlineScoreUrl)

                    model: [
                        //: The text between `<a href=\"%1\">` and `</a>` will be a clickable link to the online score in question
                        { text: qsTrc("project/save", "Replace the existing <a href=\"%1\">online score</a>").arg(root.existingOnlineScoreUrl), value: true },
                        { text: qsTrc("project/save", "Publish as new online score"), value: false }
                    ]

                    delegate: RoundedRadioButton {
                        checked: modelData.value === root.replaceExistingOnlineScore
                        text: modelData.text

                        navigation.name: modelData.text
                        navigation.panel: optionsNavPanel
                        navigation.row: 3 + model.index

                        onToggled: {
                            root.replaceExistingOnlineScore = modelData.value
                        }
                    }
                }
            }

            RowLayout {
                id: buttonsRow
                Layout.alignment: Qt.AlignRight
                spacing: 12

                NavigationPanel {
                    id: buttonsNavPanel
                    name: "SaveToCloudButtons"
                    enabled: buttonsRow.enabled && buttonsRow.visible
                    direction: NavigationPanel.Horizontal
                    section: root.navigationSection
                    order: 2
                }

                FlatButton {
                    text: qsTrc("project/save", "Save to computer")
                    visible: !root.isPublish

                    navigation.panel: buttonsNavPanel
                    navigation.column: 2

                    onClicked: {
                        root.done(SaveToCloudResponse.SaveLocallyInstead)
                    }
                }

                FlatButton {
                    text: qsTrc("global", "Cancel")

                    navigation.panel: buttonsNavPanel
                    navigation.column: 3

                    onClicked: {
                        root.done(SaveToCloudResponse.Cancel)
                    }
                }

                FlatButton {
                    id: saveButton
                    text: root.isPublish ? qsTrc("project/save", "Publish") : qsTrc("project/save", "Save")
                    accentButton: enabled
                    enabled: Boolean(root.name)

                    navigation.panel: buttonsNavPanel
                    navigation.column: 1

                    onClicked: {
                        root.done(SaveToCloudResponse.Ok, {
                                      name: root.name,
                                      visibility: root.visibility,
                                      replaceExistingOnlineScore: root.replaceExistingOnlineScore
                                  })
                    }
                }
            }
        }
    }
}

