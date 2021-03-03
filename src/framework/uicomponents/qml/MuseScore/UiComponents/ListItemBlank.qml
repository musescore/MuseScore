import QtQuick 2.15

import MuseScore.Ui 1.0

Item {
    id: root

    property string hint

    property bool isSelected: false
    property alias radius: background.radius

    property color defaultColor: ui.theme.buttonColor

    signal clicked()
    signal doubleClicked()
    signal hovered(var isHovered, var mouseX, int mouseY)

    implicitHeight: 30
    implicitWidth: Boolean(ListView.view) ? ListView.view.width : 30

    Accessible.selectable: true
    Accessible.selected: isSelected

    Rectangle {
        id: background

        anchors.fill: parent

        color: "transparent"
        opacity: root.enabled ? 1 : ui.theme.itemOpacityDisabled
    }

    states: [
        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed && !root.isSelected

            PropertyChanges {
                target: background
                opacity: ui.theme.buttonOpacityHover
                color: defaultColor
            }
        },

        State {
            name: "PRESSED"
            when: mouseArea.pressed && !root.isSelected

            PropertyChanges {
                target: background
                opacity: ui.theme.buttonOpacityHit
                color: defaultColor
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

        hoverEnabled: root.visible

        onHoveredChanged: {
            root.hovered(mouseArea.containsMouse, mouseX, mouseY)
        }

        onClicked: {
            root.clicked()
        }

        onDoubleClicked: {
            root.doubleClicked()
        }

        onContainsMouseChanged: {
            if (!Boolean(root.hint)) {
                return
            }

            if (containsMouse) {
                ui.tooltip.show(this, root.hint)
            } else {
                ui.tooltip.hide(this)
            }
        }
    }
}
