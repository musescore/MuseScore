import QtQuick 2
import QtQuick.Layouts 1

import MuseScore.UiComponents 1
import MuseScore.UserScores 1

StyledTextLabel {
    property ExportDialogModel model
    property int firstColumnWidth

    text: qsTrc("userscores", "Each page of the selected parts will be exported as a separate SVG file.")
    horizontalAlignment: Text.AlignLeft
    wrapMode: Text.WordWrap
}
