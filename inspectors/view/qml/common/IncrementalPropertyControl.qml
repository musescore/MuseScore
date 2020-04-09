import QtQuick 2.9
import MuseScore.Inspectors 3.3 // should be removed when component will be moved to the gui module, currently needed only for DoubleInputValidator

FocusableItem {
    id: root

    property alias iconModeEnum: _iconModeEnum
    property int iconMode: iconModeEnum.left
    property int iconSize: 16
    property alias icon: iconImage.icon

    property bool isIndeterminate: false
    readonly property string indeterminateText: "--"
    property var currentValue
    property real step: 0.5
    property int decimals: 2
    property real maxValue: 999
    property real minValue: -999
    property alias measureUnitsSymbol: measureUnitsLabel.text

    readonly property int spacing: 8

    signal valueEdited(var newValue)

    implicitHeight: 32
    implicitWidth: parent.width

    QtObject {
        id: _iconModeEnum

        readonly property int left: 1
        readonly property int right: 2
        readonly property int hidden: 3
    }

    Rectangle {
        id: iconBackground

        anchors.verticalCenter: parent.verticalCenter

        height: root.iconSize
        width: root.iconSize

        color: globalStyle.button

        opacity: root.enabled ? 1.0 : 0.3

        visible: String(iconImage.icon) !== ""

        StyledIcon {
            id: iconImage

            anchors.centerIn: parent
            sourceSize.height: iconSize
            sourceSize.width: iconSize
        }
    }

    Rectangle {
        id: propertyEditorRect

        anchors.top: parent.top
        anchors.bottom: parent.bottom

        color: "#FFFFFF"
        border.color: "#A2A2A2"
        border.width: 1

        opacity: root.enabled ? 1.0 : 0.3

        radius: 2

        TextInput {
            id: valueInput

            anchors.verticalCenter: propertyEditorRect.verticalCenter
            anchors.left: propertyEditorRect.left
            anchors.leftMargin: 12

            color: "#000000"
            font {
                family: globalStyle.font.family
                pointSize: globalStyle.font.pointSize
            }
            maximumLength: 6

            focus: false
            activeFocusOnPress: false
            selectByMouse: true
            visible: !root.isIndeterminate || activeFocus

            text: root.currentValue === undefined ? "" : root.currentValue
            validator: DoubleInputValidator {}

            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    valueInput.focus = false
                }
            }

            onTextChanged: {
                if (!acceptableInput)
                    return;

                var newValue = parseFloat(text)

                if (isNaN(newValue)) {
                    newValue = 0
                }

                root.valueEdited(+newValue.toFixed(decimals))
            }
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

            anchors.verticalCenter: propertyEditorRect.verticalCenter
            anchors.left: propertyEditorRect.left
            anchors.leftMargin: 12

            text: root.indeterminateText
            color: "#000000"
            visible: root.isIndeterminate && valueInput.activeFocus === false
        }

        MouseArea {
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
                right: valueAdjustControl.left
            }

            propagateComposedEvents: true

            onPressed: {
                if (!valueInput.activeFocus) {
                    valueInput.forceActiveFocus()
                }

                mouse.accepted = false
            }
        }

        ValueAdjustControl {
            id: valueAdjustControl

            anchors.verticalCenter: propertyEditorRect.verticalCenter
            anchors.right: propertyEditorRect.right

            icon: "qrc:/resources/icons/arrow_down.svg"

            onIncreaseButtonClicked: {
                var value = root.isIndeterminate ? 0.0 : currentValue
                var newValue = value + step

                if (newValue > root.maxValue)
                    return

                root.valueEdited(+newValue.toFixed(decimals))
            }

            onDecreaseButtonClicked: {
                var value = root.isIndeterminate ? 0.0 : currentValue
                var newValue = value - step

                if (newValue < root.minValue)
                    return

                root.valueEdited(+newValue.toFixed(decimals))
            }
        }
    }

    states: [
        State {
            name: "ICON_ALIGN_LEFT"
            when: root.iconMode === iconModeEnum.left

            AnchorChanges { target: iconBackground; anchors.left: root.left }

            PropertyChanges { target: iconBackground; visible: true }

            AnchorChanges { target: propertyEditorRect; anchors.left: iconBackground.right }

            PropertyChanges { target: propertyEditorRect; anchors.leftMargin: spacing
                                                          width: root.width - iconBackground.width - root.spacing }
        },

        State {
            name: "ICON_ALIGN_RIGHT"
            when: root.iconMode === iconModeEnum.right

            AnchorChanges { target: propertyEditorRect; anchors.left: root.left }

            PropertyChanges { target: propertyEditorRect; width: root.width - iconBackground.width - root.spacing }

            AnchorChanges { target: iconBackground; anchors.left: propertyEditorRect.right }

            PropertyChanges { target: iconBackground; anchors.leftMargin: spacing
                                                      visible: true }
        },

        State {
            name: "ICON_MODE_HIDDEN"
            when: String(root.icon) === ""

            AnchorChanges { target: propertyEditorRect; anchors.left: root.left }

            PropertyChanges { target: propertyEditorRect; width: root.width }

            PropertyChanges { target: iconBackground; visible: false }
        }
    ]
}
