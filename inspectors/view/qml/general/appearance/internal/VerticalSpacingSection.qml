import QtQuick 2.9
import "../../../common"

Column {
    anchors.left: parent.left
    anchors.right: parent.horizontalCenter
    anchors.rightMargin: 2

    spacing: 8

    StyledTextLabel {
        text: qsTr("Minimum distance")
    }

    IncrementalPropertyControl {
        icon: "qrc:/resources/icons/vertical_adjustment.svg"
    }
}
