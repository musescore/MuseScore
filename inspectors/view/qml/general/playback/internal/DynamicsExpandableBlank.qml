import QtQuick 2.9
import MuseScore.Inspectors 3.3
import QtQuick.Controls 2.0
import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: Column {
        id: contentLayout

        height: implicitHeight
        width: root.width

        spacing: 16

        Column {
            spacing: 8

            width: parent.width

            StyledTextLabel {
                text: qsTr("Applies to")
            }

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTr("Staff"), value: Dynamic.SCOPE_STAFF },
                    { text: qsTr("Single instrument"), value: Dynamic.SCOPE_SINGLE_INSTRUMENT },
                    { text: qsTr("All instruments"), value: Dynamic.SCOPE_ALL_INSTRUMENTS }
                ]

                currentIndex: root.model && !root.model.scopeType.isUndefined ? indexOfValue(root.model.scopeType.value) : -1

                onValueChanged: {
                    root.model.scopeType.value = value
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            Column {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                spacing: 8

                StyledTextLabel {
                    text: qsTr("Velocity")
                }

                IncrementalPropertyControl {
                    id: velocityControl
                    iconMode: iconModeEnum.hidden

                    step: 1
                    decimals: 0
                    maxValue: 127
                    minValue: 0
                    validator: IntInputValidator {
                        top: velocityControl.maxValue
                        bottom: velocityControl.minValue
                    }

                    isIndeterminate: model ? model.velocity.isUndefined : false
                    currentValue: model ? model.velocity.value : 0

                    onValueEdited: { model.velocity.value = newValue }
                }
            }

            Column {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                spacing: 8

                Text {
                    text: qsTr("Velocity change")

                    color: globalStyle.buttonText

                    font {
                        family: globalStyle.font.family
                        pixelSize: globalStyle.font.pixelSize
                    }
                }

                IncrementalPropertyControl {
                    id: velocityChangeControl
                    iconMode: iconModeEnum.hidden

                    enabled: model ? model.velocityChange.isEnabled : false

                    step: 1
                    decimals: 0
                    maxValue: 127
                    minValue: -127
                    validator: IntInputValidator {
                        top: velocityChangeControl.maxValue
                        bottom: velocityChangeControl.minValue
                    }

                    isIndeterminate: model && enabled ? model.velocityChange.isUndefined : false
                    currentValue: model ? model.velocityChange.value : 0

                    onValueEdited: { model.velocityChange.value = newValue }
                }
            }
        }

        Column {
            spacing: 8

            width: parent.width

            StyledTextLabel {
                text: qsTr("Change speed")
            }

            RadioButtonGroup {
                id: radioButtonList

                height: 30
                width: parent.width

                model: [
                    { textRole: "Slow", valueRole: Dynamic.VELOCITY_CHANGE_SPEED_SLOW },
                    { textRole: "Normal", valueRole: Dynamic.VELOCITY_CHANGE_SPEED_NORMAL },
                    { textRole: "Fast", valueRole: Dynamic.VELOCITY_CHANGE_SPEED_FAST }
                ]

                delegate: FlatRadioButton {
                    id: radioButtonDelegate

                    ButtonGroup.group: radioButtonList.radioButtonGroup

                    checked: root.model && !root.model.velocityChangeSpeed.isUndefined ? root.model.velocityChangeSpeed.value === modelData["valueRole"]
                                                                                       : false
                    onToggled: {
                        root.model.velocityChangeSpeed.value = modelData["valueRole"]
                    }

                    StyledTextLabel {
                        text: modelData["textRole"]

                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }
}
