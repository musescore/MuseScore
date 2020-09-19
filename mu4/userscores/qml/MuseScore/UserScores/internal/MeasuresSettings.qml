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

    StyledTextLabel {
        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter

        font.pixelSize: 16
        text: model.measureCount + qsTrc("userscores", " measures,\n Pickup: ") +
              model.pickupTimeSignature.numerator + "/" + model.pickupTimeSignature.denominator
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

        implicitHeight: 310
        implicitWidth: 320

        arrowX: root.arrowX
        x: popupPositionX
        y: popupPositionY

        Column {
            anchors.fill: parent
            anchors.margins: 10

            spacing: 30

            Column {
                anchors.left: parent.left
                anchors.right: parent.right

                spacing: 14

                CheckBox {
                    id: withPickupMeasure

                    anchors.left: parent.left
                    anchors.right: parent.right

                    checked: root.model.withPickupMeasure

                    text: qsTrc("userscores", "Show pickup measure")

                    onClicked: {
                        root.model.withPickupMeasure = !checked
                    }
                }

                TimeSignatureFraction {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    numerator: root.model.pickupTimeSignature.numerator
                    denominator: root.model.pickupTimeSignature.denominator
                    availableDenominators: root.model.timeSignatureDenominators()
                    active: withPickupMeasure.checked

                    onNumeratorSelected: {
                        root.model.setPickupTimeSignatureNumerator(value)
                    }

                    onDenominatorSelected: {
                        root.model.setPickupTimeSignatureDenominator(value)
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right

                height: 2

                color: ui.theme.buttonColor
            }


            Column {
                anchors.left: parent.left
                anchors.right: parent.right

                spacing: 14

                StyledTextLabel {
                    horizontalAlignment: Text.AlignLeft
                    text: qsTrc("userscores", "Initial number of measures")
                }

                IncrementalPropertyControl {
                    id: measuresCountControl

                    implicitWidth: 80

                    enabled: withPickupMeasure.checked

                    iconMode: iconModeEnum.hidden
                    currentValue: root.model.measureCount
                    step: 1

                    maxValue: root.model.measureCountRange().max
                    minValue: root.model.measureCountRange().min
                    validator: IntInputValidator {
                        top: measuresCountControl.maxValue
                        bottom: measuresCountControl.minValue
                    }

                    onValueEdited: {
                        root.model.measureCount = newValue
                    }
                }

                StyledTextLabel {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    horizontalAlignment: Text.AlignLeft
                    text: qsTrc("userscores", "Hint: You can also add & delete measures after you have created your score")
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                }
            }
        }
    }
}
