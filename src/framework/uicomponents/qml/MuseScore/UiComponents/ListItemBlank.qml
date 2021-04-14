import QtQuick 2.15

import MuseScore.Ui 1.0

FocusableControl {
    id: root

    property string hint

    property bool isSelected: false
    property alias radius: root.background.radius

    property color normalStateColor: "transparent"
    property color hoveredStateColor: privateProperties.defaultColor
    property color pressedStateColor: privateProperties.defaultColor

    signal clicked()
    signal doubleClicked()
    signal hovered(var isHovered, var mouseX, int mouseY)

    implicitHeight: 30
    implicitWidth: Boolean(ListView.view) ? ListView.view.width : 30

    Accessible.selectable: true
    Accessible.selected: isSelected

    background.color: normalStateColor

    mouseArea.hoverEnabled: root.visible
    mouseArea.onHoveredChanged: root.hovered(mouseArea.containsMouse, mouseArea.mouseX, mouseArea.mouseY)

    mouseArea.onClicked: root.clicked()
    mouseArea.onDoubleClicked: root.doubleClicked()

    mouseArea.onContainsMouseChanged: {
        if (!Boolean(root.hint)) {
            return
        }

        if (mouseArea.containsMouse) {
            ui.tooltip.show(this, root.hint)
        } else {
            ui.tooltip.hide(this)
        }
    }

    QtObject {
        id: privateProperties

        property color defaultColor: ui.theme.buttonColor
    }

    background.opacity: root.enabled ? 1 : ui.theme.itemOpacityDisabled

    states: [
        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed && !root.isSelected

            PropertyChanges {
                target: root.background
                opacity: ui.theme.buttonOpacityHover
                color: hoveredStateColor
            }
        },

        State {
            name: "PRESSED"
            when: mouseArea.pressed && !root.isSelected

            PropertyChanges {
                target: root.background
                opacity: ui.theme.buttonOpacityHit
                color: pressedStateColor
            }
        },

        State {
            name: "SELECTED"
            when: root.isSelected

            PropertyChanges {
                target: root.background
                opacity: ui.theme.accentOpacityHit
                color: ui.theme.accentColor
            }
        }
    ]
}
