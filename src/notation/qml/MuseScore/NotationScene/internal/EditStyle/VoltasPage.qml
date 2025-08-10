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

    contentWidth: Math.max(column.implicitWidth, width)
    contentHeight: column.height

    VoltasPageModel {
        id: voltasPage
    }

    ColumnLayout {
        id: column
        width: parent.width
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/voltas", "Voltas")

            ColumnLayout {
                spacing: 12

                RowLayout {
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/voltas", "Vertical position:")
                        horizontalAlignment: Text.AlignLeft
                        Layout.preferredWidth: 160
                    }

                    IncrementalPropertyControl {
                        Layout.preferredWidth: 100
                        currentValue: voltasPage.voltaPosAbove.value.y
                        measureUnitsSymbol: qsTrc("global", "sp")
                        maxValue: 0
                        minValue: -100
                        decimals: 2
                        step: 0.25

                        onValueEdited: function(newValue) {
                            voltasPage.voltaPosAbove.value.y = newValue
                        }
                    }

                    FlatButton {
                        icon: IconCode.UNDO
                        enabled: !voltasPage.voltaPosAbove.isDefault
                        onClicked: voltasPage.voltaPosAbove.value = voltasPage.voltaPosAbove.defaultValue
                    }
                }

                StyleSpinboxWithReset {
                    styleItem: voltasPage.voltaHook
                    label: qsTrc("notation/editstyle/voltas", "Hook height:")
                    suffix: qsTrc("global", "sp")
                    labelAreaWidth: 160
                    controlAreaWidth: 100
                    spinBoxWidth: 100
                }

                StyleSpinboxWithReset {
                    styleItem: voltasPage.voltaLineWidth
                    label: qsTrc("notation/editstyle/voltas", "Line thickness:")
                    suffix: qsTrc("global", "sp")
                    labelAreaWidth: 160
                    controlAreaWidth: 100
                    spinBoxWidth: 100
                }

                RadioButtonSelectorWithReset {
                    styleItem: voltasPage.voltaLineStyle
                    label: qsTrc("notation", "Line style:")
                    labelAreaWidth: 160
                    controlAreaWidth: 276

                    model: [
                        { iconCode: IconCode.LINE_NORMAL, value: 0, title: qsTrc("notation", "Normal") },
                        { iconCode: IconCode.LINE_DASHED, value: 1, title: qsTrc("notation", "Dashed") },
                        { iconCode: IconCode.LINE_DOTTED, value: 2, title: qsTrc("notation", "Dotted") },
                    ]
                }

                StyleSpinboxWithReset {
                    visible: voltasPage.voltaLineStyle.value === 1
                    styleItem: voltasPage.voltaDashLineLen
                    label: qsTrc("notation/editstyle/voltas", "Dash:")
                    labelAreaWidth: 160
                    controlAreaWidth: 100
                    spinBoxWidth: 100
                }

                StyleSpinboxWithReset {
                    visible: voltasPage.voltaLineStyle.value === 1
                    styleItem: voltasPage.voltaDashGapLen
                    label: qsTrc("notation/editstyle/voltas", "Gap:")
                    labelAreaWidth: 160
                    controlAreaWidth: 100
                    spinBoxWidth: 100
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/voltas", "Alignment")

            ColumnLayout {
                spacing: 12

                RowLayout {
                    spacing: 16

                    StyledImage {
                        horizontalPadding: 0
                        verticalPadding: 0
                        forceHeight: 96
                        source: voltasPage.voltaAlignStartBeforeKeySig.value === false
                                ? "voltasImages/voltaAfterKeySig.png"
                                : "voltasImages/voltaBeforeKeySig.png"
                    }

                    ColumnLayout {
                        spacing: 8

                        StyledTextLabel {
                            text: qsTrc("notation/editstyle/voltas", "At start of system, start volta:")
                            horizontalAlignment: Text.AlignLeft
                        }

                        RoundedRadioButton {
                            text: qsTrc("notation/editstyle/voltas", "After key signature")
                            checked: voltasPage.voltaAlignStartBeforeKeySig.value === false
                            onToggled: voltasPage.voltaAlignStartBeforeKeySig.value = false
                        }

                        RoundedRadioButton {
                            text: qsTrc("notation/editstyle/voltas", "On key signature")
                            checked: voltasPage.voltaAlignStartBeforeKeySig.value === true
                            onToggled: voltasPage.voltaAlignStartBeforeKeySig.value = true
                        }
                    }
                }

                RowLayout {
                    spacing: 16

                    StyledImage {
                        horizontalPadding: 0
                        verticalPadding: 0
                        forceHeight: 96
                        source: voltasPage.voltaAlignEndLeftOfBarline.value === false
                                ? "voltasImages/voltaRightOfBarline.png"
                                : "voltasImages/voltaLeftOfBarline.png"
                    }

                    ColumnLayout {
                        spacing: 8

                        StyledTextLabel {
                            text: qsTrc("notation/editstyle/voltas", "Align start hook:")
                            horizontalAlignment: Text.AlignLeft
                        }

                        RoundedRadioButton {
                            text: qsTrc("notation/editstyle/voltas", "To right side of barlines")
                            checked: voltasPage.voltaAlignEndLeftOfBarline.value === false
                            onToggled: voltasPage.voltaAlignEndLeftOfBarline.value = false
                        }

                        RoundedRadioButton {
                            text: qsTrc("notation/editstyle/voltas", "To left side of barlines")
                            checked: voltasPage.voltaAlignEndLeftOfBarline.value === true
                            onToggled: voltasPage.voltaAlignEndLeftOfBarline.value = true
                        }
                    }
                }
            }
        }
    }
}
