import QtQuick 2.5

import MuseScore.Ui 1.0

Item {
    id: root

    property string hint

    property bool isSelected: false
    property alias radius: background.radius

    signal clicked()
    signal doubleClicked()
    signal hovered()

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

        hoverEnabled: root.visible

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
