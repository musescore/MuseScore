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

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Rectangle {
    anchors.fill: parent
    color: "transparent"

    TimeSigPageModel {
        id: model
    }

    ColumnLayout {
        width: parent.width
        spacing: 12

        RowLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                StyledTextLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft
                    text: qsTrc("notation/editstyle/timesignatures", "Position:")
                }

                RoundedRadioButton {
                    text: qsTrc("notation/editstyle/timesignatures", "On all staves")
                    checked: model.timeSigPlacement.value === 0
                    onToggled: model.timeSigPlacement.value = 0
                }

                RoundedRadioButton {
                    text: qsTrc("notation/editstyle/timesignatures", "Above staves")
                    checked: model.timeSigPlacement.value === 1
                    onToggled: model.timeSigPlacement.value = 1
                }

                RoundedRadioButton {
                    text: qsTrc("notation/editstyle/timesignatures", "Across staves")
                    checked: model.timeSigPlacement.value === 2
                    onToggled: model.timeSigPlacement.value = 2
                }

                StyledTextLabel {
                    visible: model.timeSigPlacement.value !== 0
                    text: qsTrc("notation/editstyle/timesignatures", "Set where time signatures appear using the Layout panel.")
                }
            }

            Rectangle {
                Layout.preferredWidth: childrenRect.width + 16
                Layout.fillHeight: true
                color: "#ffffff"
                border.color: ui.theme.strokeColor
                radius: ui.theme.borderWidth

                Image {
                    id: image

                    height: parent.height - 16
                    anchors.centerIn: parent
                    mipmap: true

                    fillMode: Image.PreserveAspectFit

                    source: model.timeSigPlacement.value === 0 ? "timeSigImages/timesig-on_all_staves.png"
                          : model.timeSigPlacement.value === 1 ? "timeSigImages/timesig-above_staves.png"
                                                               : "timeSigImages/timesig-across_staves.png"
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.fillHeight: true

            title: qsTrc("notation/editstyle/timesignatures", "Style and size")

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8

                    property var placementStyle: model.timeSigPlacement.value === 0 ? model.timeSigNormalStyle
                                               : model.timeSigPlacement.value === 1 ? model.timeSigAboveStyle
                                                                                    : model.timeSigAcrossStyle

                    StyledTextLabel {
                        horizontalAlignment: Text.AlignLeft
                        text: qsTrc("notation/editstyle/timesignatures", "Font style:")
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation/editstyle/timesignatures", "Standard")
                        checked: parent.placementStyle.value === 0
                        onToggled: parent.placementStyle.value = 0
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation/editstyle/timesignatures", "Narrow")
                        checked: parent.placementStyle.value === 1
                        onToggled: parent.placementStyle.value = 1
                    }

                    RoundedRadioButton {
                        //: Means: whether to use a sans-serif font for large time signatures
                        text: qsTrc("notation/editstyle/timesignatures", "Narrow sans-serif")
                        checked: parent.placementStyle.value === 2
                        onToggled: parent.placementStyle.value = 2
                    }
                }

                ColumnLayout {
                    visible: model.timeSigPlacement.value === 1
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8
                    StyledTextLabel {
                        horizontalAlignment: Text.AlignLeft
                        text: qsTrc("notation/editstyle/timesignatures", "Alignment with barlines:")
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation/editstyle/timesignatures", "Centered")
                        checked: model.timeSigCenterOnBarline.value === true
                        onToggled: model.timeSigCenterOnBarline.value = true
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation/editstyle/timesignatures", "Left-aligned")
                        checked: model.timeSigCenterOnBarline.value === false
                        onToggled: model.timeSigCenterOnBarline.value = false
                    }
                }

                ColumnLayout {
                    visible: model.timeSigPlacement.value === 1
                    enabled: model.timeSigCenterOnBarline.value === true
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8
                    StyledTextLabel {
                        horizontalAlignment: Text.AlignLeft
                        text: qsTrc("notation/editstyle/timesignatures", "Cautionary time signature at the end of the system:")
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation/editstyle/timesignatures", "Hang into page margin")
                        checked: model.timeSigHangIntoMargin.value === true
                        onToggled: model.timeSigHangIntoMargin.value = true
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation/editstyle/timesignatures", "Align within page margin")
                        checked: model.timeSigHangIntoMargin.value === false
                        onToggled: model.timeSigHangIntoMargin.value = false
                    }
                }

                ColumnLayout {
                    visible: model.timeSigPlacement.value === 2
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8
                    StyledTextLabel {
                        horizontalAlignment: Text.AlignLeft
                        text: qsTrc("notation/editstyle/timesignatures", "Vertical alignment with staves:")
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation/editstyle/timesignatures", "Align to top staff")
                        checked: model.timeSigCenterAcrossStaveGroup.value === false
                        onToggled: model.timeSigCenterAcrossStaveGroup.value = false
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation/editstyle/timesignatures", "Center across stave group")
                        checked: model.timeSigCenterAcrossStaveGroup.value === true
                        onToggled: model.timeSigCenterAcrossStaveGroup.value = true
                    }
                }


                RowLayout {
                    Layout.fillWidth: true

                    spacing: 12

                    StyledTextLabel {
                        horizontalAlignment: Text.AlignLeft
                        text: qsTrc("notation/editstyle/timesignatures", "Scale:")
                    }

                    RowLayout {
                        spacing: 6

                        property var scaleStyle: model.timeSigPlacement.value === 0 ? model.timeSigNormalScale
                                               : model.timeSigPlacement.value === 1 ? model.timeSigAboveScale
                                                                                    : model.timeSigAcrossScale

                        IncrementalPropertyControl {
                            Layout.preferredWidth: 80
                            decimals: 2
                            measureUnitsSymbol: 'x'

                            currentValue: parent.scaleStyle.value.width
                            onValueEdited: function(newValue) {
                                parent.scaleStyle.value.width = newValue
                            }
                        }

                        IncrementalPropertyControl {
                            Layout.preferredWidth: 80
                            decimals: 2
                            measureUnitsSymbol: 'x'

                            currentValue: parent.scaleStyle.value.height
                            onValueEdited: function(newValue) {
                                parent.scaleStyle.value.height = newValue
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    spacing: 12

                    property var numDistStyle: model.timeSigPlacement.value === 0 ? model.timeSigNormalNumDist
                                             : model.timeSigPlacement.value === 1 ? model.timeSigAboveNumDist
                                                                                : model.timeSigAcrossNumDist

                    StyledTextLabel {
                        horizontalAlignment: Text.AlignLeft
                        text: qsTrc("notation/editstyle/timesignatures", "Number gap:")
                    }

                    IncrementalPropertyControl {
                        Layout.preferredWidth: 80
                        decimals: 2
                        measureUnitsSymbol: 'sp'

                        currentValue: parent.numDistStyle.value
                        onValueEdited: function(newValue) {
                            parent.numDistStyle.value = newValue
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    spacing: 12

                    enabled: !(model.timeSigPlacement.value === 2 && model.timeSigCenterAcrossStaveGroup.value === true)

                    property var yStyle: model.timeSigPlacement.value === 0 ? model.timeSigNormalY
                                             : model.timeSigPlacement.value === 1 ? model.timeSigAboveY
                                                                                : model.timeSigAcrossY

                    StyledTextLabel {
                        horizontalAlignment: Text.AlignLeft
                        text: qsTrc("notation/editstyle/timesignatures", "Vertical offset:")
                    }

                    IncrementalPropertyControl {
                        Layout.preferredWidth: 80
                        decimals: 2
                        measureUnitsSymbol: 'sp'

                        currentValue: parent.yStyle.value
                        onValueEdited: function(newValue) {
                            parent.yStyle.value = newValue
                        }
                    }
                }
            }
        }
    }
}
