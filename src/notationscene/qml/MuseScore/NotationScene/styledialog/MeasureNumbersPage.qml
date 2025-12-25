/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

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
                            Layout.minimumWidth: 192
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
                            required iconCode
                            required property int value

                            width: ui.theme.defaultButtonSize
                            height: ui.theme.defaultButtonSize

                            transparent: true
                            iconFontSize: 16

                            checked: barNumbersModel.measureNumberHPlacement.value === value
                            onToggled: barNumbersModel.measureNumberHPlacement.value = value
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

                    RadioButtonGroup {
                        orientation: ListView.Vertical
                        spacing: 8

                        model:[
                            { text: qsTrc("notation/editstyle/voltas", "Above system"), value: 0 },
                            { text: qsTrc("notation/editstyle/voltas", "Below system"), value: 1 },
                            { text: qsTrc("notation/editstyle/voltas", "At system marking positions"), value: 2 },
                            { text: qsTrc("notation/editstyle/voltas", "On all staves"), value: 3 },
                        ]

                        delegate: RoundedRadioButton {
                            required text
                            required property int value

                            checked: barNumbersModel.measureNumberPlacementMode.value === value
                            onClicked: barNumbersModel.measureNumberPlacementMode.value = value
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8
                    visible: barNumbersModel.measureNumberPlacementMode.value === 3

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
                            required text
                            required property int value

                            width: 176
                            height: ui.theme.defaultButtonSize

                            checked: barNumbersModel.measureNumberVPlacement.value === value
                            onToggled: barNumbersModel.measureNumberVPlacement.value = value
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    RowLayout {
                        spacing: 6

                        StyledTextLabel {
                            Layout.preferredWidth: 156
                            text: qsTrc("notation/editstyle/voltas", "Offset when above staff:")
                            horizontalAlignment: Text.AlignLeft
                            wrapMode: Text.Wrap
                        }

                        StyledXYControllerWithReset {
                            styleItem: barNumbersModel.measureNumberPosAbove
                        }
                    }

                    RowLayout {
                        spacing: 6

                        StyledTextLabel {
                            Layout.preferredWidth: 156
                            text: qsTrc("notation/editstyle/voltas", "Offset when below staff:")
                            horizontalAlignment: Text.AlignLeft
                            wrapMode: Text.Wrap
                        }

                        StyledXYControllerWithReset {
                            styleItem: barNumbersModel.measureNumberPosBelow
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
                        spacing: 4

                        ComboBoxDropdown {
                            id: mmRestBarRangeTextStyleDropdown
                            Layout.minimumWidth: 192
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
                        width: 362
                        model: [
                            { text: qsTrc("notation/editstyle/voltas", "Brackets"), value: 0},
                            { text: qsTrc("notation/editstyle/voltas", "Parentheses"), value: 1},
                            { text: qsTrc("notation/editstyle/voltas", "None"), value: 2},
                        ]

                        delegate: FlatRadioButton {
                            required text
                            required property int value

                            height: ui.theme.defaultButtonSize
                            
                            checked: barNumbersModel.mmRestRangeBracketType.value === value
                            onToggled: barNumbersModel.mmRestRangeBracketType.value = value
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
                            required iconCode
                            required property int value

                            width: ui.theme.defaultButtonSize
                            height: ui.theme.defaultButtonSize

                            transparent: true
                            iconFontSize: 16

                            checked: barNumbersModel.mmRestRangeHPlacement.value === value
                            onToggled: barNumbersModel.mmRestRangeHPlacement.value = value
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
                        width: 362
                        model: [
                            { text: qsTrc("notation/editstyle/voltas", "Above"), value: 0},
                            { text: qsTrc("notation/editstyle/voltas", "Below"), value: 1},
                        ]

                        delegate: FlatRadioButton {
                            required text
                            required property int value

                            height: ui.theme.defaultButtonSize
                            
                            checked: barNumbersModel.mmRestRangeVPlacement.value === value
                            onToggled: barNumbersModel.mmRestRangeVPlacement.value = value
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
