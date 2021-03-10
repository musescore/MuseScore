import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

import "internal"

QmlDialog {
    id: root

    width: 756
    height: 396

    modal: true

    title: qsTrc("userscores", "Export")

    Rectangle {
        id: content

        anchors.fill: parent

        color: ui.theme.popupBackgroundColor

        ExportScoreModel {
            id: scoresModel
        }

        ExportScoreSuffixModel {
            id: suffixModel
        }

        ExportScoreSettingsModel {
            id: settingsModel
        }

        QtObject {
            id: privateProperties

            readonly property int sideMargin: 24
        }

        Component.onCompleted: {
            scoresModel.load();
            suffixModel.load();
            settingsModel.load();

            scoresModel.selectCurrentNotation()
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: privateProperties.sideMargin
            spacing: 2 * privateProperties.sideMargin

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width / 2 - parent.spacing / 2
                spacing: 18

                StyledTextLabel {
                    text: qsTrc("userscores", "Select parts to export")
                    font: ui.theme.bodyBoldFont
                }

                ExportScoresListView {
                    id: exportScoresListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    scoresModel: scoresModel
                }

                RowLayout {
                    Layout.topMargin: 10
                    spacing: 12

                    FlatButton {
                        Layout.fillWidth: true

                        text: qsTrc("userscores", "Select all")

                        onClicked: {
                            scoresModel.setAllSelected(true)
                        }
                    }

                    FlatButton {
                        Layout.fillWidth: true

                        text: qsTrc("userscores", "Clear selection")

                        onClicked: {
                            scoresModel.setAllSelected(false)
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width / 2 - parent.spacing / 2
                spacing: 18

                StyledTextLabel {
                    text: qsTrc("userscores", "Export settings")
                    font: ui.theme.bodyBoldFont
                }

                ExportOptionsView {
                    id: exportOptionsView
                    Layout.fillHeight: true

                    scoresModel: scoresModel
                    suffixModel: suffixModel
                    settingsModel: settingsModel
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    Layout.topMargin: 10
                    spacing: 12

                    FlatButton {
                        text: qsTrc("global", "Close")

                        onClicked: {
                            root.hide()
                        }
                    }

                    FlatButton {
                        id: exportButton

                        text: qsTrc("userscores", "Export…")
                        enabled: scoresModel.selectionLength > 0;
                        accentButton: enabled
                        onClicked: {
                            if (scoresModel.exportScores()) {
                                root.hide();
                            }
                        }
                    }
                }
            }
        }
    }
}
