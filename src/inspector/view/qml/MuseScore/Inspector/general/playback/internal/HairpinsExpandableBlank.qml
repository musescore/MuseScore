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
        height: implicitHeight
        width: parent.width

        spacing: 12

        InspectorPropertyView {
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            titleText: qsTr("Velocity change")
            propertyItem: model ? model.velocityChange : null

            IncrementalPropertyControl {
                id: velocityChangeControl
                iconMode: iconModeEnum.hidden

                step: 1
                decimals: 0
                maxValue: 127
                minValue: 0
                validator: IntInputValidator {
                    top: velocityChangeControl.maxValue
                    bottom: velocityChangeControl.minValue
                }

                isIndeterminate: model ? model.velocityChange.isUndefined : false
                currentValue: model ? model.velocityChange.value : 0

                onValueEdited: { model.velocityChange.value = newValue }
            }
        }

        InspectorPropertyView {
            titleText: qsTr("Changes in dynamics range")
            propertyItem: root.model ? root.model.velocityChangeType : null

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTr("Linear (default)"), value: Hairpin.VELOCITY_EASING_LINEAR },
                    { text: qsTr("Exponential"), value: Hairpin.VELOCITY_EASING_EXPONENTIAL },
                    { text: qsTr("Ease-in"), value: Hairpin.VELOCITY_EASING_IN },
                    { text: qsTr("Ease-out"), value: Hairpin.VELOCITY_EASING_OUT },
                    { text: qsTr("Ease-in and out"), value: Hairpin.VELOCITY_EASING_IN_OUT }
                ]

                currentIndex: root.model && !root.model.velocityChangeType.isUndefined ? indexOfValue(root.model.velocityChangeType.value) : -1

                onValueChanged: {
                    root.model.velocityChangeType.value = value
                }
            }
        }
    }
}

