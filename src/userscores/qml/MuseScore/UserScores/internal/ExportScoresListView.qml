import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Rectangle {
    id: background
    property alias scoresModel: listView.model

    color: ui.theme.textFieldColor
    border.width: 1
    border.color: ui.theme.strokeColor

    ListView {
        id: listView
        anchors.fill: parent
        anchors.margins: background.border.width

        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ScrollBar.vertical: StyledScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }

        delegate: ListItemBlank {
            hoveredStateColor: "transparent"
            pressedStateColor: "transparent"

            CheckBox {
                anchors.margins: 4
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right

                text: model.isMain ? qsTrc("userscores", "Main Score") : model.title
                font: model.isMain ? ui.theme.bodyBoldFont : ui.theme.bodyFont

                checked: model.isSelected
                onClicked: {
                    scoresModel.toggleSelected(model.index)
                }
            }
        }
    }
}
