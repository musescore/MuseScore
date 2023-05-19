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
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0
import MuseScore.Cloud 1.0

StyledDialogView {
    id: root

    property string name
    property int visibility: CloudVisibility.Public
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
            spacing: 28

            StyledTextLabel {
                id: titleLabel
                text: qsTrc("project/save", "Share on Audio.com")
                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            ColumnLayout {
                id: optionsColumn
                spacing: 20

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
                        id: nameField

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
                    spacing: 12

                    StyledTextLabel {
                        Layout.fillWidth: true
                        //: visibility of a score on Audio.com: public or unlisted
                        text: qsTrc("project/save", "Visibility")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RadioButtonGroup {
                        id: radioButtonList

                        Layout.fillWidth: true

                        spacing: 28
                        orientation: ListView.Vertical

                        model: [
                            { valueRole: CloudVisibility.Public, text: qsTrc("project/save", "Public"), description: qsTrc("project/save", "Anyone will be able to listen to this audio") },
                            { valueRole: CloudVisibility.Unlisted, text: qsTrc("project/save", "Unlisted"), description: qsTrc("project/save", "Only people with a link can listen to this audio")  },
                        ]

                        delegate: RoundedRadioButton {
                            id: timeFractionButton

                            property bool isCurrent: radioButtonList.currentIndex === model.index

                            ButtonGroup.group: radioButtonList.radioButtonGroup

                            spacing: 18
                            leftPadding: 0
                            implicitHeight: 40

                            contentComponent: Column {
                                property string accessibleName: qsTrc("project/share", "Visibility") + ": " + textLabel.text + ", " +
                                                                qsTrc("project/share", "Description") + descriptionLabel.text

                                spacing: 8

                                StyledTextLabel {
                                    id: textLabel

                                    text: modelData["text"]
                                    font: ui.theme.bodyBoldFont
                                    horizontalAlignment: Qt.AlignLeft
                                }

                                StyledTextLabel {
                                    id: descriptionLabel

                                    text: modelData["description"]
                                    font: ui.theme.bodyFont
                                    horizontalAlignment: Qt.AlignLeft
                                }
                            }

                            checked: (root.visibility === modelData["valueRole"])

                            navigation.name: modelData["valueRole"]
                            navigation.panel: optionsNavPanel
                            navigation.row: nameField.navigation.row + 1 + model.index

                            onToggled: {
                                root.visibility = modelData["valueRole"]
                            }

                            onCheckedChanged: {
                                if (checked && !navigation.active) {
                                    navigation.requestActive()
                                }
                            }
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
                    name: "ShareOnCloudButtons"
                    enabled: buttonsRow.enabled && buttonsRow.visible
                    direction: NavigationPanel.Horizontal
                    section: root.navigationSection
                    order: 2
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
                    id: shareButton
                    text: qsTrc("project/share", "Share")
                    accentButton: enabled
                    enabled: Boolean(root.name)

                    navigation.panel: buttonsNavPanel
                    navigation.column: 1

                    onClicked: {
                        root.done(SaveToCloudResponse.Ok, {
                                      name: root.name,
                                      visibility: root.visibility
                                  })
                    }
                }
            }
        }
    }
}

