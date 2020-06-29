import QtQuick 2.0
import MuseScore.Ui 1.0

FocusableItem {
    id: root

    property bool checked: false
    property bool isIndeterminate: false

    property alias text: label.text

    signal clicked

    implicitHeight: contentRow.height
    width: parent.width

    opacity: root.enabled ? 1.0 : 0.3

    Item {
        id: contentRow

        height: Math.max(box.height, label.implicitHeight)
        width: parent.width

        Rectangle {
            id: box

            height: 20
            width: 20

            border.width: 1
            border.color: "#00000000"

            radius: 2
            color: ui.theme.buttonColor
            opacity: ui.theme.buttonOpacityNormal

            StyledIconLabel {
                anchors.fill: parent
                iconCode: root.isIndeterminate ? IconCode.MINUS : IconCode.TICK_RIGHT_ANGLE

                visible: root.checked || root.isIndeterminate
            }
        }

        StyledTextLabel {
            id: label

            anchors.verticalCenter: box.verticalCenter
            anchors.left: box.right
            anchors.leftMargin: 8
            anchors.right: parent.right

            horizontalAlignment: Text.AlignLeft
        }
    }

    MouseArea {
        id: clickableArea

        anchors.fill: contentRow
        anchors.margins: -4

        hoverEnabled: true

        onClicked: { root.clicked() }
    }

    states: [
        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed

            PropertyChanges {
                target: box
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
                border.color: ui.theme.strokeColor
                border.width: 1
            }
        },

        State {
            name: "PRESSED"
            when: clickableArea.containsMouse && clickableArea.pressed

            PropertyChanges {
                target: box
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHit
                border.color: ui.theme.strokeColor
            }
        }
    ]
}
