import QtQuick 2.9
import "../../../common"

Column {
    id: root

    height: implicitHeight
    width: parent.width

    spacing: 16

    Column {
        spacing: 8

        height: implicitHeight
        width: parent.width

        StyledTextLabel {
            anchors.left: parent.left

            text: qsTr("Offset")
        }

        Item {
            height: childrenRect.height
            width: parent.width

            IncrementalPropertyControl {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 4

                icon: "qrc:/resources/icons/horizontal_adjustment.svg"
            }

            IncrementalPropertyControl {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 4
                anchors.right: parent.right

                icon: "qrc:/resources/icons/vertical_adjustment.svg"
            }
        }
    }

    CheckBox {
        id: snapToGridCheckbox

        text: qsTr("Snap to grid")
    }

    FlatButton {
        text: qsTr("Configure grid")

        visible: snapToGridCheckbox.checked
    }
}
