import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

FlatButton {
    id: root

    property var model: null

    height: 96
    accentButton: popup.visible

    QtObject {
        id: privateProperties

        function formatNoteSymbol(noteIcon, withDot) {
            var iconStr = String.fromCharCode(noteIcon)

            if (Boolean(withDot)) {
                return iconStr + String.fromCharCode(MusicalSymbolCodes.DOT)
            }

            return iconStr
        }
    }

    TempoView {
        id: tempoView

        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter

        tempoValue: root.model.tempo.value
        tempoNoteSymbol: privateProperties.formatNoteSymbol(model.tempo.noteIcon, model.tempo.withDot)
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

        x: root.x - (width - root.width) / 2
        y: root.height

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

            SeparatorLine {
                anchors.leftMargin: -(parent.anchors.leftMargin + popup.leftPadding)
                anchors.rightMargin: -(parent.anchors.rightMargin + popup.rightPadding)
            }

            RadioButtonGroup {
                anchors.horizontalCenter: parent.horizontalCenter

                height: 50

                model: root.model.tempoMarks()

                clip: true
                boundsBehavior: ListView.StopAtBounds

                delegate: FlatRadioButton {
                    width: 48
                    height: width

                    enabled: withTempo.checked

                    onClicked: {
                        var tempo = root.model.tempo
                        tempo.noteIcon = modelData.noteIcon
                        tempo.withDot = modelData.withDot
                        root.model.tempo = tempo
                    }

                    StyledTextLabel {
                        topPadding: 24
                        font.family: ui.theme.musicalFont.family
                        font.pixelSize: 24
                        lineHeightMode: Text.FixedHeight
                        lineHeight: 10
                        text: privateProperties.formatNoteSymbol(modelData.noteIcon, modelData.withDot)
                    }
                }
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: 20
                enabled: withTempo.checked

                StyledTextLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "="
                    font: ui.theme.headerFont
                }

                IncrementalPropertyControl {
                    id: control

                    implicitWidth: 126

                    iconMode: iconModeEnum.hidden
                    currentValue: root.model.tempo.value
                    step: 1

                    maxValue: root.model.tempoRange().max
                    minValue: root.model.tempoRange().min
                    validator: IntInputValidator {
                        top: control.maxValue
                        bottom: control.minValue
                    }

                    onValueEdited: {
                        var tempo = root.model.tempo
                        tempo.value = newValue
                        root.model.tempo = tempo
                    }
                }
            }
        }
    }
}
