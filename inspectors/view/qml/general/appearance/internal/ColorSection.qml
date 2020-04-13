import QtQuick 2.9
import QtQuick.Dialogs 1.2
import "../../../common"

Column {
    id: root

    height: implicitHeight
    width: parent.width

    spacing: 8

    StyledTextLabel {
        text: qsTr("Colour")
    }

    Rectangle {
        id: colorPreviewRect

        height: 26
        width: parent.width
        color: "#000000"

        MouseArea {
            anchors.fill: parent

            onClicked: {
                colorDialog.open()
            }
        }

        ColorDialog {
            id: colorDialog

            onAccepted: {
                colorPreviewRect.color = colorDialog.color
            }
        }
    }
}
