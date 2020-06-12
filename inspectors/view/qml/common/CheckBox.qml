import QtQuick 2.0
import MuseScore.Inspectors 3.3

FocusableItem {
    id: root

    property bool checked: false
    property bool isIndeterminate: false

    property alias text: label.text

    signal clicked

    implicitHeight: contentRow.height
    width: parent.width

    opacity: root.enabled ? 1.0 : 0.3

    Behavior on opacity {
        NumberAnimation { duration: 100; }
    }

    Item {
        id: contentRow

        height: Math.max(box.height, label.implicitHeight)
        width: parent.width

        Rectangle {
            id: box

            height: 20
            width: 20

            border.width: 1

            radius: 2
            color: Qt.rgba(globalStyle.button.r, globalStyle.button.g, globalStyle.button.b, 0.75)
            border.color: "#A2A2A2"

            StyledIconLabel {
                anchors.fill: parent
                iconCode: root.isIndeterminate ? IconNameTypes.MINUS : IconNameTypes.TICK_RIGHT_ANGLE

                visible: root.checked || root.isIndeterminate
            }

            Behavior on border.color {
                ColorAnimation { duration: 100; }
            }
            Behavior on color {
                ColorAnimation { duration: 100; }
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
                radius: 1
                color: Qt.rgba(globalStyle.button.r, globalStyle.button.g, globalStyle.button.b, 1.0)
                border.color: globalStyle.highlight
            }
        },

        State {
            name: "PRESSED"
            when: clickableArea.containsMouse && clickableArea.pressed

            PropertyChanges {
                target: box
                radius: 1
                color: "#C0C0C0"
                border.color: globalStyle.highlight
            }
        }
    ]
}
