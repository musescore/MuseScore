import QtQuick 2.7

import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Row {
    spacing: 2

    height: childrenRect.height

    UndoRedoModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    } 

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
