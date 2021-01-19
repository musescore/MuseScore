//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019-2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

import "../palettes"

FocusScope {
    id: root

    property var model

    Keys.onEscapePressed: {
        root.model.ignore()
    }

    Rectangle {
        anchors.fill: parent

        color: globalStyle.window
    }

    ColumnLayout {
        id: contentWrapper

        anchors.fill: parent
        anchors.margins: 20

        spacing: 8

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop

                spacing: 20

                TextLabel {
                    Layout.fillWidth: true

                    font.family: globalStyle.font.family
                    font.bold: true
                    font.pixelSize: 20
                    color: globalStyle.buttonText
                    horizontalAlignment: Qt.AlignHCenter

                    text: qsTr("Would you like to try our improved score style?")
                }

                Image {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredHeight: 199
                    Layout.preferredWidth: 548
                    fillMode: Image.PreserveAspectFit
                    source: "placeholder.png"
                }
            }

            ColumnLayout {
                Layout.fillWidth: true

                spacing: 8

                CheckBoxControl {
                    checked: root.model ? root.model.isLelandAllowed : false
                    text: qsTr("Our new professional notation font, Leland")

                    onToggled: {
                        root.model.isLelandAllowed = checked
                    }
                }

                CheckBoxControl {
                    checked: root.model ? root.model.isEdwinAllowed : false
                    text: qsTr("Our improved text font, Edwin")

                    onToggled: {
                        root.model.isEdwinAllowed = checked
                    }
                }

                CheckBoxControl {
                    visible: root.model ? root.model.isAutomaticPlacementAvailable : false
                    checked: root.model ? root.model.isAutomaticPlacementAllowed : false
                    text: qsTr("Automatic placement (spacing changes introduced in V3.0)")

                    onToggled: {
                        root.model.isAutomaticPlacementAllowed = checked
                    }
                }

                TextLabel {
                    Layout.topMargin: 12
                    Layout.fillWidth: true

                    font.family: globalStyle.font.family
                    font.pixelSize: 14
                    color: globalStyle.buttonText
                    horizontalAlignment: Qt.AlignLeft

                    text: root.model ? qsTr("Since this file was created in MuseScore %1, some layout changes may occur.").arg(root.model.creationAppVersion)
                                     : qsTr("(unknown)")
                }

                TextLabel {
                    Layout.topMargin: 12
                    Layout.fillWidth: true

                    font.family: globalStyle.font.family
                    font.pixelSize: 14
                    color: globalStyle.buttonText
                    horizontalAlignment: Qt.AlignLeft

                    text: "<a href=\"%1\">%2</a>".arg(Qt.locale().name === "zh_CN" ? "https://www.bilibili.com/video/BV1FT4y1K7UM" : "https://youtu.be/qLR40BGNy68").arg(qsTr("Watch our release video to learn more"))

                    onLinkActivated: {
                        if (root.model) {
                            root.model.showMoreDetails()
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: -12

            height: 2

            color: globalStyle.button
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBoxControl {
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true

                checked: root.model ? root.model.shouldNeverAskAgain : false
                text: qsTr("Remember my choice and don't ask again")

                onToggled: {
                    root.model.shouldNeverAskAgain = checked
                }
            }
        }

        StyledDialogButtonBox {
            Layout.preferredWidth: parent.width / 2
            Layout.alignment: Qt.AlignRight

            onRejected: if (root.model) root.model.ignore()
            onAccepted: if (root.model) root.model.apply()

            StyledButton {
                text: qsTr("Keep old style")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                focus: true
            }

            StyledButton {
                text: qsTr("Apply new style")
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                enabled: root.model ? root.model.isApplyingAvailable : false
            }
        }
    }
}
