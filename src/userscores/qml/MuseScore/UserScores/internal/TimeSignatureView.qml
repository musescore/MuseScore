import QtQuick 2.9

import MuseScore.UiComponents 1.0

Item {
    id: root

    property var numerator: 0
    property var denominator: 0

    height: contentColumn.height
    width: contentColumn.width

    Column {
        id: contentColumn
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
}
