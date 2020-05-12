import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspectors 3.3
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

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                text: qsTr("Pause before new section starts")
            }

            IncrementalPropertyControl {
                isIndeterminate: model ? model.pause.isUndefined : false
                currentValue: model ? model.pause.value : 0
                iconMode: iconModeEnum.hidden
                maxValue: 999
                minValue: 0
                step: 0.5
                measureUnitsSymbol: qsTr("s")

                onValueEdited: { model.pause.value = newValue }
            }
        }

        CheckBox {
            isIndeterminate: model ? model.startWithLongInstrNames.isUndefined : false
            checked: model && !isIndeterminate ? model.startWithLongInstrNames.value : false
            text: qsTr("Start new section with long instrument names")

            onClicked: { model.startWithLongInstrNames.value = !checked }
        }

        CheckBox {
            isIndeterminate: model ? model.resetBarNums.isUndefined : false
            checked: model && !isIndeterminate ? model.resetBarNums.value : false
            text: qsTr("Reset bar numbers for new section")

            onClicked: { model.resetBarNums.value = !checked }
        }
    }
}
