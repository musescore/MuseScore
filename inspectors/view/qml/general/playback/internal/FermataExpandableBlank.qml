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
            text: qsTr("Time stretch")
        }

        IncrementalPropertyControl {
            iconMode: iconModeEnum.hidden

            measureUnitsSymbol: "%"
            isIndeterminate: model ? model.timeStretch.isUndefined : false
            currentValue: model ? model.timeStretch.value : 0

            onValueEdited: { model.timeStretch.value = newValue }
        }
    }
}
