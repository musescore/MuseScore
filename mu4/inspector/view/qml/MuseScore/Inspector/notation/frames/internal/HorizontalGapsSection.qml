import QtQuick 2.9
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

Item {
    id: root

    property QtObject leftGap: undefined
    property QtObject rightGap: undefined

    height: childrenRect.height
    width: parent.width

    InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 4

        titleText: qsTr("Left gap")
        propertyItem: leftGap

        IncrementalPropertyControl {
            icon: IconCode.LEFT_GAP

            enabled: leftGap ? leftGap.isEnabled : false
            isIndeterminate: leftGap && enabled ? leftGap.isUndefined : false
            currentValue: leftGap ? leftGap.value : 0

            onValueEdited: { leftGap.value = newValue }
        }
    }

    InspectorPropertyView {
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 4
        anchors.right: parent.right

        titleText: qsTr("Right gap")
        propertyItem: rightGap

        IncrementalPropertyControl {
            icon: IconCode.RIGHT_GAP

            enabled: rightGap ? rightGap.isEnabled : false
            isIndeterminate: rightGap && enabled ? rightGap.isUndefined : false
            currentValue: rightGap ? rightGap.value : 0

            onValueEdited: { rightGap.value = newValue }
        }
    }
}
