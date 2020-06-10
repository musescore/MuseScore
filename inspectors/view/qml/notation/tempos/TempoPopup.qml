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

        CheckBox {
            id: followWrittenTempoCheckbox

            isIndeterminate: model ? model.isDefaultTempoForced.isUndefined : false
            checked: model && !isIndeterminate ? model.isDefaultTempoForced.value : false
            text: qsTr("Follow written tempo")

            onClicked: { model.isDefaultTempoForced.value = !checked }
        }

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                text: qsTr("Override written tempo")
            }

            IncrementalPropertyControl {
                enabled: model ? !model.isEmpty && !followWrittenTempoCheckbox.checked : false
                isIndeterminate: model ? model.tempo.isUndefined : false
                currentValue: model ? model.tempo.value : 0
                iconMode: iconModeEnum.hidden
                measureUnitsSymbol: qsTr("BPM")

                onValueEdited: { model.tempo.value = newValue }
            }
        }
    }
}
