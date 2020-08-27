import QtQuick 2.9
import MuseScore.Inspector 1.0
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

        titleText: qsTr("Time stretch")
        propertyItem: model ? model.timeStretch : null

        IncrementalPropertyControl {
            id: timeStretchControl

            iconMode: iconModeEnum.hidden

            measureUnitsSymbol: "%"
            isIndeterminate: model ? model.timeStretch.isUndefined : false
            currentValue: model ? model.timeStretch.value : 0

            step: 1
            decimals: 0
            maxValue: 400
            minValue: 0
            validator: IntInputValidator {
                top: timeStretchControl.maxValue
                bottom: timeStretchControl.minValue
            }

            onValueEdited: { model.timeStretch.value = newValue }
        }
    }
}
