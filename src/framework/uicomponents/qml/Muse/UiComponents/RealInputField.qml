import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

TextInputField {

    id: root

    property real currentValue: 0.0

    property real max: 10000.0
    property real min: -10000.0
    property int decimals: 2

    currentText: parseFloat(root.currentValue.toFixed(root.decimals))

    validator: DoubleInputValidator {
        top: root.max
        bottom: root.min
        decimal: root.decimals
    }

    onTextEdited: function(newTextValue) {
        root.currentValue = parseFloat(newTextValue)
    }
}
