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

    signal currentTextEdited(var newTextValue)

    implicitHeight: 32
    implicitWidth: parent.width

    color: ui.theme.textFieldColor
    border.color: ui.theme.textFieldColor
    border.width: 1

    opacity: root.enabled ? 1.0 : 0.3

    radius: 4

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: hintIcon.visible ? 0 : 12
        anchors.rightMargin: 12

        spacing: 0

        Item {
            height: root.height
            width: height

            StyledIconLabel {
                id: hintIcon

                anchors.centerIn: parent

                color: ui.theme.fontSecondaryColor
            }

            visible: Boolean(hintIcon.iconCode)
        }

        TextField {
            id: valueInput

            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: !measureUnitsLabel.visible

            color: ui.theme.fontPrimaryColor

            font {
                family: ui.theme.font.family
                pointSize: ui.theme.font.pointSize
            }

            background: Rectangle {
                color: root.color
            }

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
            when: clickableArea.containsMouse && !valueInput.focus

            PropertyChanges {
                target: root
                border.color: ui.theme.strokeColor
                border.width: 1
            }

            PropertyChanges {
                target: hintIcon
                color: ui.theme.strokeColor
            }

            /*PropertyChanges {
                target: valueInput
                placeholderTextColor: ui.theme.strokeColor
            }*/
        },

        State {
            name: "FOCUSED"
            when: valueInput.focus && valueInput.selectedText === ""

            PropertyChanges {
                target: root
                border.color: ui.theme.accentColor
                border.width: 1
            }

            PropertyChanges {
                target: hintIcon
                color: ui.theme.fontPrimaryColor
            }
        },

        State {
            name: "TEXT_SELECTED"
            when: valueInput.focus && valueInput.selectedText !== ""

            PropertyChanges {
                target: root
                border.color: ui.theme.accentColor
                border.width: 1
            }

            PropertyChanges {
                target: hintIcon
                color: ui.theme.fontPrimaryColor
            }
        }
    ]

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
