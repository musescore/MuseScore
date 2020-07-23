import QtQuick 2.9
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: Column {
        spacing: 16

        height: implicitHeight
        width: root.width

        Item {
            height: childrenRect.height
            width: root.width

            InspectorPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTr("Velocity")
                propertyItem: model ? model.velocity : null

                IncrementalPropertyControl {
                    id: velocityControl
                    iconMode: iconModeEnum.hidden

                    step: 1
                    decimals: 0
                    maxValue: 127
                    minValue: -127
                    validator: IntInputValidator {
                        top: velocityControl.maxValue
                        bottom: velocityControl.minValue
                    }

                    isIndeterminate: model ? model.velocity.isUndefined : false
                    currentValue: model ? model.velocity.value : 0

                    onValueEdited: { model.velocity.value = newValue }
                }
            }

            InspectorPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                titleText: qsTr("Tunings (cents)")
                propertyItem: model ? model.tuning : null

                IncrementalPropertyControl {
                    iconMode: iconModeEnum.hidden

                    isIndeterminate: model ? model.tuning.isUndefined : false
                    currentValue: model ? model.tuning.value : 0

                    onValueEdited: { model.tuning.value = newValue }
                }
            }
        }

        CheckBox {
            text: qsTr("Override dynamics")

            isIndeterminate: model ? model.overrideDynamics.isUndefined : false
            checked: model && !isIndeterminate ? model.overrideDynamics.value : false

            onClicked: { model.overrideDynamics.value = !checked }
        }
    }
}
