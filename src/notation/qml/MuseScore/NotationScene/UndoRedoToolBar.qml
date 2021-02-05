import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    UndoRedoModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    Row {
        anchors.centerIn: parent

        height: childrenRect.height
        spacing: 2

        FlatButton {
            icon: model.undoItem.icon
            hint: model.undoItem.description
            enabled: model.undoItem.enabled
            normalStateColor: "transparent"

            onClicked: {
                model.undo()
            }
        }

        FlatButton {
            icon: model.redoItem.icon
            hint: model.redoItem.description
            enabled: model.redoItem.enabled
            normalStateColor: "transparent"

            onClicked: {
                model.redo()
            }
        }
    }
}
