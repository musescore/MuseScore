import QtQuick 2.9
import MuseScore.Inspectors 3.3

import "../../../common"

Column {

    property QtObject intBracketProperty: undefined
    property alias title: textLabel.text

    width: parent.width
    spacing: 8

    StyledTextLabel {
        id: textLabel
    }

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

