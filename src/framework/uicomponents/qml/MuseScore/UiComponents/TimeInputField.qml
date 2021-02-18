import QtQuick 2.15

import MuseScore.Ui 1.0

Row {
    id: root

    property date time
    property date maxTime
    property alias maxMillisecondsNumber: millisecondsField.maxValue

    property var font: ui.theme.tabFont

    signal timeEdited(var newTime)

    spacing: 4

    opacity: enabled ? 1 : ui.theme.itemOpacityDisabled

    NumberInputField {
        id: hoursField

        maxValue: root.maxTime.getHours()
        value: root.time.getHours()

        font: root.font

        onValueEdited: {
            var newTime = root.time
            newTime.setHours(newValue)
            root.timeEdited(newTime)
        }
    }

    StyledTextLabel {
        anchors.verticalCenter: parent.verticalCenter
        text: ":"
        font: root.font
    }

    NumberInputField {
        id: minutesField

        maxValue: hoursField.value === root.maxTime.getHours() ? root.maxTime.getMinutes() : 60
        value: root.time.getMinutes()

        displayedNumberLength: 2
        font: root.font

        onValueEdited: {
            var newTime = root.time
            newTime.setMinutes(newValue)
            root.timeEdited(newTime)
        }
    }

    StyledTextLabel {
        anchors.verticalCenter: parent.verticalCenter
        text: ":"
        font: root.font
    }

    NumberInputField {
        id: secondsField

        maxValue: minutesField.value === root.maxTime.getMinutes() ? root.maxTime.getSeconds() : 60
        value: root.time.getSeconds()

        displayedNumberLength: 2
        font: root.font

        onValueEdited: {
            var newTime = root.time
            newTime.setSeconds(newValue)
            root.timeEdited(newTime)
        }
    }

    StyledTextLabel {
        anchors.verticalCenter: parent.verticalCenter
        text: ":"
        font: root.font
    }

    NumberInputField {
        id: millisecondsField

        readonly property int maxDigitCount: maxValue.toString().length
        readonly property int precision: Math.max(1000 / Math.pow(10, maxDigitCount), 1)

        maxValue: secondsField.value === root.maxTime.getSeconds() ? root.maxTime.getMilliseconds() : 1000
        value: root.time.getMilliseconds() / precision

        font: root.font

        onValueEdited: {
            var newTime = root.time
            newTime.setMilliseconds(newValue * precision)
            root.timeEdited(newTime)
        }
    }
}
