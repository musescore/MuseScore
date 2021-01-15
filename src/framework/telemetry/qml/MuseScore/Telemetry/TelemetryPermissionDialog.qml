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

import QtQuick 2.12

import MuseScore.Telemetry 1.0
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

QmlDialog {
    id: root

    height: 500
    width: 460

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        TelemetryPermissionModel {
            id: permissionModel
        }

        Column {
            anchors.fill: parent
            anchors.leftMargin: 32
            anchors.rightMargin: 32

            spacing: 36

            Item {
                height: 1
                width: parent.width
            }

            StyledTextLabel {
                width: parent.width

                font: ui.theme.headerBoldFont

                text: qsTrc("telemetry", "Help us improve MuseScore")
            }

            Column {
                width: parent.width

                spacing: 24

                StyledTextLabel {
                    width: parent.width

                    text: qsTrc("telemetry", "We'd like to collect anonymous telemetry data to help us prioritize improvements. " +
                                "This includes how often you use certain features, statistics " +
                                "on preferred file formats, crashes, number of instruments per score, etc.")

                    horizontalAlignment: Qt.AlignLeft
                    wrapMode: Text.WordWrap
                }

                StyledTextLabel {
                    width: parent.width

                    text: qsTrc("telemetry", "We <u>do not</u> collect any personal data or sensitive information, such as " +
                                "location, source code, file names, or music")

                    font: ui.theme.bodyBoldFont
                    horizontalAlignment: Qt.AlignLeft
                    wrapMode: Text.WordWrap
                }

                StyledTextLabel {
                    text: qsTrc("telemetry", "Do you allow MuseScore to send us anonymous reports?")

                    horizontalAlignment: Qt.AlignLeft
                    wrapMode: Text.WordWrap
                }
            }

            Column {
                width: parent.width

                spacing: 32

                Column {
                    width: parent.width

                    spacing: 12

                    FlatButton {
                        anchors.horizontalCenter: parent.horizontalCenter

                        text: qsTrc("telemetry", "Yes, send anonymous reports")

                        onClicked: {
                            permissionModel.allowUseTelemetry()
                            root.hide()
                        }
                    }

                    StyledTextLabel {
                        width: parent.width

                        text: qsTrc("telemetry", "(You can change this behaviour any time in" +
                                    " Preferences… > General > Telemetry)")

                        wrapMode: Text.WordWrap
                    }
                }

                Column {
                    width: parent.width

                    spacing: 10

                    FlatButton {
                        anchors.horizontalCenter: parent.horizontalCenter

                        text: qsTrc("telemetry", "Don't send")

                        onClicked: {
                            permissionModel.forbidUseTelemetry()
                            root.hide()
                        }
                    }

                    StyledTextLabel {
                        anchors.horizontalCenter: parent.horizontalCenter

                        text: qsTrc("telemetry", "For more information, please take a look at our %1Privacy Policy%2").arg("<a href=\"https://musescore.com/legal/privacy\">").arg("</a>")

                        onLinkActivated: {
                            api.launcher.openUrl(link)
                        }
                    }
                }
            }

            Item {
                height: 1
                width: parent.width
            }
        }
    }
}
