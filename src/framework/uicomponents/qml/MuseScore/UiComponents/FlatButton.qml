import QtQuick 2.8
import MuseScore.UiComponents 1.0

FocusableItem {
    id: root

    property alias icon: buttonIcon.iconCode
    property alias text: textLabel.text
    property int iconPixelSize: buttonIcon.isEmpty ? 0 : ui.theme.iconsFont.pixelSize

    property int orientation: Qt.Vertical

    property bool accentButton: false

    QtObject {
        id: privateProperties

        property color defaultColor: accentButton ? ui.theme.accentColor : ui.theme.buttonColor
        property bool isVertical: orientation === Qt.Vertical
    }

    property color normalStateColor: privateProperties.defaultColor
    property color hoveredStateColor: privateProperties.defaultColor
    property color pressedStateColor: privateProperties.defaultColor

    property alias hovered: clickableArea.containsMouse

    signal clicked

    height: contentWrapper.height + 14
    width: (Boolean(text) ? Math.max(contentWrapper.width + 32, privateProperties.isVertical ? 132 : 0) : contentWrapper.width + 16)

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: normalStateColor
        opacity: ui.theme.buttonOpacityNormal
        border.width: 0
        radius: 3
    }

    Item {
        id: contentWrapper

        property int spacing: Boolean(!buttonIcon.isEmpty) && Boolean(textLabel.text) ? 4 : 0

        anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

        height: !privateProperties.isVertical ? Math.max(buttonIcon.height, textLabel.height) : buttonIcon.height + textLabel.height + spacing
        width: privateProperties.isVertical ? Math.max(textLabel.width, buttonIcon.width) : buttonIcon.width + textLabel.width + spacing

        StyledIconLabel {
            id: buttonIcon

            font.pixelSize: root.iconPixelSize
        }

        StyledTextLabel {
            id: textLabel

            height: text === "" ? 0 : implicitHeight
            horizontalAlignment: Text.AlignHCenter
        }

        states: [
            State {
                when: !privateProperties.isVertical
                AnchorChanges {
                    target: buttonIcon
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                }
                AnchorChanges {
                    target: textLabel
                    anchors.left: buttonIcon.right
                    anchors.verticalCenter: parent.verticalCenter
                }
                PropertyChanges {
                    target: textLabel
                    anchors.leftMargin: contentWrapper.spacing
                }
            },
            State {
                when: privateProperties.isVertical
                AnchorChanges {
                    target: buttonIcon
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                AnchorChanges {
                    target: textLabel
                    anchors.top: buttonIcon.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                PropertyChanges {
                    target: textLabel
                    anchors.leftMargin: contentWrapper.spacing
                }
            }
        ]
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
                color: pressedStateColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                color: hoveredStateColor
                opacity: ui.theme.buttonOpacityHover
            }
        }
    ]
}
