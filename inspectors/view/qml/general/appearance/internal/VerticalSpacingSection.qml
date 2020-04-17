import QtQuick 2.9
import "../../../common"

Column {
    id: root

    property QtObject minimumDistance: undefined

    anchors.left: parent.left
    anchors.right: parent.horizontalCenter
    anchors.rightMargin: 2

    spacing: 8

    StyledTextLabel {
        text: qsTr("Minimum distance")
    }

    IncrementalPropertyControl {
        icon: "qrc:/resources/icons/vertical_adjustment.svg"

        isIndeterminate: minimumDistance ? minimumDistance.isUndefined : false
        currentValue: minimumDistance ? minimumDistance.value : 0

        onValueEdited: { minimumDistance.value = newValue }
    }
}
