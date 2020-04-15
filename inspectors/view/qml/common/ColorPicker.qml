import QtQuick 2.9
import QtQuick.Dialogs 1.2


Rectangle {
    id: root

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
        border.color: globalStyle.button
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
            root.color = colorDialog.color
        }
    }

    states: [
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

