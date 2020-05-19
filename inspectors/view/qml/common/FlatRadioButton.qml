import QtQuick 2.7
import QtQuick.Controls 2.0

RadioDelegate {
    id: root

    default property Component contentComponent

    property alias backgroundColor: backgroundRect.color

    implicitHeight: 30
    implicitWidth: ListView.view ? (ListView.view.width - (ListView.view.spacing * (ListView.view.count - 1))) / ListView.view.count
                                 : 30

    hoverEnabled: true

    background: Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: globalStyle.button

        radius: 2
    }

    contentItem: Item {
        anchors.fill: parent

        Rectangle {
            id: selectionOverlay

            anchors.fill: parent

            color: "#00000000"
            border.width: 0
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
                target: selectionOverlay
                color: Qt.darker(globalStyle.button, 1.1)
                border.width: 0
            }
        },

        State {
            name: "SELECTED"
            when: root.checked && !root.hovered

            PropertyChanges {
                target: selectionOverlay
                color: Qt.rgba(globalStyle.highlight.r, globalStyle.highlight.g, globalStyle.highlight.b, 0.5)
                border.width: 0
            }
        },

        State {
            name: "HOVERED"
            when: root.hovered && !root.checked && !root.pressed

            PropertyChanges {
                target: selectionOverlay
                color: "#00000000"
                border.color: "#25000000"
                border.width: 1
            }
        },

        State {
            name: "SELECTED_HOVERED"
            when: root.hovered && root.checked

            PropertyChanges {
                target: selectionOverlay
                color: Qt.rgba(globalStyle.highlight.r, globalStyle.highlight.g, globalStyle.highlight.b, 0.75)
                border.width: 0
            }
        }
    ]
}
