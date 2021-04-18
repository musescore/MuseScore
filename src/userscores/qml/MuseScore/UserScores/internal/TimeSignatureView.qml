import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Item {
    id: root

    property var numerator: 0
    property var denominator: 0
    property var type: AdditionalInfoModel.Fraction

    height: contentColumn.height
    width: contentColumn.width

    Column {
        id: contentColumn
        visible: parent.type === AdditionalInfoModel.Fraction
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        spacing: 0

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 0

            Repeater {
                model: root.numerator
                StyledIconLabel {
                    font.family: ui.theme.musicalFont.family
                    font.pixelSize: 60
                    lineHeightMode: Text.FixedHeight
                    lineHeight: 30
                    iconCode: modelData
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 0

            Repeater {
                model: root.denominator
                StyledIconLabel {
                    font.family: ui.theme.musicalFont.family
                    font.pixelSize: 60
                    lineHeightMode: Text.FixedHeight
                    lineHeight: 30
                    iconCode: modelData
                }
            }
        }
    }

    StyledIconLabel {
        id: iconLabel
        visible: parent.type !== AdditionalInfoModel.Fraction
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        font.family: ui.theme.musicalFont.family
        font.pixelSize: 60
        iconCode: parent.type === AdditionalInfoModel.Common ? MusicalSymbolCodes.TIMESIG_COMMON
                                                             : MusicalSymbolCodes.TIMESIG_CUT
    }
}
