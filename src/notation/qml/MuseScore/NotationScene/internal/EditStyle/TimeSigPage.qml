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
    color: ui.theme.backgroundPrimaryColor

    TimeSigPageModel {
        id: model
    }

    ColumnLayout {
        width: parent.width
        spacing: 12

        StyledTextLabel {
            horizontalAlignment: Text.AlignLeft
            text: qsTrc("notation", "Position:")
        }

        RoundedRadioButton {
            text: qsTrc("notation", "On all staves")
            checked: model.timeSigPlacement.value === 0
            onToggled: model.timeSigPlacement.value = 0
        }

        RoundedRadioButton {
            text: qsTrc("notation", "Above staves")
            checked: model.timeSigPlacement.value === 1
            onToggled: model.timeSigPlacement.value = 1
        }

        RoundedRadioButton {
            text: qsTrc("notation", "Across staves")
            checked: model.timeSigPlacement.value === 2
            onToggled: model.timeSigPlacement.value = 2
        }

        FlatButton {
            visible: model.timeSigPlacement.value !== 0
            text: qsTrc("notation", "Edit system object staves...")
            //onClicked:
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.fillHeight: true

            title: qsTrc("notation", "Style")

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
                        text: qsTrc("notation", "Font style:")
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation", "Standard")
                        checked: parent.placementStyle.value === 0
                        onToggled: parent.placementStyle.value = 0
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation", "Narrow")
                        checked: parent.placementStyle.value === 1
                        onToggled: parent.placementStyle.value = 1
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation", "Large sans-serif")
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
                        text: qsTrc("notation", "Alignment with barlines:")
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation", "Centered")
                        checked: model.timeSigCenterOnBarline.value === true
                        onToggled: model.timeSigCenterOnBarline.value = true
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation", "Left-aligned")
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
                        text: qsTrc("notation", "Cautionary time signature at the end of the system:")
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation", "Hang into page margin")
                        checked: model.timeSigHangIntoMargin.value === true
                        onToggled: model.timeSigHangIntoMargin.value = true
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation", "Align within page margin")
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
                        text: qsTrc("notation", "Vertical alignment with staves:")
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation", "Align to top staff")
                        checked: model.timeSigCenterAcrossStaveGroup.value === false
                        onToggled: model.timeSigCenterAcrossStaveGroup.value = false
                    }

                    RoundedRadioButton {
                        text: qsTrc("notation", "Center across stave group")
                        checked: model.timeSigCenterAcrossStaveGroup.value === true
                        onToggled: model.timeSigCenterAcrossStaveGroup.value = true
                    }
                }


                RowLayout {
                    Layout.fillWidth: true

                    spacing: 12

                    StyledTextLabel {
                        horizontalAlignment: Text.AlignLeft
                        text: qsTrc("notation", "Scale:")
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
                        text: qsTrc("notation", "Number gap:")
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
                        text: qsTrc("notation", "Vertical offset:")
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
