import QtQuick 2.15

import MuseScore.Ui 1.0

Row {
    id: root

    property date time // format: h:mm:ss:ms
    signal timeEdited(var newTime)

    property int maxHours: 9
    property int maxMinutes: 60
    property int maxSeconds: 60
    property int maxMilliseconds: 9

    property var font: ui.theme.tabFont

    spacing: 4

    opacity: enabled ? 1 : ui.theme.itemOpacityDisabled

    NumberInputField {
        maxValue: root.maxHours
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
        maxValue: root.maxMinutes
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
        maxValue: root.maxSeconds
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
        maxValue: root.maxMilliseconds
        value: root.time.getMilliseconds()

        font: root.font

        onValueEdited: {
            var newTime = root.time
            newTime.setMilliseconds(newValue)
            root.timeEdited(newTime)
        }
    }
}
