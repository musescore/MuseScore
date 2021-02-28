import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.AppShell 1.0

QmlDialog {
    id: root

    fixedSize: Qt.size(456, 312)

    title: qsTrc("appshell", "About MusicXML")

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor

        AboutModel {
            id: aboutModel
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 30

            ColumnLayout {
                id: content

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: 36
                Layout.leftMargin: 40
                Layout.rightMargin: 40

                spacing: 24

                StyledTextLabel {
                    Layout.fillWidth: true

                    text: qsTrc("appshell", "MusicXML is an open file format for exchanging digital sheet music, supported by many applications.")
                    font: ui.theme.bodyBoldFont
                    wrapMode: Text.WordWrap
                }

                Column {
                    Layout.fillWidth: true
                    spacing: 12

                    StyledTextLabel {
                        width: parent.width

                        text: qsTrc("appshell", "Copyright 2004 - 2017 the Contributors to the MusicXML Specification, published by the W3C Music Notation Community Group under the W3C Community Contributor License Agreement (CLA):")
                        wrapMode: Text.WordWrap
                        maximumLineCount: 3
                    }

                    StyledTextLabel {
                        width: parent.width

                        text: {
                            var license = "<a href='%1'>%2</a>"
                            var musicXMLLicenseUrl = aboutModel.musicXMLLicenseUrl()
                            return license.arg(musicXMLLicenseUrl.url).arg(musicXMLLicenseUrl.displayName)
                        }
                        wrapMode: Text.WordWrap
                        maximumLineCount: 3
                    }
                }

                Column {
                    Layout.fillWidth: true
                    spacing: 12

                    StyledTextLabel {
                        width: parent.width
                        text: qsTrc("appshell", "A human-readable summary is available:")
                        wrapMode: Text.WordWrap
                        maximumLineCount: 3
                    }

                    StyledTextLabel {
                        width: parent.width

                        text: {
                            var license = "<a href='%1'>%2</a>"
                            var musicXMLLicenseDeedUrl = aboutModel.musicXMLLicenseDeedUrl()
                            return license.arg(musicXMLLicenseDeedUrl.url).arg(musicXMLLicenseDeedUrl.displayName)
                        }
                        wrapMode: Text.WordWrap
                        maximumLineCount: 3
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 16
                Layout.bottomMargin: 16

                spacing: 12

                FlatButton {
                    text: qsTrc("global", "OK")

                    onClicked: {
                        root.hide()
                    }
                }
            }
        }
    }
}
