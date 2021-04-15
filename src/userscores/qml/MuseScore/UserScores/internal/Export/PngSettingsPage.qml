import QtQuick 2
import QtQuick.Layouts 1

import MuseScore.UiComponents 1
import MuseScore.UserScores 1

ColumnLayout {
    id: root
    spacing: 12

    property ExportDialogModel model
    property int firstColumnWidth

    ExportOptionItem {
        text: qsTrc("userscores", "Resolution:")
        firstColumnWidth: root.firstColumnWidth

        IncrementalPropertyControl {
            Layout.preferredWidth: 80
            currentValue: root.model.pngResolution
            minValue: 32
            maxValue: 5000
            step: 1
            decimals: 0
            measureUnitsSymbol: qsTrc("userscores", "dpi")
            onValueEdited: {
                root.model.pngResolution = newValue
            }
        }
    }

    CheckBox {
        text: qsTrc("userscores", "Transparent background")
        checked: root.model.pngTransparentBackground
        onClicked: {
            root.model.pngTransparentBackground = !checked
        }
    }

    StyledTextLabel {
        Layout.fillWidth: true
        text: qsTrc("userscores", "Each page of the selected parts will be exported as a separate PNG file.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
    }
}
