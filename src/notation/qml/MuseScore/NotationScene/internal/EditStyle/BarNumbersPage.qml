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

    signal goToTextStylePage(string s)

    contentWidth: column.width
    contentHeight: column.height

    BarNumbersPageModel {
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

                    RowLayout {
                        ToggleButton {
                            checked: barNumbersModel.showMeasureNumber.value === true
                            onToggled: {
                                barNumbersModel.showMeasureNumber.value = !barNumbersModel.showMeasureNumber.value
                            }
                        }

                        StyledTextLabel {
                            text: qsTrc("notation/editstyle/voltas", "Show measure numbers")
                            horizontalAlignment: Text.AlignLeft
                        }
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
                        StyledDropdown {
                            Layout.preferredWidth: 190
                            model: barNumbersModel.textStyles
                            currentIndex: indexOfValue(barNumbersModel.measureNumberTextStyle.value)
                            onActivated: function(index, value) {
                                barNumbersModel.measureNumberTextStyle.value = value
                            }
                        }

                        FlatButton {
                            text: qsTrc("notation", "Edit text style")

                            onClicked: {
                                root.goToTextStylePage("measure-number")
                            }
                        }

                        FlatButton {
                            icon: IconCode.UNDO
                            enabled: !barNumbersModel.measureNumberTextStyle.isDefault
                            onClicked: barNumbersModel.measureNumberTextStyle.value = barNumbersModel.measureNumberTextStyle.defaultValue
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
                        text: qsTrc("notation/editstyle/voltas", "Alignment")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RadioButtonGroup {
                        model: [
                            { iconCode: IconCode.ALIGN_LEFT, value: 0},
                            { iconCode: IconCode.ALIGN_HORIZONTAL_CENTER, value: 1},
                            { iconCode: IconCode.ALIGN_RIGHT, value: 2 }
                        ]

                        delegate: FlatRadioButton {
                            width: 30
                            height: 30
                            transparent: true
                            iconCode: modelData.iconCode
                            iconFontSize: 16
                            checked: barNumbersModel.measureNumberSystem.value === true
                                     ? barNumbersModel.measureNumberHPlacement.value === modelData.value
                                     : barNumbersModel.measureNumberHPlacementInterval.value === modelData.value
                            onToggled: barNumbersModel.measureNumberSystem.value === true
                                       ? barNumbersModel.measureNumberHPlacement.value = modelData.value
                                       : barNumbersModel.measureNumberHPlacementInterval.value = modelData.value
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
                        checked: barNumbersModel.measureNumberAllStaves.value === false
                        onClicked: barNumbersModel.measureNumberAllStaves.value = false
                        text: qsTrc("notation/editstyle/voltas", "On all staves")
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Position above:")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RowLayout {
                        IncrementalPropertyControl {
                            Layout.preferredWidth: 100
                            currentValue: barNumbersModel.measureNumberPosAbove.value.y
                            measureUnitsSymbol: qsTrc("global", "sp")
                            maxValue: 0
                            minValue: -100
                            decimals: 2
                            step: 0.25

                            onValueEdited: function(newValue) {
                                barNumbersModel.measureNumberPosAbove.value.y = newValue
                            }
                        }

                        FlatButton {
                            icon: IconCode.UNDO
                            enabled: !barNumbersModel.measureNumberPosAbove.isDefault
                            onClicked: barNumbersModel.measureNumberPosAbove.value = barNumbersModel.measureNumberPosAbove.defaultValue
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Position below:")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RowLayout {
                        IncrementalPropertyControl {
                            Layout.preferredWidth: 100
                            currentValue: barNumbersModel.measureNumberPosBelow.value.y
                            measureUnitsSymbol: qsTrc("global", "sp")
                            maxValue: 100
                            minValue: 0
                            decimals: 2
                            step: 0.25

                            onValueEdited: function(newValue) {
                                barNumbersModel.measureNumberPosBelow.value.y = newValue
                            }
                        }

                        FlatButton {
                            icon: IconCode.UNDO
                            enabled: !barNumbersModel.measureNumberPosBelow.isDefault
                            onClicked: barNumbersModel.measureNumberPosBelow.value = barNumbersModel.measureNumberPosBelow.defaultValue
                        }
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

                RowLayout {
                    ToggleButton {
                        checked: barNumbersModel.mmRestShowMeasureNumberRange.value === true
                        onToggled: {
                            barNumbersModel.mmRestShowMeasureNumberRange.value = !barNumbersModel.mmRestShowMeasureNumberRange.value
                        }
                    }

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Show measure number ranges")
                        horizontalAlignment: Text.AlignLeft
                    }
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
                        StyledDropdown {
                            Layout.preferredWidth: 190
                            model: barNumbersModel.textStyles
                            currentIndex: indexOfValue(barNumbersModel.mmRestRangeTextStyle.value)
                            onActivated: function(index, value) {
                                barNumbersModel.mmRestRangeTextStyle.value = value
                            }
                        }

                        FlatButton {
                            text: qsTrc("notation", "Edit text style")

                            onClicked: {
                                root.goToTextStylePage("multimeasure-rest-range")
                            }
                        }

                        FlatButton {
                            icon: IconCode.UNDO
                            enabled: !barNumbersModel.mmRestRangeTextStyle.isDefault
                            onClicked: barNumbersModel.mmRestRangeTextStyle.value = barNumbersModel.mmRestRangeTextStyle.defaultValue
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
                            height: 30
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
                            { iconCode: IconCode.ALIGN_LEFT, value: 0},
                            { iconCode: IconCode.ALIGN_HORIZONTAL_CENTER, value: 1},
                            { iconCode: IconCode.ALIGN_RIGHT, value: 2 }
                        ]

                        delegate: FlatRadioButton {
                            width: 30
                            height: 30
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
                            height: 30
                            text: modelData.text
                            checked: barNumbersModel.mmRestRangeVPlacement.value === modelData.value
                            onToggled: barNumbersModel.mmRestRangeVPlacement.value = modelData.value
                        }
                    }
                }

                ColumnLayout {
                    id: offsetControl
                    enabled: barNumbersModel.mmRestShowMeasureNumberRange.value === true
                    width: parent.width
                    spacing: 8

                    property StyleItem offsetProperty: barNumbersModel.mmRestRangeVPlacement.value === 0
                                                       ? barNumbersModel.mmRestRangePosAbove
                                                       : barNumbersModel.mmRestRangePosBelow

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Offset")
                        horizontalAlignment: Text.AlignLeft
                    }

                    RowLayout {
                        spacing: 6

                        IncrementalPropertyControl {
                            Layout.preferredWidth: 100
                            decimals: 2
                            measureUnitsSymbol: 'sp'
                            prefixIcon: IconCode.HORIZONTAL
                            minValue: -100
                            maxValue: 100
                            step: 0.1

                            currentValue: offsetControl.offsetProperty.value.x
                            onValueEdited: function(newValue) {
                                offsetControl.offsetProperty.value.x = newValue
                            }
                        }

                        IncrementalPropertyControl {
                            Layout.preferredWidth: 100
                            decimals: 2
                            measureUnitsSymbol: 'sp'
                            prefixIcon: IconCode.VERTICAL
                            minValue: -100
                            maxValue: 100
                            step: 0.1

                            currentValue: offsetControl.offsetProperty.value.y
                            onValueEdited: function(newValue) {
                                offsetControl.offsetProperty.value.y = newValue
                            }
                        }

                        FlatButton {
                            icon: IconCode.UNDO
                            enabled: !offsetControl.offsetProperty.isDefault
                            onClicked: offsetControl.offsetProperty.value = offsetControl.offsetProperty.value
                        }
                    }
                }
            }
        }
    }
}
