import QtQuick 2.15

import MuseScore.Ui 1.0

Row {
    id: root

    property date time // format: h:mm:ss:ms

    spacing: 4

    opacity: enabled ? 1 : ui.theme.itemOpacityDisabled

    NumberInputField {
        id: hoursField

        maxValue: 9

        value: root.time.getHours()

        onValueChanged: {
            var newTime = root.time
            newTime.setHours(value)
            root.time = newTime
        }
    }

    StyledTextLabel {
        anchors.verticalCenter: parent.verticalCenter
        text: ":"
        font: hoursField.font
    }

    NumberInputField {
        maxValue: 60

        value: root.time.getMinutes()

        onValueChanged: {
            var newTime = root.time
            newTime.setMinutes(value)
            root.time = newTime
        }
    }

    StyledTextLabel {
        anchors.verticalCenter: parent.verticalCenter
        text: ":"
        font: hoursField.font
    }

    NumberInputField {
        maxValue: 60

        value: root.time.getSeconds()

        onValueChanged: {
            var newTime = root.time
            newTime.setSeconds(value)
            root.time = newTime
        }
    }

    StyledTextLabel {
        anchors.verticalCenter: parent.verticalCenter
        text: ":"
        font: hoursField.font
    }

    NumberInputField {
        maxValue: 9

        value: root.time.getMilliseconds()

        onValueChanged: {
            var newTime = root.time
            newTime.setMilliseconds(value)
            root.time = newTime
        }
    }
}
