import QtQuick 2.15

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    NotationToolBarModel {
        id: toolbarModel
    }

    Component.onCompleted: {
        toolbarModel.load()
    }

    ListView {
        id: view

        anchors.verticalCenter: parent.verticalCenter

        width: contentWidth
        height: contentItem.childrenRect.height

        orientation: Qt.Horizontal
        interactive: false
        spacing: 2

        model: toolbarModel

        delegate: FlatButton {
            text: model.title
            icon: model.icon
            enabled: model.enabled
            textFont: ui.theme.tabFont

            normalStateColor: "transparent"
            orientation: Qt.Horizontal

            onClicked: {
                toolbarModel.open(model.index)
            }
        }
    }
}
