import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    property alias isToolBarVisible: model.isToolBarVisible

    function load() {
        model.load()
    }

    UndoRedoModel {
        id: model
    }

    Row {
        anchors.centerIn: parent

        height: childrenRect.height
        spacing: 2

        FlatButton {
            icon: model.undoItem.icon
            iconFont: ui.theme.toolbarIconsFont

            hint: model.undoItem.description
            enabled: model.undoItem.enabled
            normalStateColor: "transparent"

            onClicked: {
                model.undo()
            }
        }

        FlatButton {
            icon: model.redoItem.icon
            iconFont: ui.theme.toolbarIconsFont

            hint: model.redoItem.description
            enabled: model.redoItem.enabled
            normalStateColor: "transparent"

            onClicked: {
                model.redo()
            }
        }
    }
}
