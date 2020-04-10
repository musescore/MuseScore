import QtQuick 2.9
import MuseScore.Inspectors 3.3
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

        Column {
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            height: implicitHeight

            spacing: 8

            StyledTextLabel {
                text: qsTr("Velocity change")
            }

            IncrementalPropertyControl {
                iconMode: iconModeEnum.hidden

                step: 1
                decimals: 0
                isIndeterminate: model ? model.velocityChange.isUndefined : false
                currentValue: model ? model.velocityChange.value : 0

                onValueEdited: { model.velocityChange.value = newValue }
            }
        }

        Column {
            height: implicitHeight
            width: parent.width

            spacing: 8

            StyledTextLabel {
                text: qsTr("Changes in dynamics range")
            }

            StyledComboBox {
                width: parent.width

                textRole: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTr("Linear (default)"), value: Hairpin.VELOCITY_EASING_LINEAR },
                    { text: qsTr("Exponential"), value: Hairpin.VELOCITY_EASING_EXPONENTIAL },
                    { text: qsTr("Ease-in"), value: Hairpin.VELOCITY_EASING_IN },
                    { text: qsTr("Ease-out"), value: Hairpin.VELOCITY_EASING_OUT },
                    { text: qsTr("Ease-in and out"), value: Hairpin.VELOCITY_EASING_IN_OUT }
                ]

                currentIndex: root.model && !root.model.velocityChangeType.isUndefined ? indexOfValue(root.model.velocityChangeType.value) : -1

                onCurrentValueChanged: {
                    root.model.velocityChangeType.value = value
                }
            }
        }
    }
}

