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

import QtQuick 2.0
import MuseScore.Telemetry 3.3

FocusScope {
    id: root

    signal closeRequested

    height: childrenRect.height

    Keys.onEscapePressed: {
        root.closeRequested()
    }

    Rectangle {
        anchors.fill: parent

        color: globalStyle.window
    }

    TelemetryPermissionModel {
        id: permissionModel
    }

    Column {
        id: contentWrapper

        width: root.width

        spacing: 36

        Item {
            id: topSpacer

            height: 1
            width: parent.width
        }

        TextLabel {
            id: titleLabel

            anchors {
                left: parent.left
                right: parent.right
            }

            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap

            color: "#00447a"
            font.pixelSize: 28

            text: qsTr("Help us improve MuseScore")
        }

        Column {
            id: messageContentWrapper

            anchors {
                left: parent.left
                leftMargin: 32
                right: parent.right
                rightMargin: 32
            }

            spacing: 24

            TextLabel {
                id: intent

                lineHeight: 1.5

                text: qsTr("We'd like to collect anonymous telemetry data to help us prioritize improvements. " +
                           "This includes how often you use certain features, statistics " +
                           "on preferred file formats, crashes, number of instruments per score, etc.")
            }

            TextLabel {
                id: ignoredDataTypesLabel

                anchors {
                    left: parent.left
                    leftMargin: 10
                    right: parent.right
                    rightMargin: 10
                }

                lineHeight: 1.5

                text: "<b>" + qsTr("We <u>do not</u> collect any personal data or sensitive information, such as " +
                           "location, source code, file names, or music") + "</b>"
            }

            TextLabel {
                id: questionLabel

                text: qsTr("Do you allow MuseScore to send us anonymous reports?")
            }
        }

        Column {
            id: buttonWrapper

            width: parent.width

            spacing: 24

            Column {

                width: parent.width

                spacing: 10

                DialogButton {
                    id: positiveButton

                    anchors {
                        horizontalCenter: parent.horizontalCenter
                    }

                    text: qsTr("Yes, send anonymous reports")

                    onClicked: {
                        permissionModel.accept()
                        root.closeRequested()
                    }
                }

                TextLabel {
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                    }

                    text: qsTr("(You can change this behaviour any time in 'Preferences… > General > Telemetry')")
                }
            }

            Column {

                width: parent.width

                spacing: 10

                DialogButton {
                    id: negativeButton

                    anchors {
                        horizontalCenter: parent.horizontalCenter
                    }

                    text: qsTr("Don't send")

                    onClicked: {
                        permissionModel.reject()
                        root.closeRequested()
                    }
                }

                TextLabel {
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                    }

                    text: qsTr("For more information, please take a look at our %1Privacy Policy%2").arg("<a href=\"https://musescore.com/legal/privacy\">").arg("</a>")

                    onLinkActivated: {
                        permissionModel.openLink(link)
                    }
                }
            }
        }

        Item {
            id: bottomSpacer

            height: 1
            width: parent.width
        }
    }
}
