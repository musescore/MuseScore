import QtQuick 2.9
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

Item {
    id: root

    property QtObject gapAbove: undefined
    property QtObject gapBelow: undefined

    height: childrenRect.height
    width: parent.width

    Column {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        spacing: 8

        StyledTextLabel {
            text: qsTr("Gap above")
        }

        IncrementalPropertyControl {
            icon: IconCode.GAP_ABOVE

            enabled: gapAbove ? gapAbove.isEnabled : false
            isIndeterminate: gapAbove && enabled ? gapAbove.isUndefined : false
            currentValue: gapAbove ? gapAbove.value : 0

            onValueEdited: { gapAbove.value = newValue }
        }
    }

    Column {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        spacing: 8

        StyledTextLabel {
            text: qsTr("Gap below")
        }

        IncrementalPropertyControl {
            icon: IconCode.GAP_BELOW

            enabled: gapBelow ? gapBelow.isEnabled : false
            isIndeterminate: gapBelow && enabled ? gapBelow.isUndefined : false
            currentValue: gapBelow ? gapBelow.value : 0

            onValueEdited: { gapBelow.value = newValue }
        }
    }
}
