import QtQuick 2.8

FocusableItem {
    id: root

    property alias icon: buttonIcon.iconCode
    property alias text: textLabel.text
    property int iconPixelSize: buttonIcon.isEmpty ? 0 : 16

    signal clicked

    height: contentWrapper.implicitHeight + 16
    width: parent.width

    opacity: root.enabled ? 1.0 : 0.3

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: Qt.rgba(globalStyle.button.r, globalStyle.button.g, globalStyle.button.b, 0.5)

        radius: 2
    }

    Column {
        id: contentWrapper

        anchors.verticalCenter: parent.verticalCenter

        height: implicitHeight
        width: parent.width

        spacing: 4

        StyledIconLabel {
            id: buttonIcon

            anchors.horizontalCenter: parent.horizontalCenter
        }

        StyledTextLabel {
            id: textLabel

            anchors.horizontalCenter: parent.horizontalCenter
            height: implicitHeight
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
