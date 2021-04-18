import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.UiComponents 1.0
import "../../../common"

Column {
    id: root

    signal pushBackRequested()
    signal pushFrontRequested()

    height: implicitHeight
    width: parent.width

    spacing: 16

    Column {
        width: parent.width

        spacing: 8

        StyledTextLabel {
            text: qsTrc("inspector", "Arrange")
        }

        Item {
            height: childrenRect.height
            width: parent.width

            FlatButton {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 4

                text: qsTrc("inspector", "Backwards")

                onClicked: {
                    root.pushBackRequested()
                }
            }

            FlatButton {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 4
                anchors.right: parent.right

                text: qsTrc("inspector", "Forwards")

                onClicked: {
                    root.pushFrontRequested()
                }
            }
        }
    }
}
