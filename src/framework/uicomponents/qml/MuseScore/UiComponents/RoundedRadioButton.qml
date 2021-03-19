import QtQuick 2.7
import QtQuick.Controls 2.0

RadioButton {
    id: root

    default property Component contentComponent

    implicitHeight: 20
    implicitWidth: ListView.view ? (ListView.view.width - (ListView.view.spacing * (ListView.view.count - 1))) / ListView.view.count
                                 : 30

    spacing: 30

    hoverEnabled: true

    contentItem: Item {
        anchors.fill: parent
        anchors.leftMargin: root.indicator.width + root.spacing + root.leftPadding

        Loader {
            id: contentLoader

            anchors.fill: parent

            sourceComponent: contentComponent
        }
    }

    indicator: Item {
        x: root.leftPadding
        y: Boolean(parent) ? parent.height / 2 - height / 2 : 0
        implicitWidth: 20
        implicitHeight: implicitWidth

        Rectangle {
            id: borderRect
            implicitWidth: 20
            implicitHeight: implicitWidth
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            radius: 13

            color: ui.theme.textFieldColor
            border.color: ui.theme.fontPrimaryColor
            opacity: ui.theme.buttonOpacityNormal
        }

        Rectangle {
            id: mainRect
            width: 10
            height: width
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            radius: 7

            color: ui.theme.accentColor
            visible: root.checked
        }
    }

    states: [
        State {
            name: "PRESSED"
            when: root.pressed

            PropertyChanges {
                target: borderRect
                opacity: ui.theme.accentOpacityHit
            }

            PropertyChanges {
                target: mainRect
                visible: true
            }
        },

        State {
            name: "SELECTED"
            when: root.checked && !root.hovered

            PropertyChanges {
                target: borderRect
                opacity: ui.theme.accentOpacityNormal
            }

            PropertyChanges {
                target: mainRect
                visible: true
            }
        },

        State {
            name: "HOVERED"
            when: root.hovered && !root.checked && !root.pressed

            PropertyChanges {
                target: borderRect
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "SELECTED_HOVERED"
            when: root.hovered && root.checked

            PropertyChanges {
                target: borderRect
                opacity: ui.theme.accentOpacityHover
            }

            PropertyChanges {
                target: mainRect
                visible: true
            }
        }
    ]
}
