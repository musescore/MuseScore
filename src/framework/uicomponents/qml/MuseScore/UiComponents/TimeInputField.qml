import QtQuick 2.15

import MuseScore.Ui 1.0

Row {
    id: root

    property date time // format: h:mm:ss:ms
    signal timeEdited(var newTime)

    property var font: ui.theme.tabFont

    spacing: 4

    opacity: enabled ? 1 : ui.theme.itemOpacityDisabled

    NumberInputField {
        maxValue: 9
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
        maxValue: 60
        value: root.time.getMinutes()

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
        maxValue: 60
        value: root.time.getSeconds()

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
        maxValue: 9
        value: root.time.getMilliseconds() / 100

        font: root.font

        onValueEdited: {
            var newTime = root.time
            newTime.setMilliseconds(newValue * 100)
            root.timeEdited(newTime)
        }
    }
}
