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

        color: ui.theme.button

        radius: 2
    }

    Rectangle {
        id: selectionOverlay

        anchors.fill: parent

        color: "#00000000"
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
                target: selectionOverlay
                color: Qt.darker(ui.theme.button, 1.1)
                border.width: 0
            }
        },

        State {
            name: "SELECTED"
            when: root.checked && !clickableArea.hovered

            PropertyChanges {
                target: selectionOverlay
                color: Qt.rgba(ui.theme.highlight.r, ui.theme.highlight.g, ui.theme.highlight.b, 0.5)
                border.width: 0
            }
        },

        State {
            name: "HOVERED"
            when: clickableArea.hovered && !root.checked && !clickableArea.pressed

            PropertyChanges {
                target: selectionOverlay
                color: "#00000000"
                border.color: "#25000000"
                border.width: 1
            }
        },

        State {
            name: "SELECTED_HOVERED"
            when: clickableArea.hovered && root.checked

            PropertyChanges {
                target: selectionOverlay
                color: Qt.rgba(ui.theme.highlight.r, ui.theme.highlight.g, ui.theme.highlight.b, 0.75)
                border.width: 0
            }
        }
    ]
}
