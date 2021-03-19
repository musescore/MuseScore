import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0

Column {
    id: root

    property var allFonts: []

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

    GridLayout {
        rows: 2
        columns: 2

        rowSpacing: 8
        columnSpacing: 0

        Item {
            width: root.firstColumnWidth
            height: childrenRect.height

            StyledTextLabel {
                horizontalAlignment: Qt.AlignLeft
                text: qsTrc("appshell", "Font face:")
            }
        }

        StyledComboBox {
            id: selectFontControl

            implicitWidth: 208

            textRoleName: "text"
            valueRoleName: "value"

            model: {
                var result = []

                for (var i = 0; i < root.allFonts.length; ++i) {
                    result.push({"text" : root.allFonts[i], "value" : root.allFonts[i]})
                }

                return result
            }

            onActivated: {
                root.fontChangeRequested(index)
            }
        }

        Item {
            width: root.firstColumnWidth
            height: childrenRect.height

            StyledTextLabel {
                horizontalAlignment: Qt.AlignLeft
                text: qsTrc("appshell", "Body text size:")
            }
        }

        IncrementalPropertyControl {
            id: bodyTextSizeControl

            implicitWidth: 112

            minValue: 0
            maxValue: 100
            decimals: 0
            step: 1

            measureUnitsSymbol: qsTrc("appshell", "pt")

            onValueEdited: {
                root.bodyTextSizeChangeRequested(newValue)
            }
        }
    }
}
