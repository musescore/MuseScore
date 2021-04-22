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
import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0

import "../common"
import "notes"
import "fermatas"
import "tempos"
import "glissandos"
import "barlines"
import "sectionbreaks"
import "markers"
import "jumps"
import "keysignatures"
import "accidentals"
import "fretdiagrams"
import "pedals"
import "spacers"
import "clefs"
import "hairpins"
import "crescendos"
import "stafftype"
import "frames"
import "articulations"
import "ornaments"
import "ambituses"
import "images"
import "chordsymbols"
import "brackets"
import "timesignatures"
import "bends"
import "tremolobars"
import "mmrests"
import "tremolos"
import "measurerepeats"

InspectorSectionView {
    id: root

    implicitHeight: grid.implicitHeight

    function updateContentHeight(popupContentHeight) {
        root.contentHeight = implicitHeight + popupContentHeight
    }

    GridLayout {
        id: grid

        width: parent.width

        columns: 2
        columnSpacing: 4

        NoteSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "NoteSettings"
            navigation.row: root.navigationRow(1)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_NOTE) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        FermataSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "FermataSettings"
            navigation.row: root.navigationRow(2)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_FERMATA) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        GlissandoSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "GlissandoSettings"
            navigation.row: root.navigationRow(3)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_GLISSANDO) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        TempoSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "TempoSettings"
            navigation.row: root.navigationRow(4)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TEMPO) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        BarlineSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "BarlineSettings"
            navigation.row: root.navigationRow(5)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            barlineSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_BARLINE) : null
            staffSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_STAFF) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        SectionBreakSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "SectionBreakSettings"
            navigation.row: root.navigationRow(6)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_SECTIONBREAK) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        MarkerSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "MarkerSettings"
            navigation.row: root.navigationRow(7)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_MARKER) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        JumpSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "JumpSettings"
            navigation.row: root.navigationRow(8)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_JUMP) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        KeySignatureSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "KeySignatureSettings"
            navigation.row: root.navigationRow(9)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_KEYSIGNATURE) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        AccidentalSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "AccidentalSettings"
            navigation.row: root.navigationRow(10)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_ACCIDENTAL) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        FretDiagramSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "FretDiagramSettings"
            navigation.row: root.navigationRow(11)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_FRET_DIAGRAM) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        PedalSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "PedalSettings"
            navigation.row: root.navigationRow(12)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_PEDAL) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        SpacerSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "SpacerSettings"
            navigation.row: root.navigationRow(13)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_SPACER) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        ClefSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "ClefSettings"
            navigation.row: root.navigationRow(14)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_CLEF) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        HairpinSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "HairpinSettings"
            navigation.row: root.navigationRow(15)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_HAIRPIN) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        CrescendoSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "CrescendoSettings"
            navigation.row: root.navigationRow(16)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_CRESCENDO) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        StaffTypeSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "StaffTypeSettings"
            navigation.row: root.navigationRow(17)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_STAFF_TYPE_CHANGES) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        TextFrameSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "TextFrameSettings"
            navigation.row: root.navigationRow(18)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TEXT_FRAME) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        VerticalFrameSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "VerticalFrameSettings"
            navigation.row: root.navigationRow(19)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_VERTICAL_FRAME) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        HorizontalFrameSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "HorizontalFrameSettings"
            navigation.row: root.navigationRow(20)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_HORIZONTAL_FRAME) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        ArticulationSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "ArticulationSettings"
            navigation.row: root.navigationRow(21)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_ARTICULATION) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        OrnamentSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "OrnamentSettings"
            navigation.row: root.navigationRow(22)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_ORNAMENT) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        AmbitusSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "AmbitusSettings"
            navigation.row: root.navigationRow(23)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_AMBITUS) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        ImageSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "ImageSettings"
            navigation.row: root.navigationRow(24)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_IMAGE) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        ChordSymbolSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "ChordSymbolSettings"
            navigation.row: root.navigationRow(25)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_CHORD_SYMBOL) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        BracketSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "BracketSettings"
            navigation.row: root.navigationRow(26)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_BRACKET) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        BraceSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "BraceSettings"
            navigation.row: root.navigationRow(27)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_BRACE) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        TimeSignatureSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "TimeSignatureSettings"
            navigation.row: root.navigationRow(28)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TIME_SIGNATURE) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        MMRestSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "MMRestSettings"
            navigation.row: root.navigationRow(29)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_MMREST) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        BendSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "BendSettings"
            navigation.row: root.navigationRow(30)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_BEND) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        TremoloBarSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "TremoloBarSettings"
            navigation.row: root.navigationRow(31)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TREMOLOBAR) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        TremoloSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "TremoloSettings"
            navigation.row: root.navigationRow(32)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TREMOLO) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        MeasureRepeatSettings {
            navigation.panel: root.navigationPanel
            navigation.name: "MeasureRepeatSettings"
            navigation.row: root.navigationRow(33)
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_MEASURE_REPEAT) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }
    }
}
