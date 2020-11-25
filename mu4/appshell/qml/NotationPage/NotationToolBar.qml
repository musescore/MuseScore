import QtQuick 2.12
import QtQuick.Controls 2.12

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    GridViewSectional {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

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
            property var item: Boolean(itemModel) ? itemModel : null

            normalStateColor: Boolean(item) && item.checkedRole ? ui.theme.accentColor : "transparent"

            icon: Boolean(item) ? item.iconRole : null

            onClicked: {
                toolModel.click(item.nameRole)
            }
        }
    }

    FlatButton {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        icon: IconCode.CONFIGURE
        normalStateColor: "transparent"

        onClicked: {
            api.launcher.open("musescore://notation/noteinputbarcustomise")
        }
    }

    NotationToolBarModel {
        id: toolModel
    }

    Component.onCompleted: {
        toolModel.load()
    }
}
