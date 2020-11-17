import QtQuick 2.12
import QtQuick.Controls 2.12

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    property alias orientation: gridView.orientation

    QtObject {
        id: privatesProperties

        property bool isHorizontal: orientation === Qt.Horizontal
    }

    GridViewSectional {
        id: gridView
        anchors.fill: parent

        sectionRole: "sectionRole"

        cellWidth: 36
        cellHeight: cellWidth

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
        id: customizeButton

        anchors.margins: 8

        icon: IconCode.CONFIGURE
        normalStateColor: "transparent"

        onClicked: {
            api.launcher.open("musescore://notation/noteinputbar/customise")
        }
    }

    NotationToolBarModel {
        id: toolModel
    }

    Component.onCompleted: {
        toolModel.load()
    }

    states: [
        State {
            when: privatesProperties.isHorizontal
            PropertyChanges {
                target: gridView
                sectionWidth: 2
                sectionHeight: root.height
                rows: 1
                columns: gridView.noLimit
            }

            AnchorChanges {
                target: customizeButton
                anchors.right: root.right
                anchors.verticalCenter: root.verticalCenter
            }
        },
        State {
            when: !privatesProperties.isHorizontal
            PropertyChanges {
                target: gridView
                sectionWidth: root.width
                sectionHeight: 2
                rows: gridView.noLimit
                columns: 2
            }

            AnchorChanges {
                target: customizeButton
                anchors.bottom: root.bottom
                anchors.right: root.right
            }
        }
    ]
}
