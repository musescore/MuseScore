import QtQuick 2.12
import QtQuick.Controls 2.12

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    GridViewSectional {
        id: gridView

        anchors.fill: parent

        sectionRole: "sectionRole"

        cellWidth: 36
        cellHeight: cellWidth

        sectionWidth: 2
        sectionHeight: root.height

        rows: 1

        model: toolModel

        sectionDelegate: Rectangle {
            color: ui.theme.strokeColor
        }

        itemDelegate: FlatButton {
            id: control

            property var item: Boolean(itemModel) ? itemModel : null

            normalStateColor: Boolean(item) && item.checkedRole ? ui.theme.accentColor : "transparent"

            icon: Boolean(item) ? item.iconRole : null

            onClicked: {
                toolModel.click(item.nameRole)
            }
        }
    }

    NotationToolBarModel {
        id: toolModel
    }

    Component.onCompleted: {
        toolModel.load()
    }
}
