import QtQuick 2.15

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    property alias isToolBarVisible: toolbarModel.isToolBarVisible

    NotationToolBarModel {
        id: toolbarModel
    }

    function load() {
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
            iconFont: ui.theme.toolbarIconsFont

            hint: model.hint
            enabled: model.enabled
            textFont: ui.theme.tabFont

            normalStateColor: "transparent"
            orientation: Qt.Horizontal

            onClicked: {
                toolbarModel.handleAction(model.code)
            }
        }
    }
}
