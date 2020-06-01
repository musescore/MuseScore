import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspectors 3.3
import "../../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        StyledTextLabel {
            text: qsTr("Line type")
        }

        RadioButtonGroup {
            id: lineTypeButtonList

            height: 30
            width: parent.width

            model: [
                { iconRole: IconNameTypes.LINE_NORMAL, typeRole: PedalTypes.HOOK_TYPE_NONE },
                { iconRole: IconNameTypes.LINE_WITH_END_HOOK, typeRole: PedalTypes.HOOK_TYPE_RIGHT_ANGLE },
                { iconRole: IconNameTypes.LINE_WITH_ANGLED_END_HOOK, typeRole: PedalTypes.HOOK_TYPE_ACUTE_ANGLE },
                { iconRole: IconNameTypes.LINE_PEDAL_STAR_ENDING, typeRole: PedalTypes.HOOK_TYPE_STAR }
            ]

            delegate: FlatRadioButton {

                ButtonGroup.group: lineTypeButtonList.radioButtonGroup

                checked: root.model && !root.model.hookType.isUndefined ? root.model.hookType.value === modelData["typeRole"]
                                                                        : false

                onToggled: {
                    root.model.hookType.value = modelData["typeRole"]
                }

                StyledIconLabel {
                    iconCode: modelData["iconRole"]
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
                    text: qsTr("Thickness")
                }

                IncrementalPropertyControl {
                    id: thicknessControl
                    iconMode: iconModeEnum.hidden

                    isIndeterminate: root.model ? root.model.thickness.isUndefined : false
                    currentValue: root.model ? root.model.thickness.value : 0

                    onValueEdited: { root.model.thickness.value = newValue }
                }
            }

            Column {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                spacing: 8

                StyledTextLabel {
                    text: qsTr("Hook height")
                }

                IncrementalPropertyControl {
                    id: hookHeightControl

                    iconMode: iconModeEnum.hidden

                    enabled: root.model ? root.model.hookHeight.isEnabled : false
                    isIndeterminate: root.model && enabled ? root.model.hookHeight.isUndefined : false
                    currentValue: root.model ? root.model.hookHeight.value : 0

                    onValueEdited: { root.model.hookHeight.value = newValue }
                }
            }
        }

        CheckBox {
            id: showBothSideHookCheckbox

            /*isIndeterminate: model ? model.isDefaultTempoForced.isUndefined : false
            checked: model && !isIndeterminate ? model.isDefaultTempoForced.value : false*/
            text: qsTr("Show hook on both ends")

            //onClicked: { model.isDefaultTempoForced.value = !checked }
        }

        SeparatorLine { anchors.margins: -10 }

        RadioButtonGroup {
            id: lineStyleButtonList

            height: 30
            width: parent.width

            model: [
                { iconRole: IconNameTypes.LINE_NORMAL, typeRole: PedalTypes.LINE_STYLE_SOLID },
                { iconRole: IconNameTypes.LINE_DASHED, typeRole: PedalTypes.LINE_STYLE_DASHED },
                { iconRole: IconNameTypes.LINE_DOTTED, typeRole: PedalTypes.LINE_STYLE_DOTTED },
                { iconRole: IconNameTypes.CUSTOM, typeRole: PedalTypes.LINE_STYLE_CUSTOM }
            ]

            delegate: FlatRadioButton {

                ButtonGroup.group: lineStyleButtonList.radioButtonGroup

                checked: root.model && !root.model.lineStyle.isUndefined ? root.model.lineStyle.value === modelData["typeRole"]
                                                                         : false

                onToggled: {
                    root.model.lineStyle.value = modelData["typeRole"]
                }

                StyledIconLabel {
                    iconCode: modelData["iconRole"]
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
                    text: qsTr("Dash")
                }

                IncrementalPropertyControl {
                    id: dashControl
                    iconMode: iconModeEnum.hidden

                    isIndeterminate: root.model ? root.model.dashLineLength.isUndefined : false
                    currentValue: root.model ? root.model.dashLineLength.value : 0

                    onValueEdited: { root.model.dashLineLength.value = newValue }
                }
            }

            Column {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                spacing: 8

                StyledTextLabel {
                    text: qsTr("Gap")
                }

                IncrementalPropertyControl {
                    id: gapControl

                    iconMode: iconModeEnum.hidden

                    enabled: root.model ? root.model.dashGapLength.isEnabled : false
                    isIndeterminate: root.model && enabled ? root.model.dashGapLength.isUndefined : false
                    currentValue: root.model ? root.model.dashGapLength.value : 0

                    onValueEdited: { root.model.dashGapLength.value = newValue }
                }
            }
        }

        RadioButtonGroup {
            id: pedalPositionButtonList

            height: 30
            width: parent.width

            model: [
                { textRole: qsTr("Above"), valueRole: PedalTypes.PLACEMENT_TYPE_ABOVE },
                { textRole: qsTr("Below"), valueRole: PedalTypes.PLACEMENT_TYPE_BELOW }
            ]

            delegate: FlatRadioButton {

                ButtonGroup.group: pedalPositionButtonList.radioButtonGroup

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
}
