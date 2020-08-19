import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

StyledPopup {
    id: root

    property QtObject barlineSettingsModel: null
    property QtObject staffSettingsModel: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 16

        InspectorPropertyView {
            titleText: qsTr("Style")
            propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.type : null

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTranslate("symUserNames", "Single barline"), value: BarlineTypes.TYPE_NORMAL },
                    { text: qsTranslate("symUserNames", "Double barline"), value: BarlineTypes.TYPE_DOUBLE },
                    { text: qsTranslate("symUserNames", "Left (start) repeat sign"), value: BarlineTypes.TYPE_START_REPEAT },
                    { text: qsTranslate("symUserNames", "Right (end) repeat sign"), value: BarlineTypes.TYPE_END_REPEAT },
                    { text: qsTranslate("symUserNames", "Right and left repeat sign"), value: BarlineTypes.TYPE_END_START_REPEAT },
                    { text: qsTranslate("symUserNames", "Dashed barline"), value: BarlineTypes.TYPE_DASHED },
                    { text: qsTranslate("symUserNames", "Final barline"), value: BarlineTypes.TYPE_FINAL },
                    { text: qsTranslate("symUserNames", "Dotted barline"), value: BarlineTypes.TYPE_DOTTED },
                    { text: qsTranslate("symUserNames", "Reverse final barline"), value: BarlineTypes.TYPE_REVERSE_END },
                    { text: qsTranslate("symUserNames", "Heavy barline"), value: BarlineTypes.TYPE_HEAVY },
                    { text: qsTranslate("symUserNames", "Heavy double barline"), value: BarlineTypes.TYPE_DOUBLE_HEAVY },
                ]

                currentIndex: root.barlineSettingsModel && !root.barlineSettingsModel.type.isUndefined ? indexOfValue(root.barlineSettingsModel.type.value) : -1

                onValueChanged: {
                    root.barlineSettingsModel.type.value = value
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTr("Repeat style")
            propertyItem: barlineSettingsModel ? barlineSettingsModel.hasToShowTips : null

            RadioButtonGroup {
                id: repeatStyle

                height: 30
                width: parent.width

                model: [
                    { iconRole: IconCode.BARLINE_UNWINGED, valueRole: false },
                    { iconRole: IconCode.BARLINE_WINGED, valueRole: true }
                ]

                delegate: FlatRadioButton {

                    ButtonGroup.group: repeatStyle.radioButtonGroup

                    checked: barlineSettingsModel && !barlineSettingsModel.hasToShowTips.isUndefined ? barlineSettingsModel.hasToShowTips.value === modelData["valueRole"]
                                                                                                     : false

                    onToggled: {
                        barlineSettingsModel.hasToShowTips.value = modelData["valueRole"]
                    }

                    StyledIconLabel {
                        iconCode: modelData["iconRole"]
                    }
                }
            }
        }

        CheckBox {
            id: spanToNextStaffCheckBox

            isIndeterminate: barlineSettingsModel ? barlineSettingsModel.isSpanToNextStaff.isUndefined : false
            checked: barlineSettingsModel && !isIndeterminate ? barlineSettingsModel.isSpanToNextStaff.value : false
            text: qsTr("Span to next staff")

            onClicked: { barlineSettingsModel.isSpanToNextStaff.value = !checked }
        }

        SeparatorLine {
            anchors.margins: -10
        }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTr("Span from")
                propertyItem: barlineSettingsModel ? barlineSettingsModel.spanFrom : null

                IncrementalPropertyControl {
                    id: spanFromControl
                    iconMode: iconModeEnum.hidden

                    isIndeterminate: barlineSettingsModel ? barlineSettingsModel.spanFrom.isUndefined : false
                    currentValue: barlineSettingsModel ? barlineSettingsModel.spanFrom.value : 0

                    onValueEdited: { barlineSettingsModel.spanFrom.value = newValue }
                }
            }

            InspectorPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                titleText: qsTr("Span to")
                propertyItem: barlineSettingsModel ? barlineSettingsModel.spanTo : null

                IncrementalPropertyControl {
                    id: spanToControl

                    iconMode: iconModeEnum.hidden

                    enabled: barlineSettingsModel ? barlineSettingsModel.spanTo.isEnabled : false

                    isIndeterminate: barlineSettingsModel && enabled ? barlineSettingsModel.spanTo.isUndefined : false
                    currentValue: barlineSettingsModel ? barlineSettingsModel.spanTo.value : 0

                    onValueEdited: { barlineSettingsModel.spanTo.value = newValue }
                }
            }
        }

        FlatButton {
            width: parent.width

            text: qsTr("Apply to all staffs")

            enabled: !spanFromControl.isIndeterminate && !spanToControl.isIndeterminate

            onClicked: {
                if (!staffSettingsModel)
                    return

                staffSettingsModel.barlinesSpanFrom.value = spanFromControl.currentValue
                staffSettingsModel.barlinesSpanTo.value = spanToControl.currentValue
            }
        }

        Column {
            id: presetButtons

            spacing: 8

            width: parent.width

            StyledTextLabel {
                text: qsTr("Span presets")
            }

            RowLayout {
                width: parent.width

                FlatButton {
                    text: qsTr("Default")
                    Layout.fillWidth: true
                    onClicked: { barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_DEFAULT) }
                }

                FlatButton {
                    text: qsTr("Tick 1")
                    Layout.fillWidth: true
                    onClicked: { barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_TICK_1) }
                }


                FlatButton {
                    text: qsTr("Tick 2")
                    Layout.fillWidth: true
                    onClicked: { barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_TICK_2) }
                }
            }

            RowLayout {
                width: parent.width

                FlatButton {
                    text: qsTr("Short 1")
                    Layout.fillWidth: true
                    onClicked: { barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_SHORT_1) }
                }

                FlatButton {
                    text: qsTr("Short 2")
                    Layout.fillWidth: true
                    onClicked: { barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_SHORT_2) }
                }
            }
        }
    }
}
