import QtQuick 2.9
import MuseScore.Inspectors 3.3
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
