import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.AppShell 1.0

QmlDialog {
    id: root

    fixedSize: Qt.size(480, 424)

    title: qsTrc("appshell", "About MuseScore")

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

                spacing: 32

                Image {
                    id: logo
                    Layout.alignment: Qt.AlignHCenter

                    source: "qrc:/qml/resources/mu_logo.svg"
                    sourceSize: Qt.size(100, 100)
                }

                Column {
                    spacing: 8
                    Layout.fillWidth: true

                    StyledTextLabel {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTrc("appshell", "Version: ") + aboutModel.museScoreVersion()
                        font: ui.theme.bodyBoldFont
                    }

                    Row {
                        spacing: 4
                        anchors.horizontalCenter: parent.horizontalCenter

                        StyledTextLabel {
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTrc("appshell", "Revision: ") + aboutModel.museScoreRevision()
                        }

                        FlatButton {
                            anchors.verticalCenter: parent.verticalCenter
                            icon: IconCode.COPY

                            onClicked: {
                                aboutModel.copyRevisionToClipboard()
                            }
                        }
                    }
                }

                StyledTextLabel {
                    Layout.fillWidth: true
                    text: {
                        var message = qsTrc("appshell", "Visit <a href='%1'>%2</a> for new versions and more information.<br>Support MuseScore with your <a href='%3'>%4</a>.")
                        var museScoreUrl = aboutModel.museScoreUrl()
                        var museScoreContributionUrl = aboutModel.museScoreContributionUrl()
                        return message
                        .arg(museScoreUrl.url)
                        .arg(museScoreUrl.displayName)
                        .arg(museScoreContributionUrl.url)
                        .arg(museScoreContributionUrl.displayName)
                    }
                    wrapMode: Text.WordWrap
                    maximumLineCount: 3
                }

                StyledTextLabel {
                    Layout.fillWidth: true
                    text: qsTrc("appshell", "Copyright © 1999-2021 MuseScore BVBA and others.\nPublished under the GNU General Public License.")
                    enabled: false
                    wrapMode: Text.WordWrap
                    maximumLineCount: 3
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
