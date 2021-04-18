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

    TimeSignatureView {
        id: timeSignatureView

        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter

        numerator: root.model.musicSymbolCodes(root.model.timeSignature.numerator)
        denominator: root.model.musicSymbolCodes(root.model.timeSignature.denominator)
        type: root.model.timeSignatureType
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

        implicitHeight: radioButtonList.height + topPadding + bottomPadding + 40
        implicitWidth: 310

        x: root.x - (width - root.width) / 2
        y: root.height

        RadioButtonGroup {
            id: radioButtonList

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20
            height: implicitHeight

            spacing: 30

            orientation: ListView.Vertical

            model: [
                { comp: fractionComp, valueRole: AdditionalInfoModel.Fraction },
                { comp: commonComp, valueRole: AdditionalInfoModel.Common },
                { comp: cutComp, valueRole: AdditionalInfoModel.Cut }
            ]

            delegate: RoundedRadioButton {
                id: timeFractionButton

                ButtonGroup.group: radioButtonList.radioButtonGroup
                width: parent.width

                spacing: 30

                contentComponent: modelData["comp"]
                checked: (root.model.timeSignatureType === modelData["valueRole"])

                onToggled: {
                    root.model.timeSignatureType = modelData["valueRole"]
                }
            }
        }
    }

    Component {
        id: fractionComp

        TimeSignatureFraction {
            anchors.fill: parent

            enabled: (root.model.timeSignatureType === AdditionalInfoModel.Fraction)
            availableDenominators: root.model.timeSignatureDenominators()

            numerator: enabled ? root.model.timeSignature.numerator : numerator
            denominator: enabled ? root.model.timeSignature.denominator : denominator

            onNumeratorSelected: {
                root.model.setTimeSignatureNumerator(value)
            }

            onDenominatorSelected: {
                root.model.setTimeSignatureDenominator(value)
            }
        }
    }

    Component {
        id: commonComp

        StyledIconLabel {
            horizontalAlignment: Text.AlignLeft
            font.family: ui.theme.musicalFont.family
            font.pixelSize: 30
            iconCode: MusicalSymbolCodes.TIMESIG_COMMON
        }
    }

    Component {
        id: cutComp

        StyledIconLabel {
            horizontalAlignment: Text.AlignLeft
            font.family: ui.theme.musicalFont.family
            font.pixelSize: 30
            iconCode: MusicalSymbolCodes.TIMESIG_CUT
        }
    }
}
