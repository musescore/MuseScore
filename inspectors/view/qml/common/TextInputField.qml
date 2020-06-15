import QtQuick 2.9
import QtQuick.Controls 2.1

Rectangle {
    id: root

    property bool isIndeterminate: false
    readonly property string indeterminateText: "--"
    property var currentText
    property alias validator: valueInput.validator
    property alias measureUnitsSymbol: measureUnitsLabel.text

    signal currentTextEdited(var newTextValue)

    implicitHeight: 32
    implicitWidth: parent.width

    color: globalStyle.button
    border.width: 0
    border.color: globalStyle.highlight

    opacity: root.enabled ? 1.0 : 0.3

    radius: 4

    Behavior on color { ColorAnimation { duration: 100 } }
    Behavior on radius { NumberAnimation { duration: 100 } }

    TextInput {
        id: valueInput

        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.leftMargin: 12

        color: globalStyle.buttonText
        font {
            family: globalStyle.font.family
            pointSize: globalStyle.font.pointSize
        }

        focus: false
        activeFocusOnPress: false
        selectByMouse: true
        selectionColor: Qt.rgba(globalStyle.highlight.r, globalStyle.highlight.g, globalStyle.highlight.b, 0.75)
        selectedTextColor: globalStyle.buttonText
        visible: !root.isIndeterminate || activeFocus

        text: root.currentText === undefined ? "" : root.currentText

        Keys.onPressed: {
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                valueInput.focus = false
            }
        }

        onActiveFocusChanged: {
            if (activeFocus) {
                selectAll()
            } else {
                deselect()
            }
        }

        onTextChanged: {
            if (!acceptableInput)
                return;

            root.currentTextEdited(text)
        }

        states: [
            State {
                name: "NORMAL"
                when: !clickableArea.containsMouse && !valueInput.focus

                PropertyChanges {
                    target: root
                    border.color: globalStyle.button
                    border.width: 0
                    radius: 4
                }
            },

            State {
                name: "HOVERED"
                when: clickableArea.containsMouse && !valueInput.focus

                PropertyChanges {
                    target: root
                    border.color: globalStyle.highlight
                    border.width: 1
                    radius: 0
                }
            },

            State {
                name: "FOCUSED"
                when: valueInput.focus

                PropertyChanges {
                    target: root
                    border.color: Qt.lighter(globalStyle.button)
                    border.width: 1
                    radius: 0
                }
            }
        ]
    }

    StyledTextLabel {
        id: measureUnitsLabel

        anchors.left: valueInput.right
        anchors.leftMargin: 4
        anchors.verticalCenter: valueInput.verticalCenter

        color: globalStyle.buttonText
        visible: !root.isIndeterminate
    }

    StyledTextLabel {
        id: undefinedValueLabel

        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.leftMargin: 12

        text: root.indeterminateText
        color: globalStyle.buttonText
        visible: root.isIndeterminate && valueInput.activeFocus === false
    }

    MouseArea {
        id: clickableArea

        anchors.fill: parent

        propagateComposedEvents: true
        hoverEnabled: true

        onPressed: {
            if (!valueInput.activeFocus) {
                valueInput.forceActiveFocus()
            }

            mouse.accepted = false
        }
    }
}
