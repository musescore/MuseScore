/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
import QtQuick.Controls 2.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Rectangle {
    id: root
    anchors.fill: parent;
    color: ui.theme.backgroundPrimaryColor

    RepeatPlayCountTextModel {
        id: repeatPlayCountTextModel
    }

    MeasureRepeatModel {
        id: measureRepeatModel
    }

    signal goToTextStylePage(string s)

    ColumnLayout {
        width: parent.width
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true

            title: qsTrc("notation", "Measure repeats")

            ColumnLayout {
                width: parent.width
                spacing: 12

                StyleSpinboxWithReset {
                    styleItem: measureRepeatModel.measureRepeatNumberPos
                    label: qsTrc("notation", "Number position:")
                    suffix: qsTrc("global", "sp")
                    min: -99

                    step: 0.5

                    controlAreaWidth: 130
                }

                CheckBox {
                    text: qsTrc("notation", "Show ‘1’ on 1-measure repeats")
                    checked: measureRepeatModel.oneMeasureRepeatShow1.value === true
                    onClicked: measureRepeatModel.oneMeasureRepeatShow1.value = !measureRepeatModel.oneMeasureRepeatShow1.value
                }

                CheckBox {
                    text: qsTrc("notation", "Show extenders on 4-measure repeats")
                    checked: measureRepeatModel.fourMeasureRepeatShowExtenders.value === true
                    onClicked: measureRepeatModel.fourMeasureRepeatShowExtenders.value = !measureRepeatModel.fourMeasureRepeatShowExtenders.value
                }

                CheckBox {
                    text: qsTrc("notation", "Number consecutive measure repeats")
                    checked: measureRepeatModel.mrNumberSeries.value === true
                    onClicked: measureRepeatModel.mrNumberSeries.value = !measureRepeatModel.mrNumberSeries.value
                }

                StyledGroupBox {
                    // Layout.fillWidth: true
                    enabled: measureRepeatModel.mrNumberSeries.value === true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 12

                        RowLayout {
                            spacing: 12

                            StyledTextLabel {
                                Layout.alignment: Qt.AlignVCenter
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                text: qsTrc("notation", "Every")
                            }

                            IncrementalPropertyControl {
                                Layout.alignment: Qt.AlignVCenter
                                currentValue: measureRepeatModel.mrNumberEveryXMeasures.value
                                minValue: 0
                                maxValue: 99
                                step: 1
                                decimals: 0
                                Layout.preferredWidth: 50

                                onValueEdited: function(newValue) {
                                    measureRepeatModel.mrNumberEveryXMeasures.value = newValue
                                }
                            }

                            StyledTextLabel {
                                Layout.alignment: Qt.AlignVCenter
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                text: qsTrc("notation", "measures")
                            }

                            FlatButton {
                                icon: IconCode.UNDO
                                enabled: !measureRepeatModel.mrNumberEveryXMeasures.isDefault
                                onClicked: measureRepeatModel.mrNumberEveryXMeasures.value = measureRepeatModel.mrNumberEveryXMeasures.defaultValue
                            }
                        }

                        CheckBox {
                            text: qsTrc("notation", "With parentheses")
                            checked: measureRepeatModel.mrNumberSeriesWithParentheses.value === true
                            onClicked: measureRepeatModel.mrNumberSeriesWithParentheses.value = !measureRepeatModel.mrNumberSeriesWithParentheses.value
                        }
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true

            title: qsTrc("notation", "Repeat play count text")

            ColumnLayout {
                width: parent.width
                spacing: 12

                RowLayout {
                    ToggleButton {
                        checked: repeatPlayCountTextModel.repeatPlayCountShow.value === true
                        onToggled: {
                            repeatPlayCountTextModel.repeatPlayCountShow.value = !repeatPlayCountTextModel.repeatPlayCountShow.value
                        }
                    }

                    StyledTextLabel {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        text: qsTrc("notation/editstyle/repeatplaycount", "Automatically show text at repeat barlines")
                    }
                }

                CheckBox {
                    text: qsTrc("notation/editstyle/repeatplaycount", "Show for single repeats (e.g. “x2”)")
                    checked: repeatPlayCountTextModel.repeatPlayCountShowSingleRepeats.value === true
                    onClicked: repeatPlayCountTextModel.repeatPlayCountShowSingleRepeats.value = !repeatPlayCountTextModel.repeatPlayCountShowSingleRepeats.value

                    enabled: repeatPlayCountTextModel.repeatPlayCountShow.value === true
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    StyledTextLabel {
                        text: qsTrc("notation", "Preset")
                        enabled: repeatPlayCountTextModel.repeatPlayCountShow.value === true
                    }

                    RowLayout {
                        spacing: 6
                        Layout.fillWidth: true

                        enabled: repeatPlayCountTextModel.repeatPlayCountShow.value === true

                        ComboBoxDropdown {
                            Layout.preferredWidth: 290 - editTextStyleButton.width - 6
                            model: repeatPlayCountTextModel.textPresetOptions()

                            styleItem: repeatPlayCountTextModel.repeatTextPreset
                        }

                        FlatButton {
                            id: editTextStyleButton
                            text: qsTrc("notation", "Edit text style")

                            onClicked: {
                                root.goToTextStylePage("repeat-play-count")
                            }
                        }
                    }
                }
            }
        }
    }
}
