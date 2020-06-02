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
                text: qsTr("Height")
            }

            IncrementalPropertyControl {
                isIndeterminate: model ? model.spacerHeight.isUndefined : false
                currentValue: model ? model.spacerHeight.value : 0
                iconMode: iconModeEnum.hidden
                maxValue: 999
                minValue: 0
                step: 0.5

                onValueEdited: { model.spacerHeight.value = newValue }
            }
        }
    }
}
