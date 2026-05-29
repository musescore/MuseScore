/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

    contentWidth: Math.max(column.implicitWidth, root.width)
    contentHeight: column.implicitHeight

    StaveSharingPageModel {
        id: staveSharingModel
    }

    ColumnLayout {
        id: column
        width: parent.width
        spacing: 12

        ColumnLayout {
            width: parent.width
            spacing: 6

            ToggleButton {
                id: enableStaveSharingToggle
                Layout.fillWidth: true
                checked: staveSharingModel.isStaveSharingEnabled
                onToggled: staveSharingModel.isStaveSharingEnabled = !checked

                text: qsTrc("notation/editstyle/stavesharing", "Enable stave sharing")
            }

            StyledGroupBox {
                id: staveSharingBox
                enabled: enableStaveSharingToggle.checked
                Layout.fillWidth: true

                ColumnLayout {
                    width: parent.width
                    spacing: 12

                    StyledGroupBox {
                        id: condensingRulesBox
                        Layout.fillWidth: true
                        title: qsTrc("notation/editstyle/stavesharing", "Condensing rules")

                        CheckBox {
                            text: qsTrc("notation/editstyle/stavesharing", "Allow voice crossing")
                            checked: staveSharingModel.allowVoiceCrossing.value === true
                            onClicked: staveSharingModel.allowVoiceCrossing.value = !staveSharingModel.allowVoiceCrossing.value
                        }
                    }

                    StyledGroupBox {
                        id: instrLabelsBox
                        Layout.fillWidth: true
                        title: qsTrc("notation/editstyle/stavesharing", "Instrument labels")

                        ColumnLayout {
                            spacing: 12
                            width: parent.width

                            RowLayout {
                                Layout.fillWidth: false
                                spacing: 8

                                StyledTextLabel {
                                    horizontalAlignment: Qt.AlignLeft
                                    text: qsTrc("notation/editstyle/stavesharing", "Show numerals as interval when the number of consecutive instruments exceeds")
                                }

                                IncrementalPropertyControl {
                                    Layout.preferredWidth: 60
                                    width: 60
                                    decimals: 0
                                    step: 1
                                    minValue: 1
                                    maxValue: 100
                                    currentValue: staveSharingModel.compressWithHyphenMoreThan.value
                                    onValueEditingFinished: function(newValue) {
                                        staveSharingModel.compressWithHyphenMoreThan.value = newValue
                                    }
                                }

                                StyleResetButton {
                                    styleItem: staveSharingModel.compressWithHyphenMoreThan
                                }
                            }

                            ColumnLayout {
                                width: parent.width
                                spacing: 8

                                StyledTextLabel {
                                    text: qsTrc("notation/editstyle/stavesharing", "Align two-instrument margin labels")
                                }

                                RadioButtonGroup {
                                    orientation: ListView.Horizontal
                                    spacing: 8

                                    model: [
                                        {text: qsTrc("notation/editstyle/timesignatures", "Horizontally"), value: 1 },
                                        {text: qsTrc("notation/editstyle/timesignatures", "Vertically"), value: 0 },
                                    ]

                                    delegate: FlatRadioButton {
                                        required text
                                        required property int value
                                        height: 30
                                        checked: staveSharingModel.twoInstrLabelAlign.value === value
                                        onToggled: staveSharingModel.twoInstrLabelAlign.value = value
                                    }
                                }
                            }

                            ColumnLayout {
                                width: parent.width
                                spacing: 8

                                StyledTextLabel {
                                    text: qsTrc("notation/editstyle/stavesharing", "Use trailing dot for margin labels")
                                }

                                StyleToggle {
                                    text: qsTrc("notation/editstyle/stavesharing", "On single instrument")
                                    styleItem: staveSharingModel.trailingDotOnMarginLabelsSingle
                                }

                                StyleToggle {
                                    text: qsTrc("notation/editstyle/stavesharing", "On multiple instruments")
                                    styleItem: staveSharingModel.trailingDotOnMarginLabelsMultiple
                                }
                            }

                            ColumnLayout {
                                width: parent.width
                                spacing: 8

                                StyledTextLabel {
                                    text: qsTrc("notation/editstyle/stavesharing", "Use trailing dot for on-staff labels")
                                }

                                StyleToggle {
                                    text: qsTrc("notation/editstyle/stavesharing", "On single instrument")
                                    styleItem: staveSharingModel.trailingDotOnInStaffLabelsSingle
                                }

                                StyleToggle {
                                    text: qsTrc("notation/editstyle/stavesharing", "On multiple instruments")
                                    styleItem: staveSharingModel.trailingDotOnInStaffLabelsMultiple
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
