import QtQuick 2.9

import MuseScore.UiComponents 1.0

Row {
    id: root

    property var numerator: 0
    property var denominator: 0
    property var availableDenominators: null

    signal numeratorSelected(var value)
    signal denominatorSelected(var value)

    spacing: 14

    IncrementalPropertyControl {
        id: control

        implicitWidth: 80

        iconMode: iconModeEnum.hidden
        currentValue: root.numerator
        step: 1

        maxValue: 63
        minValue: 1
        validator: IntInputValidator {
            top: control.maxValue
            bottom: control.minValue
        }

        onValueEdited: {
            numeratorSelected(newValue)
        }
    }

    StyledTextLabel {
        anchors.verticalCenter: parent.verticalCenter
        font: ui.theme.tabFont
        text: "/"
    }

    StyledComboBox {
        id: timeComboBox

        implicitWidth: 90

        textRoleName: "text"
        valueRoleName: "value"

        currentIndex: indexOfValue(root.denominator)

        model: {
            var resultList = []

            var denominators = root.availableDenominators
            for (var i = 0; i < denominators.length; ++i) {
                resultList.push({"text" : denominators[i], "value" : denominators[i]})
            }

            return resultList
        }

        onValueChanged: {
            denominatorSelected(value)
        }
    }
}
