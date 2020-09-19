import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

FlatButton {
    id: root

    height: 96

    property var model: null

    property var arrowX
    property var popupPositionX
    property var popupPositionY: height
    property alias oppened: popup.visible

    backgroundColor: oppened ? ui.theme.accentColor : ui.theme.buttonColor

    TempoView {
        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter

        tempoNote: MusicalSymbolCodes.CROTCHET // TODO: get from model
        tempo: model.tempo
    }

    onClicked: {
        if (!popup.isOpened) {
            popup.open()
        } else {
            popup.close()
        }
    }

    StyledPopup {
        id: popup

        implicitHeight: 250
        implicitWidth: 520

        arrowX: root.arrowX
        x: popupPositionX
        y: popupPositionY

        Column {
            anchors.fill: parent
            anchors.margins: 10

            spacing: 30

            CheckBox {
                id: withTempo

                anchors.left: parent.left
                anchors.right: parent.right

                checked: root.model.withTempo

                text: qsTrc("userscores", "Show tempo marking on my score")

                onClicked: {
                    root.model.withTempo = !checked
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right

                height: 2

                color: ui.theme.buttonColor
            }

            ListView {
                anchors.left: parent.left
                anchors.right: parent.right

                height: 50

                model: root.model.tempoMarks()
                orientation: ListView.Horizontal
                spacing: 4

                clip: true
                boundsBehavior: ListView.StopAtBounds

                delegate: FlatButton {
                    width: 48
                    height: width

                    enabled: withTempo.checked

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter

                        StyledIconLabel {
                            topPadding: 14
                            font.family: ui.theme.musicalFont.family
                            font.pixelSize: 24
                            lineHeightMode: Text.FixedHeight
                            lineHeight: 10
                            iconCode: modelData.icon
                        }

                        StyledIconLabel {
                            topPadding: 14
                            font.family: ui.theme.musicalFont.family
                            font.pixelSize: 24
                            lineHeightMode: Text.FixedHeight
                            lineHeight: 10
                            iconCode: MusicalSymbolCodes.DOT

                            visible: Boolean(modelData.withDot)
                        }
                    }
                }
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: 20

                StyledTextLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "="
                }

                IncrementalPropertyControl {
                    id: control

                    implicitWidth: 126

                    enabled: withTempo.checked

                    iconMode: iconModeEnum.hidden
                    currentValue: root.model.tempo
                    step: 1

                    maxValue: root.model.tempoRange().max
                    minValue: root.model.tempoRange().min
                    validator: IntInputValidator {
                        top: control.maxValue
                        bottom: control.minValue
                    }

                    onValueEdited: {
                        root.model.tempo = newValue
                    }
                }
            }
        }
    }
}
