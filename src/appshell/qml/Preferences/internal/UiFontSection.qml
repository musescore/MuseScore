import QtQuick 2.15

import MuseScore.UiComponents 1.0

Column {
    id: root

    property alias allFonts: selectFontControl.model

    property alias currentFontIndex: selectFontControl.currentIndex
    property alias bodyTextSize: bodyTextSizeControl.currentValue

    property int firstColumnWidth: 0

    signal fontChangeRequested(var newFontIndex)
    signal bodyTextSizeChangeRequested(var newBodyTextSize)

    spacing: 18

    StyledTextLabel {
        text: qsTrc("appshell", "Appearance")
        font: ui.theme.bodyBoldFont
    }

    Column {
        spacing: 8

        ComboBoxWithTitle {
            id: selectFontControl

            title: qsTrc("appshell", "Font face:")
            titleWidth: root.firstColumnWidth

            onValueEdited: {
                root.fontChangeRequested(currentIndex)
            }
        }

        IncrementalPropertyControlWithTitle {
            id: bodyTextSizeControl

            title: qsTrc("appshell", "Body text size:")
            titleWidth: root.firstColumnWidth

            minValue: 8
            maxValue: 24
            measureUnitsSymbol: qsTrc("appshell", "pt")

            onValueEdited: {
                root.bodyTextSizeChangeRequested(newValue)
            }
        }
    }
}
