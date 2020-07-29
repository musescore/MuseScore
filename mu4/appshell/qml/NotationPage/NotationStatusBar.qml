import QtQuick 2.7
import QtQuick.Controls 2.2
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    NotationStatusBarModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    StyledTextLabel {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 20

        text: model.accessibilityInfo
    }

    Row	{
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: 20

        spacing: 8

        StyledTextLabel {
            width: 40

            text: "-"
            font.bold: true

            MouseArea {
                anchors.fill: parent
                onClicked: model.zoomOut()
            }
        }

        StyledTextLabel {
            width: 60
            text: model.currentZoom + " %"
        }

        StyledTextLabel {
            width: 40

            text: "+"
            font.bold: true

            MouseArea {
                anchors.fill: parent
                onClicked: model.zoomIn()
            }
        }
    }
}
