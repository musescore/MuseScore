import QtQuick 2.12
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

QmlDialog {
    id: root

    width: 534
    height: 146

    modal: true

    property string operation: ""

    Rectangle {
        id: content

        anchors.fill: parent

        color: ui.theme.popupBackgroundColor

        property int measuresCount: 1

        QtObject {
            id: privateProperties

            readonly property int sideMargin: 16

            function capitalizeFirstLetter(string) {
                return string.charAt(0).toUpperCase() + string.slice(1);
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: privateProperties.sideMargin

            spacing: 20

            Column {
                Layout.fillWidth: true

                spacing: 12

                StyledTextLabel {
                    text: privateProperties.capitalizeFirstLetter(operation) + " " + qsTrc("notation", "empty measures")
                    font: ui.theme.bodyBoldFont
                }

                RowLayout {
                    Layout.fillWidth: true

                    spacing: 12

                    StyledTextLabel {
                        Layout.fillWidth: true
                        text: qsTrc("notation", "Number of measures to ") + operation.toLowerCase() + ":"
                    }

                    IncrementalPropertyControl {
                        id: countMeasuresInputField

                        Layout.alignment: Qt.AlignRight
                        Layout.preferredWidth: 100

                        iconMode: iconModeEnum.hidden
                        currentValue: content.measuresCount
                        step: 1

                        maxValue: 1000
                        minValue: 1
                        validator: IntInputValidator {
                            top: countMeasuresInputField.maxValue
                            bottom: countMeasuresInputField.minValue
                        }

                        onValueEdited: {
                            content.measuresCount = newValue
                        }
                    }
                }
            }

            Row {
                Layout.preferredHeight: childrenRect.height
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom

                spacing: 12

                FlatButton {
                    text: qsTrc("global", "Cancel")

                    onClicked: {
                        root.reject()
                    }
                }

                FlatButton {
                    text: qsTrc("global", "OK")

                    onClicked: {
                        root.ret = {errcode: 0, value: content.measuresCount}
                        root.hide()
                    }
                }
            }
        }
    }
}
