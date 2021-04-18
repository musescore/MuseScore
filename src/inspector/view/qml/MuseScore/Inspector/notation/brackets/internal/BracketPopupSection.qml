import QtQuick 2.9
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0

import "../../../common"

InspectorPropertyView {

    property QtObject intBracketProperty: undefined

    propertyItem: intBracketProperty

    IncrementalPropertyControl {
        id: columnsPositionControl
        iconMode: iconModeEnum.hidden

        step: 1
        decimals: 0
        maxValue: 127
        minValue: 0
        validator: IntInputValidator {
            top: columnsPositionControl.maxValue
            bottom: columnsPositionControl.minValue
        }

        isIndeterminate: intBracketProperty ? intBracketProperty.isUndefined : false
        currentValue: intBracketProperty ? intBracketProperty.value : 0

        onValueEdited: { intBracketProperty.value = newValue }
    }
}

