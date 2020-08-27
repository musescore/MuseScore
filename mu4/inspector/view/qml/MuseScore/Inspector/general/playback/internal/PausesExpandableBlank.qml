import QtQuick 2.9
import MuseScore.UiComponents 1.0
import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: InspectorPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        titleText: qsTr("Pause time")
        propertyItem: model ? model.pauseTime : null

        IncrementalPropertyControl {
            iconMode: iconModeEnum.hidden

            measureUnitsSymbol: "s"
            isIndeterminate: model ? model.pauseTime.isUndefined : false
            currentValue: model ? model.pauseTime.value : 0

            minValue: 0.0
            maxValue: 60.0

            onValueEdited: { model.pauseTime.value = newValue }
        }
    }
}
