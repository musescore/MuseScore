import QtQuick 2.15

import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    Row {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8

        Rectangle {
            id: item1

            width: 32
            height: 32
            color: "#eeeeee"

            Accessible.role: Accessible.Button
            Accessible.name: "Item 1"
            Accessible.onPressAction: {
                // do a button click
            }


            StyledTextLabel {
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: "Item 1"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: parent.forceActiveFocus()
            }
        }

        Rectangle {
            id: item2

            width: 32
            height: 32
            color: "#eeeeee"

            Accessible.role: Accessible.Button
            Accessible.name: "Item 2"
            Accessible.onPressAction: {
                // do a button click
            }


            StyledTextLabel {
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: "Item 2"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: parent.forceActiveFocus()
            }
        }
    }
}
