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

    color: "#FFFFFF"
    border.width: 0

    opacity: root.enabled ? 1.0 : 0.3

    radius: 4

    TextInput {
        id: valueInput

        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.leftMargin: 12

        color: "#000000"
        font {
            family: globalStyle.font.family
            pointSize: globalStyle.font.pointSize
        }

        focus: false
        activeFocusOnPress: false
        selectByMouse: true
        selectionColor: Qt.rgba(globalStyle.highlight.r, globalStyle.highlight.g, globalStyle.highlight.b, 0.75)
        selectedTextColor: "#000000"
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

                PropertyChanges { target: root; border.color: "#FFFFFF"
                                                border.width: 1 }
            },

            State {
                name: "HOVERED"
                when: clickableArea.containsMouse && !valueInput.focus

                PropertyChanges { target: root; border.color: "#CECECE"
                                                border.width: 1 }
            },

            State {
                name: "FOCUSED"
                when: valueInput.focus && valueInput.selectedText === ""

                PropertyChanges { target: root; border.color: "#ADADAD"
                                                border.width: 1 }
            },

            State {
                name: "TEXT_SELECTED"
                when: valueInput.focus && valueInput.selectedText !== ""

                PropertyChanges { target: root; border.color: globalStyle.highlight
                                                border.width: 1}
            }
        ]
    }

    StyledTextLabel {
        id: measureUnitsLabel

        anchors.left: valueInput.right
        anchors.leftMargin: 4
        anchors.verticalCenter: valueInput.verticalCenter

        color: "#000000"
        visible: !root.isIndeterminate
    }

    StyledTextLabel {
        id: undefinedValueLabel

        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.leftMargin: 12

        text: root.indeterminateText
        color: "#000000"
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
