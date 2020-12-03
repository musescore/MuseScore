import QtQuick 2.9
import QtQuick.Controls 2.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    height: parent ? parent.height : implicitHeight
    width: parent ? parent.width : implicitWidth

    property bool isSelected: Boolean(selectedRole) ? selectedRole : false

    signal clicked()

    StyledTextLabel {
        anchors.centerIn: parent
        text: Boolean(itemRole) ? itemRole.title : ""
    }

    Rectangle {
        id: background

        anchors.fill: parent

        z: -1

        color: "transparent"
        opacity: 1

        states: [
            State {
                name: "HOVERED"
                when: mouseArea.containsMouse && !mouseArea.pressed && !root.isSelected

                PropertyChanges {
                    target: background
                    opacity: ui.theme.buttonOpacityHover
                    color: ui.theme.buttonColor
                }
            },

            State {
                name: "PRESSED"
                when: mouseArea.pressed && !root.isSelected

                PropertyChanges {
                    target: background
                    opacity: ui.theme.buttonOpacityHit
                    color: ui.theme.buttonColor
                }
            },

            State {
                name: "SELECTED"
                when: root.isSelected

                PropertyChanges {
                    target: background
                    opacity: ui.theme.accentOpacityHit
                    color: ui.theme.accentColor
                }
            }
        ]

        MouseArea {
            id: mouseArea

            anchors.fill: parent

            hoverEnabled: true

            onClicked: {
                root.clicked()
            }
        }
    }
}
