import QtQuick 2.15
import QtQuick.Dialogs 1.2

import MuseScore.Ui 1.0

Rectangle {
    id: root

    property bool isIndeterminate: false

    signal newColorSelected(var newColor)

    height: 26
    width: parent.width

    opacity: enabled ? 1 : ui.theme.itemOpacityDisabled

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

    StyledIconLabel {
        anchors.fill: parent

        iconCode: IconCode.QUESTION_MARK

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

            PropertyChanges { target: backgroundRect; border.color: ui.theme.buttonColor }
        },

        State {
            name: "HOVERED"
            when: cliickableArea.containsMouse && !cliickableArea.pressed && !colorDialog.visible

            PropertyChanges { target: backgroundRect; border.color: ui.theme.accentColor }
        },

        State {
            name: "PRESSED"
            when: cliickableArea.pressed || colorDialog.visible

            PropertyChanges { target: backgroundRect; border.color: ui.theme.fontPrimaryColor }
        }
    ]
}

