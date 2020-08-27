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

        CheckBox {
            id: followWrittenTempoCheckbox

            isIndeterminate: model ? model.isDefaultTempoForced.isUndefined : false
            checked: model && !isIndeterminate ? model.isDefaultTempoForced.value : false
            text: qsTr("Follow written tempo")

            onClicked: { model.isDefaultTempoForced.value = !checked }
        }

        InspectorPropertyView {
            titleText: qsTr("Override written tempo")
            propertyItem: model ? model.tempo : null

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
