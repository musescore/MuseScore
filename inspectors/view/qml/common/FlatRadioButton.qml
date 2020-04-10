import QtQuick 2.7
import QtQuick.Controls 2.0

RadioDelegate {
    id: root

    default property Component contentComponent

    implicitHeight: 30
    implicitWidth: (ListView.view.width - (ListView.view.spacing * (ListView.view.count - 1))) / ListView.view.count

    background: Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: "#CECECE"

        radius: 2
    }

    contentItem: Item {
        anchors.fill: parent

        Rectangle {
            id: selectionBackground

            anchors.fill: parent

            color: "transparent"

            border.color: "#A2A2A2"
            border.width: 1

            radius: 2
        }

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
                target: selectionBackground
                color: Qt.rgba(globalStyle.highlight.r, globalStyle.highlight.g, globalStyle.highlight.b, 0.75)
                border.color: globalStyle.highlight
            }
        },

        State {
            name: "SELECTED"
            when: root.checked

            PropertyChanges {
                target: selectionBackground
                color: Qt.rgba(globalStyle.highlight.r, globalStyle.highlight.g, globalStyle.highlight.b, 0.5)
                border.color: globalStyle.highlight
            }
        },

        State {
            name: "HOVERED"
            when: root.hovered && !root.checked && !root.pressed

            PropertyChanges {
                target: selectionBackground
                color: Qt.rgba(globalStyle.highlight.r, globalStyle.highlight.g, globalStyle.highlight.b, 0.2)
            }
        }
    ]
}
