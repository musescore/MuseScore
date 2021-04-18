import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

RowLayout {
    id: root

    property alias noteSymbol: noteSymbolLabel.text
    property var tempoValue: 0

    property alias noteSymbolFont: noteSymbolLabel.font
    property alias tempoValueFont: tempoValueLabel.font

    property alias noteSymbolTopPadding: noteSymbolLabel.topPadding

    spacing: 0

    StyledTextLabel {
        id: noteSymbolLabel

        topPadding: 10
        lineHeightMode: Text.FixedHeight
        lineHeight: 10

        font.family: ui.theme.musicalFont.family
        font.pixelSize: ui.theme.musicalFont.pixelSize
        font.letterSpacing: 1
    }

    StyledTextLabel {
        id: tempoValueLabel

        text: " = " + root.tempoValue
    }
}
