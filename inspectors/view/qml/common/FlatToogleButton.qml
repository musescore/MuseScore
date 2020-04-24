import QtQuick 2.8

FocusableItem {
    id: root

    property alias icon: buttonIcon.iconCode
    property int iconPixelSize: 16
    property bool pressed: false

    signal toggled

    implicitHeight: 20
    implicitWidth: 20

    opacity: root.enabled ? 1.0 : 0.3

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

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
            when: root.pressed

            PropertyChanges {
                target: backgroundRect
                color: Qt.rgba(globalStyle.highlight.r, globalStyle.highlight.g, globalStyle.highlight.b, 0.3)
                border.color: globalStyle.highlight
                border.width: 2
            }
        },

        State {
            name: "NORMAL"
            when: !root.pressed

            PropertyChanges {
                target: backgroundRect
                color: Qt.rgba(globalStyle.button.r, globalStyle.button.g, globalStyle.button.b, 0.5)
                border.color: globalStyle.highlight
                border.width: 0
            }
        }
    ]
}
