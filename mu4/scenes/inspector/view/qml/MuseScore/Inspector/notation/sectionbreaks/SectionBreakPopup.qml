import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        InspectorPropertyView {
            titleText: qsTr("Pause before new section starts")
            propertyItem: model ? model.pauseDuration : null

            IncrementalPropertyControl {
                isIndeterminate: model ? model.pauseDuration.isUndefined : false
                currentValue: model ? model.pauseDuration.value : 0
                iconMode: iconModeEnum.hidden
                maxValue: 999
                minValue: 0
                step: 0.5
                measureUnitsSymbol: qsTr("s")

                onValueEdited: { model.pauseDuration.value = newValue }
            }
        }

        CheckBox {
            isIndeterminate: model ? model.shouldStartWithLongInstrNames.isUndefined : false
            checked: model && !isIndeterminate ? model.shouldStartWithLongInstrNames.value : false
            text: qsTr("Start new section with long instrument names")

            onClicked: { model.shouldStartWithLongInstrNames.value = !checked }
        }

        CheckBox {
            isIndeterminate: model ? model.shouldResetBarNums.isUndefined : false
            checked: model && !isIndeterminate ? model.shouldResetBarNums.value : false
            text: qsTr("Reset bar numbers for new section")

            onClicked: { model.shouldResetBarNums.value = !checked }
        }
    }
}
