import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Item {
    id: root

    width: childrenRect.width
    height: childrenRect.height

    ViewModeControlModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    FlatButton {
        normalStateColor: menu.isOpened ? ui.theme.accentColor : "transparent"

        text: Boolean(model.currentViewMode) ? model.currentViewMode.title : ""
        icon: Boolean(model.currentViewMode) ? model.currentViewMode.icon : IconCode.NONE

        visible: Boolean(model.currentViewMode)

        orientation: Qt.Horizontal

        StyledIconLabel {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            iconCode: IconCode.SMALL_ARROW_DOWN
        }

        onClicked: {
            if (menu.isOpened) {
                menu.toggleOpened()
                return
            }

            menu.toggleOpened()
        }

        StyledMenu {
            id: menu

            opensUpward: true
            itemWidth: 220

            model: model.items

            onHandleAction: {
                Qt.callLater(model.selectViewMode, actionCode)
                menu.close()
            }
        }
    }
}
