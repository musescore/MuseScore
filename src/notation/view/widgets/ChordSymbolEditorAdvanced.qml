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
import MuseScore.NotationScene 1.0

Flickable {
    id: root

    property ChordSymbolEditorModel editorModel: null

    contentWidth: width
    contentHeight: content.height

    boundsBehavior: Flickable.StopAtBounds
    clip: true

    ScrollBar.vertical: StyledScrollBar {}

    Row{
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 24
        Column{
            id: leftColumn
            width: (content.width - content.spacing) / 2
            spacing: 18

            LabelledSpinBox {
                id: chordScalingSpinBox
                width: Math.min(126, (parent.width - parent.spacing) / 2)
                text: qsTrc("notation", "Chord Symbol Scaling")

                currentValue: root.editorModel.chordSymbolScaling
                step: 10
                minValue: 1
                maxValue: 100

                onValueEdited: root.editorModel.setProperty("chordSymbolScaling", newValue)
            }

            StyledTextLabel{
                text: qsTrc("notation", "Quality")
                font: ui.theme.bodyBoldFont
            }

            Row{
                width: parent.width
                spacing: 12
                LabelledSpinBox {
                    id: qualityAdjustSpinBox
                    width: Math.min(126, (parent.width - parent.spacing) / 2)
                    text: qsTrc("notation", "Vertical Offset")

                    currentValue: root.editorModel.qualityAdjust
                    step: 1
                    minValue: -10
                    maxValue: 10

                    onValueEdited: root.editorModel.setProperty("QualityAdjust", newValue)
                }

                LabelledSpinBox {
                    id: qualityMagSpinBox
                    width: Math.min(126, (parent.width - parent.spacing) / 2)
                    text: qsTrc("notation", "Scaling")

                    currentValue: root.editorModel.qualityMag
                    step: 0.1
                    minValue: -10
                    maxValue: 10

                    onValueEdited: root.editorModel.setProperty("QualityMag", newValue)
                }
            }

            StyledTextLabel {
                text: qsTrc("notation", "Extensions")
                font: ui.theme.bodyBoldFont
            }

            Row{
                width: parent.width
                spacing: 12
                LabelledSpinBox {
                    id: extensionAdjustSpinBox
                    width: Math.min(126, (parent.width - parent.spacing) / 2)
                    text: qsTrc("notation", "Vertical Offset")

                    currentValue: root.editorModel.extensionAdjust
                    step: 1
                    minValue: -10
                    maxValue: 10

                    onValueEdited: root.editorModel.setProperty("ExtensionAdjust", newValue)
                }

                LabelledSpinBox {
                    id: extensionMagSpinBox
                    width: Math.min(126, (parent.width - parent.spacing) / 2)
                    text: qsTrc("notation", "Scaling")

                    currentValue: root.editorModel.extensionMag
                    step: 0.1
                    minValue: -10
                    maxValue: 10

                    onValueEdited: root.editorModel.setProperty("ExtensionMag", newValue)
                }
            }

            StyledTextLabel {
                text: qsTrc("notation", "Alterations")
                font: ui.theme.bodyBoldFont
            }

            Row{
                width: parent.width
                spacing: 12
                LabelledSpinBox {
                    id: modifierAdjustSpinBox
                    width: Math.min(126, (parent.width - parent.spacing) / 2)
                    text: qsTrc("notation", "Vertical Offset")

                    currentValue: root.editorModel.modifierAdjust
                    step: 1
                    minValue: -10
                    maxValue: 10

                    onValueEdited: root.editorModel.setProperty("ModifierAdjust", newValue)
                }

                LabelledSpinBox {
                    id: modifierMagSpinBox
                    width: Math.min(126, (parent.width - parent.spacing) / 2)
                    text: qsTrc("notation", "Scaling")

                    currentValue: root.editorModel.modifierMag

                    step: 0.1
                    minValue: -10
                    maxValue: 10

                    onValueEdited: root.editorModel.setProperty("ModifierMag", newValue)
                }
            }

            StyledTextLabel {
                text: qsTrc("notation", "Positioning")
                font: ui.theme.bodyBoldFont
            }

            LabelledSpinBox {
                id: chordSpacingSpinBox
                width: Math.min(126, (parent.width - parent.spacing) / 2)
                text: qsTrc("notation", "Minimum chord spacing")

                currentValue: root.editorModel.minHarmonyDistance

                step: 0.1
                minValue: -50
                maxValue: 10

                units: qsTrc("notation", "sp")

                onValueEdited: root.editorModel.setProperty("minHarmonyDistance", newValue)
            }

            LabelledSpinBox {
                id: barlineDistanceSpinBox
                width: Math.min(126, (parent.width - parent.spacing) / 2)
                text: qsTrc("notation", "Minimum barline distance")

                currentValue: root.editorModel.maxHarmonyBarDistance

                step: 0.5
                minValue: -50
                maxValue: 50

                units: qsTrc("notation", "sp")

                onValueEdited: root.editorModel.setProperty("maxHarmonyBarDistance", newValue)
            }

            LabelledSpinBox {
                id: distToFretboardSpinBox
                width: Math.min(126, (parent.width - parent.spacing) / 2)
                text: qsTrc("notation", "Distance to fretboard diagram")

                currentValue: root.editorModel.harmonyFretDistance

                step: 0.5
                minValue: -10000
                maxValue: 10000

                units: qsTrc("notation", "sp")

                onValueEdited: root.editorModel.setProperty("HarmonyFretDistance", newValue)
            }

            Row{
                width: parent.width
                spacing: 12
                LabelledSpinBox {
                    id: shiftAboveSpinBox
                    width: Math.min(126, (parent.width - parent.spacing) / 2)
                    text: qsTrc("notation", "Minimum shift above")

                    currentValue: root.editorModel.maxChordShiftAbove

                    step: 0.5
                    minValue: 0
                    maxValue: 100

                    units: qsTrc("notation", "sp")

                    onValueEdited: root.editorModel.setProperty("maxChordShiftAbove", newValue)
                }

                LabelledSpinBox {
                    id: shiftBelowSpinBox
                    width: Math.min(126, (parent.width - parent.spacing) / 2)
                    text: qsTrc("notation", "Minimum shift below")

                    currentValue: root.editorModel.maxChordShiftBelow

                    step: 0.5
                    minValue: 0
                    maxValue: 100

                    units: qsTrc("notation", "sp")

                    onValueEdited: root.editorModel.setProperty("maxChordShiftBelow", newValue)
                }
            }
            LabelledSpinBox {
                id: capoFretSpinBox
                width: Math.min(126, (parent.width - parent.spacing) / 2)
                text: qsTrc("notation","Capo fret position")

                currentValue: root.editorModel.capoFretPosition

                step: 1
                minValue: 0
                maxValue: 11

                onValueEdited: root.editorModel.setProperty("capoPosition", newValue)
            }
            FlatButton{
                id: chordSymbolFont
                text: qsTrc("notation","Change chord symbol font NW")
                onClicked: root.editorModel.resetProperties()
            }
            FlatButton{
                id: resetProperties
                text: qsTrc("notation","Reset settings to default")
                onClicked: root.editorModel.resetProperties()
            }
        }
        Column{
            id: rightColumn

            width: (content.width - content.spacing) / 2
            spacing: 18

            StyledTextLabel {
                text: qsTrc("notation", "Capitalization")
                font: ui.theme.bodyBoldFont
            }

            CheckBox {
                width: Math.min(126, (parent.width - parent.spacing) / 2)
                property int autoCapital: -1
                checked: (editorModel.autoCapitalization === 1.0)

                text: "Automatically capitalize note names"
                onClicked: {
                    editorModel.setProperty("autoCapitalization", checked ? 0 : 1)
                }
            }

            StyledTextLabel {
                text: qsTrc("notation", "Chord Root")
            }

            RadioButtonGroup {
                id: minorRootCapitalization

                height: 50

                model: [
                    { name: "Cmin", value: 1.0 },
                    { name: "cmin", value: 0.0 },
                ]

                delegate: FlatRadioButton {
                    height: 50
                    width: 100

                    ButtonGroup.group: minorRootCapitalization.radioButtonGroup

                    StyledTextLabel{
                        text: qsTrc("notation", modelData["name"])
                    }
                    checked: editorModel.minorRootCapitalization === modelData["value"]

                    onToggled: {
                        editorModel.setProperty("minorRootCapitalization", modelData["value"])
                    }
                }
            }

            StyledTextLabel {
                text: qsTrc("notation", "Chord quality")
            }

            RadioButtonGroup {
                id: qualitySymbolsCapitalization

                height: 50

                model: [
                    { name: "Maj", value: 1.0 },
                    { name: "maj", value: 0.0 },
                ]

                delegate: FlatRadioButton {
                    height: 50
                    width: 100

                    ButtonGroup.group: qualitySymbolsCapitalization.radioButtonGroup

                    StyledTextLabel{
                        text: qsTrc("notation", modelData["name"])
                    }
                    checked: editorModel.qualitySymbolsCapitalization === modelData["value"]

                    onToggled: {
                        editorModel.setProperty("qualitySymbolsCapitalization", modelData["value"])
                    }
                }
            }

            StyledTextLabel {
                text: qsTrc("notation", "Bass notes")
            }

            RadioButtonGroup {
                id: bassNotesCapitalization

                height: 50

                model: [
                    { name: "C/B", value: 1.0 },
                    { name: "C/b", value: 0.0 },
                ]

                delegate: FlatRadioButton {
                    height: 50
                    width: 100

                    ButtonGroup.group: bassNotesCapitalization.radioButtonGroup

                    StyledTextLabel{
                        text: qsTrc("notation", modelData["name"])
                    }
                    checked: editorModel.bassNotesCapitalization === modelData["value"]

                    onToggled: {
                        editorModel.setProperty("bassNotesCapitalization", modelData["value"])
                    }
                }
            }

            StyledTextLabel {
                text: qsTrc("notation", "Solfege")
            }

            RadioButtonGroup {
                id: solfegeNotesCapitalization

                height: 50

                model: [
                    { name: "DO", value: 1.0 },
                    { name: "Do", value: 0.0 },
                ]

                delegate: FlatRadioButton {
                    height: 50
                    width: 100

                    ButtonGroup.group: solfegeNotesCapitalization.radioButtonGroup

                    StyledTextLabel{
                        text: qsTrc("notation", modelData["name"])
                    }
                    checked: editorModel.solfegeNotesCapitalization === modelData["value"]

                    onToggled: {
                        editorModel.setProperty("solfegeNotesCapitalization", modelData["value"])
                    }
                }
            }

            StyledTextLabel {
                text: qsTrc("notation", "Parentheses")
                font: ui.theme.bodyBoldFont
            }

            StyledTextLabel {
                text: qsTrc("notation", "Alterations")
            }

            RadioButtonGroup {
                id: alterationsParentheses

                height: 50

                model: [
                    { name: "(b5)", value: 1.0 },
                    { name: "b5", value: 0.0 },
                ]

                delegate: FlatRadioButton {
                    height: 50
                    width: 100

                    ButtonGroup.group: alterationsParentheses.radioButtonGroup

                    StyledTextLabel{
                        text: qsTrc("notation", modelData["name"])
                    }
                    checked: editorModel.alterationsParentheses === modelData["value"]

                    onToggled: {
                        editorModel.setProperty("alterationsParentheses", modelData["value"])
                    }
                }
            }

            StyledTextLabel {
                text: qsTrc("notation", "Suspensions")
            }

            RadioButtonGroup {
                id: suspensionsParentheses

                height: 50

                model: [
                    { name: "(sus)", value: 1.0 },
                    { name: "sus", value: 0.0 },
                ]

                delegate: FlatRadioButton {
                    height: 50
                    width: 100

                    ButtonGroup.group: suspensionsParentheses.radioButtonGroup

                    StyledTextLabel{
                        text: qsTrc("notation", modelData["name"])
                    }
                    checked: editorModel.suspensionsParentheses === modelData["value"]

                    onToggled: {
                        editorModel.setProperty("suspensionsParentheses", modelData["value"])
                    }
                }
            }

            StyledTextLabel {
                text: qsTrc("notation", "Minor-Major sevenths")
            }

            RadioButtonGroup {
                id: minMajParentheses

                height: 50

                model: [
                    { name: "m(maj7)", value: 1.0 },
                    { name: "mmaj7", value: 0.0 },
                ]

                delegate: FlatRadioButton {
                    height: 50
                    width: 100

                    ButtonGroup.group: minMajParentheses.radioButtonGroup

                    StyledTextLabel{
                        text: qsTrc("notation", modelData["name"])
                    }
                    checked: editorModel.minMajParentheses === modelData["value"]

                    onToggled: {
                        editorModel.setProperty("minMajParentheses", modelData["value"])
                    }
                }
            }

            StyledTextLabel {
                text: qsTrc("notation", "Additions and omissions")
            }

            RadioButtonGroup {
                id: addOmitParentheses

                height: 50

                model: [
                    { name: "(add)", value: 1.0 },
                    { name: "add", value: 0.0 },
                ]

                delegate: FlatRadioButton {
                    height: 50
                    width: 100

                    ButtonGroup.group: addOmitParentheses.radioButtonGroup

                    StyledTextLabel{
                        text: qsTrc("notation", modelData["name"])
                    }
                    checked: editorModel.addOmitParentheses === modelData["value"]

                    onToggled: {
                        editorModel.setProperty("addOmitParentheses", modelData["value"])
                    }
                }
            }
        }
    }
}
