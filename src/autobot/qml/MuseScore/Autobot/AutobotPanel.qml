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

        model: autobot.testCases
        onValueChanged: autobot.setCurrentTestCase(value)
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
            onClicked: autobot.runAllFiles()
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
            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            height: 48
            isSelected: isCurrentFile

            StyledTextLabel {
                id: titleLabel
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignLeft
                text: fileTitle
            }

            StyledTextLabel {
                anchors.top: titleLabel.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: 2
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                horizontalAlignment: Text.AlignLeft
                text: fileStatus
                color: {
                    switch (fileStatus) {
                    case "none":    return "#666666"
                    case "success": return "#009900"
                    case "failed":  return "#cc0000"
                    }
                    return "#666666"
                }
            }

            onClicked: {
                autobot.runFile(fileIndex)
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
