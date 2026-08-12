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

                    CheckBox {
                        text: qsTrc("notation/editstyle/stavesharing", "Allow voice crossing")
                        checked: staveSharingModel.allowVoiceCrossing.value === true
                        onClicked: staveSharingModel.allowVoiceCrossing.value = !staveSharingModel.allowVoiceCrossing.value
                    }

                    StyledGroupBox {
                        Layout.fillWidth: true
                        title: qsTrc("notation/editstyle/stavesharing", "Instrument labels on staff")

                        ColumnLayout {
                            width: parent.width
                            spacing: 8

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                StyledTextLabel {
                                    text: qsTrc("notation/editstyle/stavesharing", "Text for unison indication:")
                                }

                                TextInputField {
                                    Layout.preferredWidth: 48
                                    currentText: staveSharingModel.textForUnisonLabel.value
                                    onTextEditingFinished: function(newValue) {
                                        staveSharingModel.textForUnisonLabel.value = newValue
                                    }
                                }

                                StyleResetButton {
                                    styleItem: staveSharingModel.textForUnisonLabel
                                }
                            }

                            StyledTextLabel {
                                text: qsTrc("notation/editstyle/instrumentnames", "When continuing onto a new system:")
                            }

                            Repeater {
                                model: [
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Restate label"), value: 1 },
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Restate with parenthesis"), value: 0 },
                                    { text: qsTrc("notation/editstyle/instrumentnames", "Do not restate"), value: 2 },
                                ]

                                RoundedRadioButton {
                                    required property var modelData
                                    text: modelData.text
                                    checked: staveSharingModel.unisonLabelRestateOnNewSystem.value === modelData.value
                                    onClicked: staveSharingModel.unisonLabelRestateOnNewSystem.value = modelData.value
                                }
                            }

                            ToggleButton {
                                id: followInstrNameToggle

                                checked: staveSharingModel.sharedOnStaffNumeralsFollowInstrumentNumerals.value === true
                                onToggled: staveSharingModel.sharedOnStaffNumeralsFollowInstrumentNumerals.value = !checked

                                text: qsTrc("notation/editstyle/stavesharing", "Format instrument numerals as in margin labels")
                            }

                            ColumnLayout {
                                spacing: 8
                                enabled: !followInstrNameToggle.checked

                                StyledTextLabel {
                                    text: qsTrc("notation/editstyle/instrumentnames", "Add trailing dot")
                                }

                                StyleToggle {
                                    text: qsTrc("notation/editstyle/instrumentnames", "On single instruments")
                                    styleItem: staveSharingModel.sharedOnStaffNumeralsTrailingDotSingle
                                }

                                StyleToggle {
                                    text: qsTrc("notation/editstyle/instrumentnames", "On multiple combined instruments")
                                    styleItem: staveSharingModel.sharedOnStaffNumeralsTrailingDotMultiple
                                }

                                RowLayout {
                                    Layout.fillWidth: false
                                    spacing: 8

                                    CheckBox {
                                        text: qsTrc("notation/editstyle/instrumentnames", "Show numerals as interval when the number of consecutive instruments exceeds")
                                        checked: staveSharingModel.sharedOnStaffNumeralsHyphenEnable.value === true
                                        onClicked: staveSharingModel.sharedOnStaffNumeralsHyphenEnable.value = !checked
                                    }

                                    IncrementalPropertyControl {
                                        Layout.preferredWidth: 60
                                        width: 60
                                        decimals: 0
                                        step: 1
                                        minValue: 1
                                        maxValue: 100
                                        currentValue: staveSharingModel.sharedOnStaffNumeralsHyphenThreshold.value
                                        onValueEditingFinished: function(newValue) {
                                            staveSharingModel.sharedOnStaffNumeralsHyphenThreshold.value = newValue
                                        }
                                    }

                                    StyleResetButton {
                                        styleItem: staveSharingModel.sharedOnStaffNumeralsHyphenThreshold
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
