import QtQuick 2
import QtQuick.Layouts 1

import MuseScore.UiComponents 1
import MuseScore.UserScores 1

ColumnLayout {
    id: root
    spacing: 12

    property ExportDialogModel model
    property int firstColumnWidth

    RadioButtonGroup {
        spacing: 12
        orientation: Qt.Vertical
        Layout.fillWidth: true
        model: root.model.musicXmlLayoutTypes()

        delegate: RoundedRadioButton {
            text: modelData["text"]
            width: parent.width
            checked: root.model.musicXmlLayoutType === modelData["value"]
            onToggled: {
                root.model.musicXmlLayoutType = modelData["value"]
            }
        }
    }
}
