import QtQuick 2.9
import QtQuick.Controls 2.1
import MuseScore.Ui 1.0

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

    color: ui.theme.textFieldColor
    border.width: 0

    opacity: root.enabled ? 1.0 : 0.3

    radius: 4

    TextInput {
        id: valueInput

        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.leftMargin: 12

        color: ui.theme.fontColor
        font {
            family: ui.theme.font.family
            pointSize: ui.theme.font.pointSize
        }

        focus: false
        activeFocusOnPress: false
        selectByMouse: true
        selectionColor: Qt.rgba(ui.theme.accentColor.r, ui.theme.accentColor.g, ui.theme.accentColor.b, ui.theme.accentOpacityNormal)
        selectedTextColor: ui.theme.fontColor
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

                PropertyChanges { target: root; border.color: ui.theme.textFieldColor
                                                border.width: 1 }
            },

            State {
                name: "HOVERED"
                when: clickableArea.containsMouse && !valueInput.focus

                PropertyChanges { target: root; border.color: ui.theme.strokeColor
                                                border.width: 1 }
            },

            State {
                name: "FOCUSED"
                when: valueInput.focus && valueInput.selectedText === ""

                PropertyChanges { target: root; border.color: ui.theme.accentColor
                                                border.width: 1 }
            },

            State {
                name: "TEXT_SELECTED"
                when: valueInput.focus && valueInput.selectedText !== ""

                PropertyChanges { target: root; border.color: ui.theme.accentColor
                                                border.width: 1}
            }
        ]
    }

    StyledTextLabel {
        id: measureUnitsLabel

        anchors.left: valueInput.right
        anchors.leftMargin: 4
        anchors.verticalCenter: valueInput.verticalCenter

        color: ui.theme.fontColor
        visible: !root.isIndeterminate
    }

    StyledTextLabel {
        id: undefinedValueLabel

        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.leftMargin: 12

        text: root.indeterminateText
        color: ui.theme.fontColor
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
