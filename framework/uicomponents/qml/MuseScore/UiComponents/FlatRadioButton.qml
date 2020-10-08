import QtQuick 2.7
import QtQuick.Controls 2.0

RadioDelegate {
    id: root

    default property Component contentComponent

    property alias radius: backgroundRect.radius

    property color normalStateColor: ui.theme.buttonColor
    property color hoverStateColor: ui.theme.buttonColor
    property color pressedStateColor: ui.theme.buttonColor
    property color selectedStateColor: ui.theme.accentColor

    implicitHeight: 30
    implicitWidth: ListView.view ? (ListView.view.width - (ListView.view.spacing * (ListView.view.count - 1))) / ListView.view.count
                                 : 30
    hoverEnabled: true

    background: Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: normalStateColor
        opacity: ui.theme.buttonOpacityNormal

        radius: 2
    }

    contentItem: Item {
        anchors.fill: parent

        Loader {
            id: contentLoader

            anchors.fill: parent

            sourceComponent: contentComponent
        }
    }

    indicator: Item {}

    states: [
        State {
            name: "HOVERED"
            when: root.hovered && !root.checked && !root.pressed

            PropertyChanges {
                target: backgroundRect
                color: hoverStateColor
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: root.pressed && !root.checked

            PropertyChanges {
                target: backgroundRect
                color: pressedStateColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "SELECTED"
            when: root.checked

            PropertyChanges {
                target: backgroundRect
                color: selectedStateColor
                opacity: ui.theme.buttonOpacityNormal
            }
        }
    ]
}
