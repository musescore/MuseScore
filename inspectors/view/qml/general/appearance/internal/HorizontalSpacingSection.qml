import QtQuick 2.9
import "../../../common"

Item {
    id: root

    height: childrenRect.height
    width: parent.width

    Column {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        spacing: 8

        StyledTextLabel {
            text: qsTr("Leading")
        }

        IncrementalPropertyControl {
            icon: "qrc:/resources/icons/horizontal_adjustment.svg"
        }
    }

    Column {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        spacing: 8

        StyledTextLabel {
            text: qsTr("Bar width")
        }

        IncrementalPropertyControl {
            icon: "qrc:/resources/icons/horizontal_adjustment.svg"
        }
    }
}
