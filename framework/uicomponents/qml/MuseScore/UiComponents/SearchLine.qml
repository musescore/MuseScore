import QtQuick 2.0
import QtQuick.Controls 2.15
import MuseScore.Ui 1.0

Rectangle {
    id: root

    property alias text: textField.text

    width: 184
    height: 30

    color: ui.theme.backgroundColor
    border.color: ui.theme.strokeColor
    border.width: 1
    radius: 3

    Row {
        anchors.fill: parent

        Item {
            width: 32
            height: 30

            StyledIconLabel {
                id: searchIcon

                anchors.centerIn: parent
                iconCode: IconCode.SEARCH
                color: ui.theme.strokeColor
            }
        }

        TextField {
            id: textField

            anchors.verticalCenter: parent.verticalCenter
            width: 134

            placeholderText: qsTrc("uicomponents", "Search")
            color: ui.theme.strokeColor

            font.italic: true
            font.family: ui.theme.font.family
            font.pointSize: ui.theme.font.pointSize

            background: Rectangle {
                color: ui.theme.backgroundColor
            }

            selectByMouse: true
        } 
    }

    states: [
        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !textField.focus

            PropertyChanges {
                target: root
                border.color: ui.theme.buttonColor
            }

            PropertyChanges {
                target: searchIcon
                color: ui.theme.buttonColor
            }

            PropertyChanges {
                target: textField
                color: ui.theme.buttonColor
            }
        },

        State {
            name: "ACTIVE"
            when: textField.focus

            PropertyChanges {
                target: root
                border.color: ui.theme.accentColor
            }

            PropertyChanges {
                target: searchIcon
                color: ui.theme.fontColor
            }

            PropertyChanges {
                target: textField
                color: ui.theme.fontColor
            }
        }
    ]

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        propagateComposedEvents: true
        hoverEnabled: true

        onPressed: {
            textField.forceActiveFocus()
            mouse.accepted = false
        }
    }
}
