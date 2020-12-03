import QtQuick 2.9
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    height: parent ? parent.height : implicitHeight
    width: parent ? parent.width : implicitWidth

    property bool isSelected: Boolean(selectedRole) ? selectedRole : false

    signal clicked()

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 6
        anchors.rightMargin: 6

        spacing: 16

        CheckBox {
            Layout.alignment: Qt.AlignLeft

            checked: Boolean(itemRole) ? itemRole.checked : false

            onClicked: {
                itemRole.checked = !checked
            }
        }

        StyledIconLabel {
            Layout.alignment: Qt.AlignLeft

            width: 36
            height: width

            iconCode: Boolean(itemRole) ? itemRole.icon : IconCode.NONE
        }

        StyledTextLabel {
            Layout.fillWidth: true

            horizontalAlignment: Qt.AlignLeft
            text: Boolean(itemRole) ? itemRole.title : ""
        }

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
