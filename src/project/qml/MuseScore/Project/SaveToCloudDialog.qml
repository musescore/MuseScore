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

    property bool canSaveToComputer: true
    property string name
    property int visibility: CloudVisibility.Private

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

        implicitWidth: Math.max(420, contentColumn.implicitWidth)
        implicitHeight: contentColumn.implicitHeight

        AccountAvatar {
            anchors.top: parent.top
            anchors.right: parent.right

            side: 38
            url: accountModel.accountInfo.avatarUrl

            AccountModel {
                id: accountModel

                Component.onCompleted: {
                    load()
                }
            }
        }

        ColumnLayout {
            id: contentColumn
            anchors.fill: parent
            spacing: 20

            StyledTextLabel {
                text: root.visibility === CloudVisibility.Public
                ? qsTrc("project", "Publish to MuseScore.com")
                : qsTrc("project", "Save to cloud")
                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            ColumnLayout {
                spacing: 16

                ColumnLayout {
                    spacing: 8

                    StyledTextLabel {
                        Layout.fillWidth: true
                        text: qsTrc("project", "Name")
                        horizontalAlignment: Text.AlignLeft
                    }

                    TextInputField {
                        Layout.fillWidth: true
                        currentText: root.name

                        onCurrentTextEdited: function(newTextValue) {
                            root.name = newTextValue
                        }
                    }
                }

                ColumnLayout {
                    spacing: 8

                    StyledTextLabel {
                        Layout.fillWidth: true
                        text: qsTrc("project", "Visibility")
                        horizontalAlignment: Text.AlignLeft
                    }

                    Dropdown {
                        Layout.fillWidth: true
                        model: [
                            { value: CloudVisibility.Private, text: qsTrc("project", "Private") },
                            { value: CloudVisibility.Public, text: qsTrc("project", "Public") }
                        ]

                        currentIndex: indexOfValue(root.visibility)
                        onCurrentValueChanged: {
                            root.visibility = currentValue
                        }
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 12

                FlatButton {
                    text: qsTrc("project", "Save to computer")
                    visible: root.canSaveToComputer

                    onClicked: {
                        root.done(SaveToCloudResponse.SaveLocallyInstead)
                    }
                }

                FlatButton {
                    text: qsTrc("global", "Cancel")

                    onClicked: {
                        root.done(SaveToCloudResponse.Cancel)
                    }
                }

                FlatButton {
                    text: qsTrc("project", "Save")
                    accentButton: enabled
                    enabled: Boolean(root.name)

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

