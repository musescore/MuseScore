import QtQuick 2.9
import MuseScore.Inspectors 3.3
import "../../../common"

Item {
    id: root

    property QtObject frameTopMargin: undefined
    property QtObject frameBottomMargin: undefined

    height: childrenRect.height
    width: parent.width

    Column {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        spacing: 8

        StyledTextLabel {
            text: qsTr("Top margin")
        }

        IncrementalPropertyControl {
            icon: IconNameTypes.TOP_MARGIN

            measureUnitsSymbol: qsTr("mm")

            enabled: frameTopMargin ? frameTopMargin.isEnabled : false
            isIndeterminate: frameTopMargin && enabled ? frameTopMargin.isUndefined : false
            currentValue: frameTopMargin ? frameTopMargin.value : 0

            onValueEdited: { frameTopMargin.value = newValue }
        }
    }

    Column {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        spacing: 8

        StyledTextLabel {
            text: qsTr("Bottom margin")
        }

        IncrementalPropertyControl {
            icon: IconNameTypes.BOTTOM_MARGIN

            measureUnitsSymbol: qsTr("mm")

            enabled: frameBottomMargin ? frameBottomMargin.isEnabled : false
            isIndeterminate: frameBottomMargin && enabled ? frameBottomMargin.isUndefined : false
            currentValue: frameBottomMargin ? frameBottomMargin.value : 0

            onValueEdited: { frameBottomMargin.value = newValue }
        }
    }
}
