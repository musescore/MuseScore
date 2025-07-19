/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

StyledFlickable {
    id: root

    signal goToTextStylePage(int index)

    contentWidth: column.width
    contentHeight: column.height

    MeasureNumbersPageModel {
        id: barNumbersModel
    }

    ColumnLayout {
        id: column
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/voltas", "Measure numbers")

            ColumnLayout {
                width: parent.width
                spacing: 12

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyleToggle {
                        styleItem: barNumbersModel.showMeasureNumber
                        text: qsTrc("notation/editstyle/voltas", "Show measure numbers")
                    }

                    CheckBox {
                        enabled: barNumbersModel.showMeasureNumber.value === true
                        checked: barNumbersModel.showMeasureNumberOne.value === true
                        onClicked: barNumbersModel.showMeasureNumberOne.value = !barNumbersModel.showMeasureNumberOne.value
                        text: qsTrc("notation/editstyle/voltas", "Show initial measure number")
                    }
                }

                ColumnLayout {
                    enabled: barNumbersModel.showMeasureNumber.value === true
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Text style")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RowLayout {
                        spacing: 4

                        ComboBoxDropdown {
                            id: barNumTextStyleDropdown
                            Layout.preferredWidth: 190
                            model: barNumbersModel.textStyles
                            styleItem: barNumbersModel.measureNumberTextStyle
                            onHandleItem: function(value) {
                                barNumbersModel.measureNumberTextStyle.value = value
                            }
                        }

                        FlatButton {
                            text: qsTrc("notation", "Edit text style")

                            onClicked: {
                                root.goToTextStylePage(barNumTextStyleDropdown.currentIndex)
                            }
                        }

                        StyleResetButton {
                            styleItem: barNumbersModel.measureNumberTextStyle
                        }
                    }
                }
            }
        }

        StyledGroupBox {
            enabled: barNumbersModel.showMeasureNumber.value === true
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/voltas", "Frequency")

            ColumnLayout {
                width: parent.width
                spacing: 8

                RoundedRadioButton {
                    checked: barNumbersModel.measureNumberSystem.value === true
                    onClicked: barNumbersModel.measureNumberSystem.value = true
                    text: qsTrc("notation/editstyle/voltas", "Start of each system")
                }

                RowLayout {
                    spacing: 12

                    RoundedRadioButton {
                        checked: barNumbersModel.measureNumberSystem.value === false
                        onClicked: barNumbersModel.measureNumberSystem.value = false
                        text: qsTrc("notation/editstyle/voltas", "Every")
                    }

                    IncrementalPropertyControl {
                        Layout.preferredWidth: 65
                        currentValue: barNumbersModel.measureNumberInterval.value

                        minValue: 1
                        maxValue: 99
                        step: 1
                        decimals: 0

                        onValueEdited: function(newValue) {
                            barNumbersModel.measureNumberInterval.value = newValue
                        }
                    }

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "measures")
                        horizontalAlignment: Text.AlignLeft
                    }
                }
            }
        }

        StyledGroupBox {
            enabled: barNumbersModel.showMeasureNumber.value === true
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/voltas", "Position")

            ColumnLayout {
                width:parent.width
                spacing: 12

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Align")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RoundedRadioButton {
                        checked: barNumbersModel.measureNumberAlignToBarline.value === true
                        onClicked: barNumbersModel.measureNumberAlignToBarline.value = true
                        text: qsTrc("notation/editstyle/voltas", "To the barline")
                    }

                    RoundedRadioButton {
                        checked: barNumbersModel.measureNumberAlignToBarline.value === false
                        onClicked: barNumbersModel.measureNumberAlignToBarline.value = false
                        text: qsTrc("notation/editstyle/voltas", "To the measure")
                    }

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Alignment")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RadioButtonGroup {
                        model: [
                            { iconCode: IconCode.ALIGN_LEFT, value: 0 },
                            { iconCode: IconCode.ALIGN_HORIZONTAL_CENTER, value: 2 },
                            { iconCode: IconCode.ALIGN_RIGHT, value: 1 }
                        ]

                        delegate: FlatRadioButton {
                            width: ui.theme.defaultButtonSize
                            height: ui.theme.defaultButtonSize
                            transparent: true
                            iconCode: modelData.iconCode
                            iconFontSize: 16
                            checked: barNumbersModel.measureNumberHPlacement.value === modelData.value
                            onToggled: barNumbersModel.measureNumberHPlacement.value = modelData.value
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Show")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RoundedRadioButton {
                        checked: barNumbersModel.measureNumberAllStaves.value === false
                        onClicked: barNumbersModel.measureNumberAllStaves.value = false
                        text: qsTrc("notation/editstyle/voltas", "At system marking positions")
                    }

                    RoundedRadioButton {
                        checked: barNumbersModel.measureNumberAllStaves.value === true
                        onClicked: barNumbersModel.measureNumberAllStaves.value = true
                        text: qsTrc("notation/editstyle/voltas", "On all staves")
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Position")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RadioButtonGroup {
                        model: [
                            { text: qsTrc("notation/editstyle/voltas", "Above"), value: 0},
                            { text: qsTrc("notation/editstyle/voltas", "Below"), value: 1},
                        ]

                        delegate: FlatRadioButton {
                            width: 176
                            height: ui.theme.defaultButtonSize
                            text: modelData.text
                            checked: barNumbersModel.measureNumberVPlacement.value === modelData.value
                            onToggled: barNumbersModel.measureNumberVPlacement.value = modelData.value
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Offset")
                        horizontalAlignment: Text.AlignLeft
                    }

                    StyledXYControllerWithReset {
                        styleItem: barNumbersModel.measureNumberVPlacement.value === 0
                                   ? barNumbersModel.measureNumberPosAbove
                                   : barNumbersModel.measureNumberPosBelow
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/voltas", "Measure number range at multimeasure rests")

            ColumnLayout {
                width: parent.width
                spacing: 12

                StyleToggle {
                    styleItem: barNumbersModel.mmRestShowMeasureNumberRange
                    text: qsTrc("notation/editstyle/voltas", "Show measure number ranges")
                }

                ColumnLayout {
                    enabled: barNumbersModel.mmRestShowMeasureNumberRange.value === true
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Text style")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RowLayout {                       
                        ComboBoxDropdown {
                            id: mmRestBarRangeTextStyleDropdown
                            Layout.preferredWidth: 190
                            model: barNumbersModel.textStyles
                            styleItem: barNumbersModel.mmRestRangeTextStyle
                            onHandleItem: function(value) {
                                barNumbersModel.mmRestRangeTextStyle.value = value
                            }
                        }

                        FlatButton {
                            text: qsTrc("notation", "Edit text style")

                            onClicked: {
                                root.goToTextStylePage(mmRestBarRangeTextStyleDropdown.currentIndex)
                            }
                        }

                        StyleResetButton {
                            styleItem: barNumbersModel.mmRestRangeTextStyle
                        }
                    }
                }

                ColumnLayout {
                    enabled: barNumbersModel.mmRestShowMeasureNumberRange.value === true
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Bracket type")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RadioButtonGroup {
                        model: [
                            { text: qsTrc("notation/editstyle/voltas", "Brackets"), value: 0},
                            { text: qsTrc("notation/editstyle/voltas", "Parenthesis"), value: 1},
                            { text: qsTrc("notation/editstyle/voltas", "None"), value: 2},
                        ]

                        delegate: FlatRadioButton {
                            width: 116
                            height: ui.theme.defaultButtonSize
                            text: modelData.text
                            checked: barNumbersModel.mmRestRangeBracketType.value === modelData.value
                            onToggled: barNumbersModel.mmRestRangeBracketType.value = modelData.value
                        }
                    }
                }

                ColumnLayout {
                    enabled: barNumbersModel.mmRestShowMeasureNumberRange.value === true
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Alignment")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RadioButtonGroup {
                        model: [
                            { iconCode: IconCode.ALIGN_LEFT, value: 0 },
                            { iconCode: IconCode.ALIGN_HORIZONTAL_CENTER, value: 2 },
                            { iconCode: IconCode.ALIGN_RIGHT, value: 1 }
                        ]

                        delegate: FlatRadioButton {
                            width: ui.theme.defaultButtonSize
                            height: ui.theme.defaultButtonSize
                            transparent: true
                            iconCode: modelData.iconCode
                            iconFontSize: 16
                            checked: barNumbersModel.mmRestRangeHPlacement.value === modelData.value
                            onToggled: barNumbersModel.mmRestRangeHPlacement.value = modelData.value
                        }
                    }
                }

                ColumnLayout {
                    enabled: barNumbersModel.mmRestShowMeasureNumberRange.value === true
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Position")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RadioButtonGroup {
                        model: [
                            { text: qsTrc("notation/editstyle/voltas", "Above"), value: 0},
                            { text: qsTrc("notation/editstyle/voltas", "Below"), value: 1},
                        ]

                        delegate: FlatRadioButton {
                            width: 176
                            height: ui.theme.defaultButtonSize
                            text: modelData.text
                            checked: barNumbersModel.mmRestRangeVPlacement.value === modelData.value
                            onToggled: barNumbersModel.mmRestRangeVPlacement.value = modelData.value
                        }
                    }
                }

                ColumnLayout {
                    enabled: barNumbersModel.mmRestShowMeasureNumberRange.value === true
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Offset")
                        horizontalAlignment: Text.AlignLeft
                    }

                    StyledXYControllerWithReset {
                        styleItem: barNumbersModel.mmRestRangeVPlacement.value === 0
                                   ? barNumbersModel.mmRestRangePosAbove
                                   : barNumbersModel.mmRestRangePosBelow
                    }
                }
            }
        }
    }
}
