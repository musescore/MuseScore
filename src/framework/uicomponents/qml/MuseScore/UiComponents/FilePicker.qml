import QtQuick 2.15
import Qt.labs.platform 1.1

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    property alias path: pathField.currentText

    property alias dialogTitle: model.title
    property alias filter: model.filter
    property alias dir: model.dir

    signal pathEdited(var newPath)

    width: 338
    height: childrenRect.height

    FilePickerModel {
        id: model
    }

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
            var file = model.selectFile()
            pathField.currentText = file
            root.pathEdited(file)
        }
    }
}
