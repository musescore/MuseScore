import QtQuick 2.7
import QtQuick.Controls 2.0

RadioDelegate {
    id: root

    default property Component contentComponent

    property alias backgroundColor: backgroundRect.color
    property alias radius: backgroundRect.radius

    implicitHeight: 30
    implicitWidth: ListView.view ? (ListView.view.width - (ListView.view.spacing * (ListView.view.count - 1))) / ListView.view.count
                                 : 30

    hoverEnabled: true

    background: Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: ui.theme.buttonColor
        opacity: root.checked || root.pressed ? 0 : ui.theme.buttonOpacityNormal

        border.width: 0
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

    indicator: Item {
    }

    states: [
        State {
            name: "PRESSED"
            when: root.pressed

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHit
                border.width: 0
            }
        },

        State {
            name: "SELECTED"
            when: root.checked && !root.hovered

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityNormal
                border.width: 0
            }
        },

        State {
            name: "HOVERED"
            when: root.hovered && !root.checked && !root.pressed

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
            when: root.hovered && root.checked

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHover
                border.width: 0
            }
        }
    ]
}
