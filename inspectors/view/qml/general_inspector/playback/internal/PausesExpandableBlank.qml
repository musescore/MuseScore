import QtQuick 2.9
import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: Column {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        spacing: 8

        height: implicitHeight
        width: root.width

        StyledTextLabel {
            text: qsTr("Pause time")
        }

        IncrementalPropertyControl {
            iconMode: iconModeEnum.hidden

            measureUnitsSymbol: "s"
            isIndeterminate: model ? model.pauseTime.isUndefined : false
            currentValue: model ? model.pauseTime.value : 0

            onValueEdited: { model.pauseTime.value = newValue }
        }
    }
}
