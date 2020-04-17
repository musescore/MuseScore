import QtQuick 2.9
import QtQuick.Dialogs 1.2


Rectangle {
    id: root

    property bool isIndeterminate: false

    signal newColorSelected(var newColor)

    height: 26
    width: parent.width
    radius: 2
    color: "#000000"

    Rectangle {
        id: backgroundRect

        anchors.fill: parent
        anchors.margins: -2

        radius: 2
        color: "transparent"
        border.width: 1
    }

    StyledIcon {
        anchors.centerIn: parent

        icon: "qrc:/resources/icons/question_mark.svg"

        pixelSize: 12

        sourceSize.height: 12
        sourceSize.width: 12

        visible: isIndeterminate
    }

    MouseArea {
        id: cliickableArea

        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            colorDialog.open()
        }
    }

    ColorDialog {
        id: colorDialog

        currentColor: root.color
        modality: Qt.ApplicationModal

        onAccepted: {
            root.newColorSelected(colorDialog.color)
        }
    }

    states: [
        State {
            name: "NORMAL"
            when: !cliickableArea.containsMouse && !colorDialog.visible

            PropertyChanges { target: backgroundRect; border.color: globalStyle.button }
        },

        State {
            name: "HOVERED"
            when: cliickableArea.containsMouse && !cliickableArea.pressed && !colorDialog.visible

            PropertyChanges { target: backgroundRect; border.color: globalStyle.highlight }
        },

        State {
            name: "PRESSED"
            when: cliickableArea.pressed || colorDialog.visible

            PropertyChanges { target: backgroundRect; border.color: "#000000" }
        }
    ]
}

