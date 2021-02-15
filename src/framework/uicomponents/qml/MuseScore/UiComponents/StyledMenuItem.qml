import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0

MenuItem {
    id: root

    implicitHeight: 30
    implicitWidth: 220

    property var hintIcon: checkable && checked ? IconCode.TICK : IconCode.NONE
    property string shortcut: ""

    background: Rectangle {
        id: background

        anchors.fill: parent

        color: "transparent"
        opacity: 1

        states: [
            State {
                name: "HOVERED"
                when: root.hovered && !root.pressed

                PropertyChanges {
                    target: background
                    color: ui.theme.accentColor
                    opacity: ui.theme.buttonOpacityHover
                }
            },

            State {
                name: "PRESSED"
                when: root.pressed

                PropertyChanges {
                    target: background
                    color: ui.theme.accentColor
                    opacity: ui.theme.buttonOpacityHit
                }
            }
        ]
    }

    indicator: Item {}

    contentItem: RowLayout {
        id: contentRow

        implicitHeight: root.implicitHeight
        implicitWidth: root.implicitWidth

        spacing: 0

        Item {
            readonly property int size: 16

            Layout.preferredWidth: size
            Layout.preferredHeight: size
            Layout.leftMargin: 6
            Layout.rightMargin: 10

            StyledIconLabel {
                iconCode: root.hintIcon
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.fillHeight: true

            text: root.text
            horizontalAlignment: Text.AlignLeft
        }

        StyledTextLabel {
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: 14

            text: root.shortcut

            visible: Boolean(text)
        }
    }
}
