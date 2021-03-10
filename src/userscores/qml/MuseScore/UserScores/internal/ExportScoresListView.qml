import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Rectangle {
    property alias scoresModel: listView.model

    color: ui.theme.backgroundSecondaryColor
    border.width: 1
    border.color: ui.theme.strokeColor

    ListView {
        id: listView
        anchors.fill: parent

        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ScrollBar.vertical: StyledScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }

        delegate: ListItemBlank {
            CheckBox {
                id: scoreCheckBox
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 7

                checked: model.isSelected
                onClicked: {
                    scoresModel.toggleSelection(model.index)
                }
            }

            StyledTextLabel {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: scoreCheckBox.right
                anchors.leftMargin: 14
                anchors.right: parent.right
                anchors.rightMargin: 7

                text: model.isMain ? qsTrc("userscores", "Main Score") : model.title
                font: model.isMain ? ui.theme.bodyBoldFont : ui.theme.bodyFont
                horizontalAlignment: Qt.AlignLeft
            }

            onClicked: {
                scoresModel.toggleSelection(model.index)
            }
        }
    }
}
