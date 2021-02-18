import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0

Item {
    id: root

    property int minValue: 0
    property int maxValue: 999
    property int value: 0
    property bool padWithZeros: true

    property alias font: textField.font

    signal valueEdited(var newValue)

    implicitWidth: textField.contentWidth
    implicitHeight: 30

    opacity: enabled ? 1 : ui.theme.itemOpacityDisabled

    QtObject {
        id: privateProperties

        property int maxNumberLength: root.maxValue.toString().length

        function pad(value) {
            var str = value.toString()

            if (!padWithZeros) {
                return str;
            }

            while (str.length < maxNumberLength) {
                str = "0" + str
            }

            return str
        }
    }

    onValueChanged: {
        textField.text = privateProperties.pad(value)
    }

    TextField {
        id: textField

        anchors.centerIn: parent

        readOnly: root.maxValue === 0
        text: privateProperties.pad(root.value)

        onTextEdited: {
            var currentValue = parseInt(text)
            var str = currentValue.toString()
            var newValue = 0

            if (str.length > privateProperties.maxNumberLength || currentValue > root.maxValue) {
                var lastDigit = str.charAt(str.length - 1)
                newValue = parseInt(lastDigit)
            } else {
                newValue = currentValue
            }

            if (newValue > root.maxValue) {
                newValue = value
            }

            text = privateProperties.pad(newValue)
            root.valueEdited(newValue)
        }

        background: Rectangle {
            id: textFieldBackground

            color: "transparent"
        }

        cursorDelegate: Item {}
        selectByMouse: false

        color: ui.theme.fontPrimaryColor
        font: ui.theme.tabFont

        validator: IntValidator {
            bottom: root.minValue
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        enabled: !textField.readOnly

        onClicked: {
            textField.forceActiveFocus()
        }
    }

    states: [
        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed && !textField.activeFocus

            PropertyChanges {
                target: textFieldBackground
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: mouseArea.pressed && !textField.activeFocus

            PropertyChanges {
                target: textFieldBackground
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "FOCUSED"
            when: textField.activeFocus && !textField.readOnly

            PropertyChanges {
                target: textFieldBackground
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityNormal
            }
        }
    ]
}
