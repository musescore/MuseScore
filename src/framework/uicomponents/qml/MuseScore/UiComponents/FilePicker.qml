import QtQuick 2.15
import Qt.labs.platform 1.1

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    property alias path: pathField.currentText

    signal pathEdited(var newPath)

    width: 338
    height: childrenRect.height

    TextInputField {
        id: pathField

        width: parent.width

        onCurrentTextEdited: {
            root.pathEdited(newTextValue)
        }
    }

    FlatButton {
        anchors.left: pathField.right
        anchors.leftMargin: 8

        icon: IconCode.OPEN_FILE

        onClicked: {
            dialog.open()
        }
    }

    FileDialog {
        id: dialog

        onAccepted: {
            pathField.currentText = dialog.file
            root.pathEdited(dialog.file)
        }
    }
}
