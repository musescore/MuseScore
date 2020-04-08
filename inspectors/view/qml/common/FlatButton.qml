import QtQuick 2.8

FocusableItem {
    id: root

    property alias icon: buttonIcon.icon
    property alias text: textLabel.text
    property int iconPixelSize: 16

    signal clicked

    height: 48
    width: parent.width

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: Qt.rgba(globalStyle.button.r, globalStyle.button.g, globalStyle.button.b, 0.5)

        radius: 2
    }

    Column {
        id: contentWrapper

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width

        spacing: 4

        StyledIcon {
            id: buttonIcon

            anchors.horizontalCenter: parent.horizontalCenter

            sourceSize.height: root.iconPixelSize
            sourceSize.width: root.iconPixelSize
        }

        StyledTextLabel {
            id: textLabel

            width: parent.width

            horizontalAlignment: Text.AlignHCenter
        }
    }

    MouseArea {
        id: clickableArea

        anchors.fill: parent

        hoverEnabled: true

        onReleased: {
            root.clicked()
        }
    }

    states: [
        State {
            name: "PRESSED"
            when: clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                color: Qt.rgba(globalStyle.button.r, globalStyle.button.g, globalStyle.button.b, 1.0)
            }
        },

        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                color: Qt.rgba(globalStyle.button.r, globalStyle.button.g, globalStyle.button.b, 0.75)
            }
        }
    ]
}
