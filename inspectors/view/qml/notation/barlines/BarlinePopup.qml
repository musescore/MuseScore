import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspectors 3.3
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

        Column {
            spacing: 8

            width: parent.width

            StyledTextLabel {
                text: qsTr("Style")
            }

            StyledComboBox {
                width: parent.width

                textRoleName: "text"
                valueRoleName: "value"

                model: [
                    { text: qsTr("Normal"), value: BarlineTypes.TYPE_NORMAL },
                    { text: qsTr("Double"), value: BarlineTypes.TYPE_DOUBLE },
                    { text: qsTr("Start repeat"), value: BarlineTypes.TYPE_START_REPEAT },
                    { text: qsTr("End repeat"), value: BarlineTypes.TYPE_END_REPEAT },
                    { text: qsTr("Broken"), value: BarlineTypes.TYPE_DASHED },
                    { text: qsTr("End"), value: BarlineTypes.TYPE_FINAL },
                    { text: qsTr("End start repeat"), value: BarlineTypes.TYPE_END_START_REPEAT },
                    { text: qsTr("Dotted"), value: BarlineTypes.TYPE_DOTTED },
                ]

                currentIndex: root.barlineSettingsModel && !root.barlineSettingsModel.type.isUndefined ? indexOfValue(root.barlineSettingsModel.type.value) : -1

                onValueChanged: {
                    root.barlineSettingsModel.type.value = value
                }
            }
        }

        Column {
            spacing: 8

            width: parent.width

            StyledTextLabel {
                text: qsTr("Repeat style")
            }

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

            Column {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                spacing: 8

                StyledTextLabel {
                    text: qsTr("Span from")
                }

                IncrementalPropertyControl {
                    id: spanFromControl
                    iconMode: iconModeEnum.hidden

                    isIndeterminate: barlineSettingsModel ? barlineSettingsModel.spanFrom.isUndefined : false
                    currentValue: barlineSettingsModel ? barlineSettingsModel.spanFrom.value : 0

                    onValueEdited: { barlineSettingsModel.spanFrom.value = newValue }
                }
            }

            Column {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                spacing: 8

                StyledTextLabel {
                    text: qsTr("Span to")
                }

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
