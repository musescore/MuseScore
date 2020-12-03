import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.UiComponents 1.0

TabButton {
    id: root

    property int sideMargin: 0
    property bool isCurrent: false
    property string backgroundColor: ui.theme.backgroundPrimaryColor

    width: implicitWidth + sideMargin * 2 - 8

    contentItem: StyledTextLabel {
        text: root.text
        font.bold: true
        opacity: 0.75
    }

    background: Rectangle {
        implicitHeight: 28

        color: root.backgroundColor

        Rectangle {
            id: selectedRect

            anchors.left: parent.left
            anchors.leftMargin: sideMargin
            anchors.right: parent.right
            anchors.rightMargin: sideMargin
            anchors.bottom: parent.bottom

            height: 2

            visible: isCurrent
            color: ui.theme.accentColor
        }
    }

    states: [
        State {
            name: "HOVERED"
            when: root.hovered

            PropertyChanges {
                target: contentItem
                opacity: 1
            }
        },
        State {
            name: "SELECTED"
            when: isCurrent

            PropertyChanges {
                target: contentItem
                opacity: 1
            }
        }
    ]
}

