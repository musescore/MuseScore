import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0

Row {
    id: root

    property alias title: titleLabel.text
    property alias titleWidth: titleLabel.width

    property alias currentValue: control.currentValue
    property alias minValue: control.minValue
    property alias maxValue: control.maxValue
    property alias measureUnitsSymbol: control.measureUnitsSymbol

    property alias control: control

    signal valueEdited(var newValue)

    spacing: 0

    StyledTextLabel {
        id: titleLabel

        anchors.verticalCenter: parent.verticalCenter

        horizontalAlignment: Qt.AlignLeft
        wrapMode: Text.WordWrap
        maximumLineCount: 2
    }

    IncrementalPropertyControl {
        id: control

        width: 102
        decimals: 0
        step: 1

        onValueEdited: {
            root.valueEdited(newValue)
        }
    }
}
