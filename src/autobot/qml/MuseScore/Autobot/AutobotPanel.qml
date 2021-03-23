import QtQuick 2.15
import MuseScore.UiComponents 1.0
import MuseScore.Autobot 1.0

Rectangle {

    color: ui.theme.backgroundPrimaryColor

    AutobotModel {
        id: autobot
    }

    StyledComboBox {
        id: selectTestCase
        anchors.left: parent.left
        anchors.right: parent.right
        textRoleName: "name"
        valueRoleName: "name"

        currentIndex: 0
        model: autobot.testCases
    }

    Row {
        id: runPanel
        anchors.top: selectTestCase.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        height: 32
        spacing: 8

        FlatButton {
            width: 64
            text: "Run All"
            onClicked: autobot.runAll(selectTestCase.currentValue)
        }

        FlatButton {
            width: 64
            text: "Stop"
            onClicked: autobot.stop()
        }

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter
            horizontalAlignment: Text.AlignLeft
            text: "Status: " + autobot.status
        }
    }

    ListView {
        anchors.top: runPanel.bottom
        anchors.bottom: statusPanel.top
        anchors.left: parent.left
        anchors.right:  parent.right
        clip: true

        model: autobot.files

        delegate: ListItemBlank {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 32
            isSelected: isCurrentFile

            StyledTextLabel {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                horizontalAlignment: Text.AlignLeft
                text: fileTitle
            }

            onClicked: {
                autobot.runFile(selectTestCase.currentValue, fileIndex)
            }
        }
    }

    Item {
        id: statusPanel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 32

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 2
            color: ui.theme.strokeColor
        }

        StyledTextLabel {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            horizontalAlignment: Text.AlignLeft
            text: "" // reserved
        }
    }
}
