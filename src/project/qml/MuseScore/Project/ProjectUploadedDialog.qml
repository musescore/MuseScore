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

StyledDialogView {
    id: root

    contentHeight: 500
    contentWidth: 750

    objectName: "ProjectUploadedDialog"

    property string scoreManagerUrl: ""

    Item {
        anchors.fill: parent

        NavigationPanel {
            id: buttonsNavPanel
            name: "ProjectUploadedDialogButtons"
            direction: NavigationPanel.Horizontal
            section: root.navigationSection
            order: 1
        }

        Image {
            id: image

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left

            width: 300

            source: "qrc:/resources/PublishScores.png"
        }

        ColumnLayout {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: image.right
            anchors.right: parent.right
            anchors.margins: 24

            spacing: 0

            StyledTextLabel {
                text: qsTrc("global", "Success!")
                font: ui.theme.tabBoldFont
            }

            StyledTextLabel {
                Layout.topMargin: 6

                text: qsTrc("project", "All saved changes will now update to the cloud")
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 228
                Layout.topMargin: 24

                Rectangle {
                    anchors.fill: parent

                    color: ui.theme.buttonColor
                    opacity: 0.4
                    radius: 3
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 28

                    z: 1000

                    StyledTextLabel {
                        text: qsTrc("project", "Publish your finished scores on MuseScore.com")
                        font: ui.theme.largeBodyBoldFont
                    }

                    Column {
                        Layout.fillWidth: true

                        spacing: 14

                        Repeater {
                            model: [
                                qsTrc("project", "Create a portfolio to showcase your music"),
                                qsTrc("project", "Gain followers and receive score comments and ratings"),
                                qsTrc("project", "Share your projects and collaborate with other musicians")
                            ]

                            Row {
                                spacing: 10

                                Rectangle {
                                    anchors.verticalCenter: parent.verticalCenter

                                    width: 9
                                    height: width
                                    radius: width / 2

                                    color: ui.theme.accentColor
                                }

                                StyledTextLabel {
                                    text: modelData
                                }
                            }
                        }
                    }

                    FlatButton {
                        Layout.alignment: Qt.AlignLeft

                        accentButton: true
                        text: qsTrc("project", "Watch video")

                        navigation.panel: buttonsNavPanel
                        navigation.column: 1

                        onClicked: {
                            root.hide()
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }

            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 10

                FlatButton {
                    text: qsTrc("project", "View score online")

                    navigation.panel: buttonsNavPanel
                    navigation.column: 2

                    onClicked: {
                        Qt.callLater(function() {
                            api.launcher.openUrl(root.scoreManagerUrl)
                        })

                        root.hide()
                    }
                }

                FlatButton {
                    text: qsTrc("global", "Close")

                    navigation.panel: buttonsNavPanel
                    navigation.column: 3

                    onClicked: {
                        root.hide()
                    }
                }
            }
        }
    }
}
