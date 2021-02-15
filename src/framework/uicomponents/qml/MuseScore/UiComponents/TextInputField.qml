import QtQuick 2.9
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import MuseScore.Ui 1.0

Rectangle {
    id: root

    property bool isIndeterminate: false
    readonly property string indeterminateText: "--"
    property var currentText: ""
    property alias validator: valueInput.validator
    property alias measureUnitsSymbol: measureUnitsLabel.text

    property alias hint: valueInput.placeholderText
    property alias hintIcon: hintIcon.iconCode
    property bool clearTextButtonVisible: false

    signal currentTextEdited(var newTextValue)
    signal textCleared()

    function selectAll() {
        valueInput.selectAll()
        forceActiveFocus()
    }

    function clear() {
        valueInput.text = ""
        currentText = ""
        textCleared()
    }

    function forceActiveFocus() {
        valueInput.forceActiveFocus()
    }

    implicitHeight: 30
    implicitWidth: parent.width

    color: ui.theme.textFieldColor
    border.color: ui.theme.strokeColor
    border.width: 1

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    radius: 4

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: hintIcon.visible ? 0 : 12
        anchors.rightMargin: 4

        spacing: 0

        StyledIconLabel {
            id: hintIcon

            Layout.fillHeight: true
            Layout.preferredWidth: 30

            visible: Boolean(!hintIcon.isEmpty)
        }

        TextField {
            id: valueInput

            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: !measureUnitsLabel.visible

            color: ui.theme.fontPrimaryColor
            font: ui.theme.bodyFont

            background: Item {}

            focus: false
            activeFocusOnPress: false
            selectByMouse: true
            selectionColor: Qt.rgba(ui.theme.accentColor.r, ui.theme.accentColor.g, ui.theme.accentColor.b, ui.theme.accentOpacityNormal)
            selectedTextColor: ui.theme.fontPrimaryColor
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
                if (!acceptableInput) {
                    return
                }

                root.currentTextEdited(text)
            }
        }

        StyledTextLabel {
            id: measureUnitsLabel

            Layout.alignment: Qt.AlignVCenter

            color: ui.theme.fontPrimaryColor
            visible: !root.isIndeterminate && Boolean(text)
        }

        FlatButton {
            id: clearTextButton

            Layout.fillHeight: true
            Layout.preferredWidth: height

            readonly property int margin: 4

            Layout.topMargin: margin
            Layout.bottomMargin: margin

            icon: IconCode.CLOSE_X_ROUNDED
            visible: root.clearTextButtonVisible

            normalStateColor: root.color
            hoveredStateColor: ui.theme.accentColor
            pressedStateColor: ui.theme.accentColor

            onClicked: {
                root.clear()
            }
        }

        Item {
            Layout.fillWidth: measureUnitsLabel.visible
        }
    }

    StyledTextLabel {
        id: undefinedValueLabel

        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.leftMargin: 12

        text: root.indeterminateText
        color: ui.theme.fontPrimaryColor
        visible: root.isIndeterminate && valueInput.activeFocus === false
    }

    states: [
        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !valueInput.activeFocus

            PropertyChanges {
                target: root
                border.color: ui.theme.strokeColor
                border.width: 1
                opacity: 0.6
            }
        },

        State {
            name: "FOCUSED"
            when: valueInput.activeFocus

            PropertyChanges {
                target: root
                border.color: ui.theme.accentColor
                border.width: 1
                opacity: 1
            }
        }
    ]

    MouseArea {
        id: clickableArea

        anchors.top: parent.top
        anchors.left: parent.left

        height: parent.height
        width: clearTextButton.visible ? parent.width - clearTextButton.width : parent.width

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
