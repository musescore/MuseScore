import QtQuick 2.15
import QtQuick.Dialogs 1.2

import MuseScore.Ui 1.0

Rectangle {
    id: root

    property bool isIndeterminate: false

    signal newColorSelected(var newColor)

    height: 30
    width: parent.width

    opacity: enabled ? 1 : ui.theme.itemOpacityDisabled

    radius: 3
    color: "#000000"

    border.width: 1

    StyledIconLabel {
        anchors.fill: parent

        iconCode: IconCode.QUESTION_MARK

        visible: isIndeterminate
    }

    MouseArea {
        id: clickableArea

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
            when: !clickableArea.containsMouse && !colorDialog.visible

            PropertyChanges { target: root; border.color: ui.theme.buttonColor }
        },

        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed && !colorDialog.visible

            PropertyChanges { target: root; border.color: ui.theme.accentColor }
        },

        State {
            name: "PRESSED"
            when: clickableArea.pressed || colorDialog.visible

            PropertyChanges { target: root; border.color: ui.theme.fontPrimaryColor }
        }
    ]
}

