import QtQuick 2.8
import MuseScore.UiComponents 1.0

FocusableItem {
    id: root

    property alias icon: buttonIcon.iconCode
    property alias text: textLabel.text
    property int iconPixelSize: buttonIcon.isEmpty ? 0 : 16
    property alias backgroundColor: backgroundRect.color

    signal clicked

    height: contentWrapper.implicitHeight + 16
    width: contentWrapper.width + 16

    opacity: root.enabled ? 1.0 : 0.3

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: ui.theme.buttonColor
        opacity: ui.theme.buttonOpacityNormal
        border.width: 0
        radius: 3
    }

    Column {
        id: contentWrapper

        anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

        height: implicitHeight

        spacing: 4

        StyledIconLabel {
            id: buttonIcon

            anchors.horizontalCenter: parent.horizontalCenter
        }

        StyledTextLabel {
            id: textLabel

            anchors.horizontalCenter: parent.horizontalCenter
            height: text === "" ? 0 : implicitHeight

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
                opacity: ui.theme.buttonOpacityHit
                border.color: ui.theme.strokeColor
                border.width: 1
            }
        },

        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                opacity: ui.theme.buttonOpacityHover
                border.color: ui.theme.strokeColor
                border.width: 1
            }
        }
    ]
}
