import QtQuick 2.15

import MuseScore.Ui 1.0

Row {
    id: root

    property date time // format: h:mm:ss:ms

    signal timeEdited(var newTime)

    spacing: 4

    opacity: enabled ? 1 : ui.theme.itemOpacityDisabled

    NumberInputField {
        id: hoursField

        maxValue: 9

        value: root.time.getHours()

        onValueEdited: {
            var newTime = root.time
            newTime.setHours(newValue)
            root.timeEdited(newTime)
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

        onValueEdited: {
            var newTime = root.time
            newTime.setMinutes(newValue)
            root.timeEdited(newTime)
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

        onValueEdited: {
            var newTime = root.time
            newTime.setSeconds(newValue)
            root.timeEdited(newTime)
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

        onValueEdited: {
            var newTime = root.time
            newTime.setMilliseconds(newValue)
            root.timeEdited(newTime)
        }
    }
}
