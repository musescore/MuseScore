import QtQuick 2.9

import MuseScore.UiComponents 1.0

Row {
    id: root

    property int numerator: 0
    property int denominator: 0
    property var availableDenominators: null

    signal numeratorSelected(var value)
    signal denominatorSelected(var value)

    spacing: 14

    IncrementalPropertyControl {
        id: control

        implicitWidth: 68
        anchors.verticalCenter: parent.verticalCenter

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
            root.numeratorSelected(newValue)
        }
    }

    StyledTextLabel {
        anchors.verticalCenter: parent.verticalCenter
        font: ui.theme.largeBodyFont
        text: "/"
    }

    StyledComboBox {
        id: timeComboBox

        implicitWidth: control.width
        anchors.verticalCenter: parent.verticalCenter

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
            root.denominatorSelected(value)
        }
    }
}
