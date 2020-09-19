import QtQuick 2.9

import MuseScore.UiComponents 1.0

Item {
    id: root

    property var tempoNote: null
    property var tempo: 0

    height: contentColumn.height
    width: contentColumn.width

    Row {
        id: contentColumn
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        spacing: 0

        StyledIconLabel {
            topPadding: 22
            font.family: ui.theme.musicalFont.family
            font.pixelSize: 36
            lineHeightMode: Text.FixedHeight
            lineHeight: 10
            iconCode: root.tempoNote
        }

        StyledTextLabel {
            font.pixelSize: 20
            text: " = " + root.tempo
        }
    }
}
