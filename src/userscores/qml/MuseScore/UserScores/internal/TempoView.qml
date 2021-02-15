import QtQuick 2.9

import MuseScore.UiComponents 1.0

Item {
    id: root

    property string tempoNoteSymbol: ""
    property int tempoValue: 0

    height: contentColumn.height
    width: contentColumn.width

    Row {
        id: contentColumn

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        spacing: 0

        StyledTextLabel {
            topPadding: 22
            font.family: ui.theme.musicalFont.family
            font.pixelSize: 36
            font.letterSpacing: 1
            lineHeightMode: Text.FixedHeight
            lineHeight: 10
            text: root.tempoNoteSymbol
        }

        StyledTextLabel {
            font: ui.theme.headerFont
            text: " = " + root.tempoValue
        }
    }
}
