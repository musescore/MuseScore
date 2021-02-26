import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

RowLayout {
    id: root

    property alias measureNumber: measureNumberField.value
    property alias maxMeasureNumber: measureNumberField.maxValue

    property alias beatNumber: beatNumberField.value
    property alias maxBeatNumber: beatNumberField.maxValue

    property var font: ui.theme.tabFont

    signal measureNumberEdited(var newValue)
    signal beatNumberEdited(var newValue)

    spacing: 4

    Item {
        Layout.preferredWidth: 20

        NumberInputField {
            id: measureNumberField

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            minValue: 1
            addLeadingZeros: false

            font: root.font

            onValueEdited: {
                root.measureNumberEdited(newValue)
            }
        }
    }

    StyledTextLabel {
        text: "."
        font: root.font
    }

    Item {
        Layout.preferredWidth: 10

        NumberInputField {
            id: beatNumberField

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            minValue: 1
            addLeadingZeros: false

            font: root.font

            onValueEdited: {
                root.beatNumberEdited(newValue)
            }
        }
    }
}
