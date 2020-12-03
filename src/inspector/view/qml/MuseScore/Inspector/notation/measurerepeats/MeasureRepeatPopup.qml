import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
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
            titleText: qsTr("Number position")
            propertyItem: model ? model.numberPosition : null

            IncrementalPropertyControl {
                isIndeterminate: model ? model.numberPosition.isUndefined : false
                currentValue: model ? model.numberPosition.value : 0
                iconMode: iconModeEnum.hidden
                maxValue: 99.0
                minValue: -99.0
                step: 0.5
                decimals: 2

                onValueEdited: { model.numberPosition.value = newValue }
            }
        }
    }
}
