import QtQuick 2.9
import MuseScore.Inspectors 3.3
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

Item {
    id: root

    property QtObject leftGap: undefined
    property QtObject rightGap: undefined

    height: childrenRect.height
    width: parent.width

    Column {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        spacing: 8

        StyledTextLabel {
            text: qsTr("Left gap")
        }

        IncrementalPropertyControl {
            icon: IconCode.LEFT_GAP

            enabled: leftGap ? leftGap.isEnabled : false
            isIndeterminate: leftGap && enabled ? leftGap.isUndefined : false
            currentValue: leftGap ? leftGap.value : 0

            onValueEdited: { leftGap.value = newValue }
        }
    }

    Column {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        spacing: 8

        StyledTextLabel {
            text: qsTr("Right gap")
        }

        IncrementalPropertyControl {
            icon: IconCode.RIGHT_GAP

            enabled: rightGap ? rightGap.isEnabled : false
            isIndeterminate: rightGap && enabled ? rightGap.isUndefined : false
            currentValue: rightGap ? rightGap.value : 0

            onValueEdited: { rightGap.value = newValue }
        }
    }
}
