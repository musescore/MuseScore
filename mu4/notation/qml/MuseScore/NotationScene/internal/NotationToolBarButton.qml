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

        color: ui.theme.backgroundPrimaryColor
        opacity: ui.theme.buttonOpacityNormal
    }

    Row {
        id: contentWrapper

        anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

        width: implicitWidth

        spacing: 8

        StyledIconLabel {
            id: buttonIcon

            anchors.verticalCenter: parent.verticalCenter
        }

        StyledTextLabel {
            id: textLabel

            anchors.verticalCenter: parent.verticalCenter
            width: text === "" ? 0 : implicitWidth

            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 15
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
                color: ui.theme.accentColor
            }
        },

        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                opacity: ui.theme.buttonOpacityHover
            }
        }
    ]
}
