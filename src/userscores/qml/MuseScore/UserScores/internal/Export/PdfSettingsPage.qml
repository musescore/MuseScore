import QtQuick 2
import QtQuick.Layouts 1

import MuseScore.UiComponents 1
import MuseScore.UserScores 1

ColumnLayout {
    id: root

    property ExportDialogModel model
    property int firstColumnWidth

    ExportOptionItem {
        text: qsTrc("userscores", "Resolution:")
        firstColumnWidth: root.firstColumnWidth

        IncrementalPropertyControl {
            Layout.preferredWidth: 80

            currentValue: root.model.pdfResolution

            minValue: 72
            maxValue: 2400
            step: 1
            decimals: 0
            measureUnitsSymbol: qsTrc("userscores", "dpi")

            onValueEdited: {
                root.model.pdfResolution = newValue
            }
        }
    }
}
