import QtQuick 2.15
import MuseScore.Ui 1.0
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    property alias keynav: keynavSub

    signal activeFocusRequested()

    Component.onCompleted: {
        toolbarModel.load()
    }

    KeyNavigationSubSection {
        id: keynavSub
        name: "NotationToolBar"
        onActiveChanged: {
            if (active) {
                root.activeFocusRequested()
                root.forceActiveFocus()
            }
        }
    }

    NotationToolBarModel {
        id: toolbarModel
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

            keynav.subsection: keynavSub
            keynav.name: model.title
            keynav.order: model.index
            keynav.enabled: model.enabled

            normalStateColor: "transparent"
            orientation: Qt.Horizontal

            onClicked: {
                toolbarModel.handleAction(model.code)
            }
        }
    }
}
