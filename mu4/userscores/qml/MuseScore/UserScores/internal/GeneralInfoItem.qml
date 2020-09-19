import QtQuick 2.9

import MuseScore.UiComponents 1.0

Column {
    id: root

    property string title: ""
    property alias info: textField.currentText

    spacing: 10

    StyledTextLabel {
        anchors.left: parent.left
        anchors.right: parent.right

        font.bold: true
        horizontalAlignment: Text.AlignLeft
        text: title
    }

    TextInputField {
        id: textField
        hint: qsTrc("userscores", "Optional")

        onCurrentTextEdited: {
            root.info = newTextValue
        }
    }
}

