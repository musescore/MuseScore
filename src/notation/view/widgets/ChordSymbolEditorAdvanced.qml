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
import QtQuick.Controls 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    property var editorModel: null

    Flickable{
        id: flickable

        // Just to make it scroll
        height: 200
        width: root.width

        boundsBehavior: Flickable.StopAtBounds
        interactive: true

        contentHeight: 400

        ScrollBar.vertical: StyledScrollBar {}

        Row{
            Column{
                id: leftColumn

                StyledTextLabel{
                    text: qsTrc("notation","Quality")
                }

                Row{
                    Column{
                        StyledTextLabel{
                            text: qsTrc("notation","Vertical Offset")
                        }

                        IncrementalPropertyControl {
                            id: qualityAdjustSpinBox

                            width: root.width/4

                            currentValue: editorModel.qualityAdjust

                            step: 1
                            minValue: -10
                            maxValue: 10

                            onValueEdited: editorModel.setProperty("QualityAdjust", newValue)
                        }
                    }

                    Column{
                        StyledTextLabel{
                            text: qsTrc("notation","Scaling")
                        }

                        IncrementalPropertyControl {
                            id: qualityMagSpinBox

                            width: root.width/4

                            currentValue: editorModel.qualityMag

                            step: 0.1
                            minValue: -10
                            maxValue: 10

                            onValueEdited: editorModel.setProperty("QualityMag", newValue)
                        }
                    }

                }

                StyledTextLabel{
                    text: qsTrc("notation","Extensions")
                }

                Row{
                    Column{
                        StyledTextLabel{
                            text: qsTrc("notation","Vertical Offset")
                        }

                        IncrementalPropertyControl {
                            id: extensionAdjustSpinBox

                            width: root.width/4

                            currentValue: editorModel.extensionAdjust

                            step: 1
                            minValue: -10
                            maxValue: 10

                            onValueEdited: editorModel.setProperty("ExtensionAdjust", newValue)
                        }
                    }

                    Column{
                        StyledTextLabel{
                            text: qsTrc("notation","Scaling")
                        }

                        IncrementalPropertyControl {
                            id: extensionMagSpinBox

                            width: root.width/4

                            currentValue: editorModel.extensionMag

                            step: 0.1
                            minValue: -10
                            maxValue: 10

                            onValueEdited: editorModel.setProperty("ExtensionMag", newValue)
                        }
                    }

                }

                StyledTextLabel{
                    text: qsTrc("notation","Alterations")
                }

                Row{
                    Column{
                        StyledTextLabel{
                            text: qsTrc("notation","Vertical Offset")
                        }

                        IncrementalPropertyControl {
                            id: modifierAdjustSpinBox

                            width: root.width/4

                            currentValue: editorModel.modifierAdjust

                            step: 1
                            minValue: -10
                            maxValue: 10

                            onValueEdited: editorModel.setProperty("ModifierAdjust", newValue)
                        }
                    }

                    Column{
                        StyledTextLabel{
                            text: qsTrc("notation","Scaling")
                        }

                        IncrementalPropertyControl {
                            id: modifierMagSpinBox

                            width: root.width/4

                            currentValue: editorModel.modifierMag

                            step: 0.1
                            minValue: -10
                            maxValue: 10

                            onValueEdited: editorModel.setProperty("ModifierMag", newValue)
                        }
                    }

                }
                StyledTextLabel{
                    text: qsTrc("notation","Positioning")
                }

                Column{
                    StyledTextLabel{
                        text: qsTrc("notation","Minimum chord spacing")
                    }

                    IncrementalPropertyControl {
                        id: chordSpacingSpinBox

                        width: root.width/4

                        currentValue: editorModel.minHarmonyDistance
                        measureUnitsSymbol: qsTrc("notation", "sp")

                        step: 0.1
                        minValue: -50
                        maxValue: 10

                        onValueEdited: editorModel.setProperty("minHarmonyDistance", newValue)
                    }
                }

                Column{
                    StyledTextLabel{
                        text: qsTrc("notation","Minimum barline distance")
                    }

                    IncrementalPropertyControl {
                        id: barlineDistanceSpinBox

                        width: root.width/4

                        currentValue: editorModel.maxHarmonyBarDistance
                        measureUnitsSymbol: qsTrc("notation", "sp")

                        step: 0.5
                        minValue: -50
                        maxValue: 50

                        onValueEdited: editorModel.setProperty("maxHarmonyBarDistance", newValue)
                    }
                }

                Column{
                    StyledTextLabel{
                        text: qsTrc("notation","Distance to fretboard diagram")
                    }

                    IncrementalPropertyControl {
                        id: distToFretboardSpinBox

                        width: root.width/4

                        currentValue: editorModel.harmonyFretDistance
                        measureUnitsSymbol: qsTrc("notation", "sp")

                        step: 0.5
                        minValue: -10000
                        maxValue: 10000

                        onValueEdited: editorModel.setProperty("HarmonyFretDistance", newValue)
                    }
                }
                Row{
                    Column{
                        StyledTextLabel{
                            text: qsTrc("notation","Minimum shift above")
                        }

                        IncrementalPropertyControl {
                            id: shiftBelowSpinBox

                            width: root.width/4

                            currentValue: editorModel.maxChordShiftAbove
                            measureUnitsSymbol: qsTrc("notation", "sp")

                            step: 0.5
                            minValue: 0
                            maxValue: 100

                            onValueEdited: editorModel.setProperty("maxChordShiftAbove", newValue)
                        }
                    }

                    Column{
                        StyledTextLabel{
                            text: qsTrc("notation","Minimum shift below")
                        }

                        IncrementalPropertyControl {
                            id: shiftAboveSpinBox

                            width: root.width/4

                            currentValue: editorModel.maxChordShiftBelow
                            measureUnitsSymbol: qsTrc("notation", "sp")

                            step: 0.5
                            minValue: 0
                            maxValue: 100

                            onValueEdited: editorModel.setProperty("maxChordShiftBelow", newValue)
                        }
                    }

                }
                Column{
                    StyledTextLabel{
                        text: qsTrc("notation","Capo fret position")
                    }

                    IncrementalPropertyControl {
                        id: capoFretSpinBox

                        width: root.width/4

                        currentValue: editorModel.capoFretPosition

                        step: 1
                        minValue: 0
                        maxValue: 11

                        onValueEdited: editorModel.setProperty("capoPosition", newValue)
                    }
                }
            }
            Column{
                id: rightColumn
            }
        }
    }
}
