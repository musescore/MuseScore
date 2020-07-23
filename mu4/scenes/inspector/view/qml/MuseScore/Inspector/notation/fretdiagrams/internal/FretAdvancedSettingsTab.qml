import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../../common"

FocusableItem {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTr("Scale")
                propertyItem: root.model ? root.model.scale : null

                IncrementalPropertyControl {
                    id: scaleControl

                    iconMode: iconModeEnum.hidden

                    isIndeterminate: root.model ? root.model.scale.isUndefined : false
                    currentValue: root.model ? root.model.scale.value : 0
                    measureUnitsSymbol: "%"
                    step: 1
                    decimals: 0
                    maxValue: 300
                    minValue: 1
                    validator: IntInputValidator {
                        top: scaleControl.maxValue
                        bottom: scaleControl.minValue
                    }

                    onValueEdited: { root.model.scale.value = newValue }
                }
            }

            InspectorPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                titleText: qsTr("Strings")
                propertyItem: root.model ? root.model.stringsCount : null

                IncrementalPropertyControl {
                    id: stringsCountControl

                    iconMode: iconModeEnum.hidden

                    isIndeterminate: root.model ? root.model.stringsCount.isUndefined : false
                    currentValue: root.model ? root.model.stringsCount.value : 0
                    step: 1
                    decimals: 0
                    maxValue: 12
                    minValue: 4
                    validator: IntInputValidator {
                        top: stringsCountControl.maxValue
                        bottom: stringsCountControl.minValue
                    }

                    onValueEdited: { root.model.stringsCount.value = newValue }
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTr("Visible frets")
                propertyItem: root.model ? root.model.fretsCount : null

                IncrementalPropertyControl {
                    id: visibleFretsCountControl

                    iconMode: iconModeEnum.hidden

                    isIndeterminate: root.model ? root.model.fretsCount.isUndefined : false
                    currentValue: root.model ? root.model.fretsCount.value : 0
                    step: 1
                    decimals: 0
                    maxValue: 6
                    minValue: 3
                    validator: IntInputValidator {
                        top: visibleFretsCountControl.maxValue
                        bottom: visibleFretsCountControl.minValue
                    }

                    onValueEdited: { root.model.fretsCount.value = newValue }
                }
            }

            InspectorPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                titleText: qsTr("Starting fret number")
                propertyItem: root.model ? root.model : null

                IncrementalPropertyControl {
                    id: startingFretNumberControl

                    iconMode: iconModeEnum.hidden

                    isIndeterminate: root.model ? root.model.startingFretNumber.isUndefined : false
                    currentValue: root.model ? root.model.startingFretNumber.value : 0
                    step: 1
                    decimals: 0
                    maxValue: 12
                    minValue: 1
                    validator: IntInputValidator {
                        top: startingFretNumberControl.maxValue
                        bottom: startingFretNumberControl.minValue
                    }

                    onValueEdited: { root.model.startingFretNumber.value = newValue }
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTr("Placement on staff")
            propertyItem: root.model ? root.model.placement : null

            RadioButtonGroup {
                id: positionButtonList

                height: 30
                width: parent.width

                model: [
                    { textRole: qsTr("Above"), valueRole: Hairpin.PLACEMENT_TYPE_ABOVE },
                    { textRole: qsTr("Below"), valueRole: Hairpin.PLACEMENT_TYPE_BELOW }
                ]

                delegate: FlatRadioButton {

                    ButtonGroup.group: positionButtonList.radioButtonGroup

                    checked: root.model && !root.model.placement.isUndefined ? root.model.placement.value === modelData["valueRole"]
                                                                             : false
                    onToggled: {
                        root.model.placement.value = modelData["valueRole"]
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

        CheckBox {
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            isIndeterminate: root.model ? root.model.isNutVisible.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.isNutVisible.value : false
            text: qsTr("Show nut")

            onClicked: { root.model.isNutVisible.value = !checked }
        }
    }
}

