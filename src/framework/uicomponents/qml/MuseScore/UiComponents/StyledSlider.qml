import QtQuick 2.12
import QtQuick.Controls 2.12

import MuseScore.Ui 1.0

Slider {
    id: root

    implicitWidth: vertical ? 4 : 220
    implicitHeight: vertical ? 220 : 4

    hoverEnabled: true

    background: Item {
        id: mainBackground

        anchors.fill: parent

        Rectangle {
            id: filledBackground

            width: handleBackground.x + handleBackground.width / 2
            height: mainBackground.height

            opacity: 1
            color: ui.theme.accentColor
            radius: 4
        }

        Rectangle {
            id: blankBackground

            height: mainBackground.height
            width: mainBackground.width - filledBackground.width
            x: filledBackground.width

            opacity: 0.5
            color: ui.theme.fontPrimaryColor
            radius: filledBackground.radius
        }
    }

    handle: Rectangle {
        id: handleBackground

        x: root.leftPadding + root.visualPosition * (root.availableWidth - width)
        y: root.topPadding + root.availableHeight / 2 - height / 2

        width: 14
        height: width
        radius: width / 2

        color: ui.theme.textFieldColor

        Rectangle {
            id: handleBorder

            anchors.fill: parent

            radius: width / 2
            opacity: 0.7
            color: "transparent"
            border.width: 1
            border.color: ui.theme.fontPrimaryColor

            states: [
                State {
                    name: "HOVERED"
                    when: root.hovered && !root.pressed

                    PropertyChanges {
                        target: handleBorder
                        opacity: 0.5
                    }
                },

                State {
                    name: "PRESSED"
                    when: root.pressed

                    PropertyChanges {
                        target: handleBorder
                        opacity: 1
                    }
                }
            ]
        }
    }

    states: [
        State {
            name: "VERTICAL"
            when: root.vertical

            PropertyChanges {
                target: filledBackground

                y: handleBackground.y + handleBackground.height / 2

                width: mainBackground.width
                height: root.height - y
            }

            PropertyChanges {
                target: blankBackground

                x: mainBackground.x
                y: mainBackground.y

                width: mainBackground.width
                height: root.height - filledBackground.height
            }

            PropertyChanges {
                target: handleBackground

                x: root.topPadding + root.availableWidth / 2 - width / 2
                y: root.bottomPadding + root.visualPosition * (root.availableHeight - height)
            }
        }
    ]
}
