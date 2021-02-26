import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

import "internal"

QmlDialog {
    id: root

    width: 640
    height: 480

    modal: true

    title: qsTrc("userscores", "Export Score")

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
            readonly property int buttonsMargin: 24
            readonly property int rectangleRadius: 4
        }


        Component.onCompleted: {
            scoresModel.load();
            suffixModel.load();
            settingsModel.load();

            settingsModel.changeType(fileTypeSelect.value)
        }

        ColumnLayout {

            anchors.fill: parent

            spacing: 0

            StyledTextLabel {
                anchors.left: parent.left
                anchors.leftMargin: privateProperties.sideMargin
                Layout.topMargin: privateProperties.sideMargin

                text: qsTrc("global", "Export")
                font: ui.theme.headerBoldFont
            }

            RowLayout {

                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: childrenRect.height
                spacing: 12

                width: parent.width - spacing
                Layout.bottomMargin: privateProperties.sideMargin
                Layout.topMargin: privateProperties.sideMargin


                ExportScoresListView {
                    id: scoresList

                    model: scoresModel

                    Layout.preferredWidth: parent.width / 2 - parent.spacing / 2 - Layout.leftMargin
                    Layout.fillHeight: true
                }

                Rectangle {
                    id: rightRectangle

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    color: ui.theme.popupBackgroundColor

                    Column {
                        Layout.fillHeight: true
                        Layout.fillWidth: true

                        spacing: 12

                        Column {
                            Layout.fillWidth: true
                            Layout.preferredHeight: childrenRect.height

                            spacing: 6

                            StyledTextLabel {
                                Layout.fillWidth: true
                                text: qsTrc("userscores", "Export To:")
                                font.capitalization: Font.AllUppercase
                            }

                            Row {
                                id: exportToPath

                                Layout.fillWidth: true
                                Layout.preferredHeight: childrenRect.height

                                spacing: 6

                                TextInputField {
                                    id: fileExportPathInput

                                    height: 30
                                    width: rightRectangle.width * 0.8

                                    currentText: scoresModel.getExportPath()

                                    onCurrentTextEdited: {
                                        scoresModel.setExportPath(newTextValue);
                                    }
                                }

                                FlatButton {
                                    id: browsePathButton

                                    icon: IconCode.NEW_FILE

                                    height:30
                                    width: rightRectangle.width * 0.1

                                    onClicked: {
                                        fileExportPathInput.currentText = scoresModel.chooseExportPath();
                                    }
                                }

                            }
                        }


                        Column {
                            id: exportSuffixSelection

                            Layout.fillWidth: true
                            Layout.preferredHeight: childrenRect.height

                            spacing: 6

                            StyledTextLabel {
                                Layout.fillWidth: true
                                Layout.topMargin: 24

                                text: qsTrc("userscores", "Export As: ")
                                font.capitalization: Font.AllUppercase

                            }

                            StyledComboBox {
                                id: fileTypeSelect

                                property var currValue: suffixModel.getDefaultRow()

                                width: rightRectangle.width * 0.8

                                textRoleName: "suffix"
                                valueRoleName: "value"

                                model: suffixModel

                                currentIndex: indexOfValue(currValue)

                                onValueChanged: {
                                    currValue = value;
                                    scoresModel.setExportSuffix(valueFromModel(indexOfValue(value), "suffix"));
                                    fileExportPathInput.currentText = scoresModel.getExportPath();

                                    settingsModel.changeType(valueFromModel(indexOfValue(value), "suffix"));
                                }
                            }
                        }


                        Column {
                            id: exportSettingsContainer

                            anchors.top: exportSuffixSelection.bottom

                            ExportFilePages {
                                model: settingsModel
                            }
                        }

                    }

                }

            }

            Row {
                Layout.preferredHeight: childrenRect.height
                Layout.bottomMargin: privateProperties.buttonsMargin
                Layout.rightMargin: privateProperties.buttonsMargin
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom

                spacing: 12

                FlatButton {
                    text: qsTrc("global", "Close")

                    onClicked: {
                        root.hide()
                    }
                }

                FlatButton {
                    id: exportButton

                    text: qsTrc("global", "Export")

                    accentButton: true

                    onClicked: {
                        scoresModel.exportScores();
                        root.hide();
                    }
                }
            }

        }

    }

}
