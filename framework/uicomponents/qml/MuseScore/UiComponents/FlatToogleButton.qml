import QtQuick 2.8
import MuseScore.UiComponents 1.0

FocusableItem {
    id: root

    property alias icon: buttonIcon.iconCode
    property bool checked: false
    property alias backgroundColor: backgroundRect.color

    signal toggled

    implicitHeight: 30
    implicitWidth: 30

    opacity: root.enabled ? 1.0 : 0.3

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: ui.theme.buttonColor
        opacity: ui.theme.buttonOpacityNormal
        border.width: 0

        radius: 2
    }

    StyledIconLabel {
        id: buttonIcon

        anchors.fill: parent
    }

    MouseArea {
        id: clickableArea

        anchors.fill: parent

        hoverEnabled: true

        onReleased: {
            root.toggled()
        }
    }

    states: [
        State {
            name: "PRESSED"
            when: clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHit
                border.width: 0
            }
        },

        State {
            name: "SELECTED"
            when: root.checked && !clickableArea.hovered

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityNormal
                border.width: 0
            }
        },

        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !root.checked && !clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
                border.color: ui.theme.strokeColor
                border.width: 1
            }
        },

        State {
            name: "SELECTED_HOVERED"
            when: clickableArea.containsMouse && root.checked

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHover
                border.width: 0
            }
        }
    ]
}
