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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

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

    property int viewType: Inspector.TYPE_UNDEFINED

    property int navigationRow: 1

    implicitHeight: loader.implicitHeight

    function updateContentHeight(popupContentHeight) {
        root.contentHeight = root.implicitHeight + popupContentHeight
    }

    Loader {
        id: loader

        width: root.width

        QtObject {
            id: prv

            function componentByType(type) {
                switch (type) {
                case Inspector.TYPE_NOTE: return noteComp
                case Inspector.TYPE_BEAM: return noteComp
                case Inspector.TYPE_NOTEHEAD: return noteComp
                case Inspector.TYPE_STEM: return noteComp
                case Inspector.TYPE_HOOK: return noteComp
                case Inspector.TYPE_FERMATA: return fermataComp
                case Inspector.TYPE_GLISSANDO: return glissandoComp
                case Inspector.TYPE_TEMPO: return tempoComp
                case Inspector.TYPE_BARLINE: return barlineComp
                case Inspector.TYPE_SECTIONBREAK: return sectionBreakComp
                case Inspector.TYPE_MARKER: return markerComp
                case Inspector.TYPE_JUMP: return jumpComp
                case Inspector.TYPE_KEYSIGNATURE: return keySignatureComp
                case Inspector.TYPE_ACCIDENTAL: return accidentalComp
                case Inspector.TYPE_FRET_DIAGRAM: return fretDiagramComp
                case Inspector.TYPE_PEDAL: return pedalComp
                case Inspector.TYPE_SPACER: return spacerComp
                case Inspector.TYPE_CLEF: return clefComp
                case Inspector.TYPE_HAIRPIN: return hairpinComp
                case Inspector.TYPE_CRESCENDO: return crescendoComp
                case Inspector.TYPE_STAFF_TYPE_CHANGES: return staffTypeComp
                case Inspector.TYPE_TEXT_FRAME: return textFrameComp
                case Inspector.TYPE_VERTICAL_FRAME: return verticalFrameComp
                case Inspector.TYPE_HORIZONTAL_FRAME: return horizontalFrameComp
                case Inspector.TYPE_ARTICULATION: return articulationComp
                case Inspector.TYPE_ORNAMENT: return ornamentComp
                case Inspector.TYPE_AMBITUS: return ambitusComp
                case Inspector.TYPE_IMAGE: return imageComp
                case Inspector.TYPE_CHORD_SYMBOL: return chordSymbolComp
                case Inspector.TYPE_BRACKET: return bracketComp
                case Inspector.TYPE_BRACE: return braceComp
                case Inspector.TYPE_TIME_SIGNATURE: return timeSignatureComp
                case Inspector.TYPE_MMREST: return mmRestComp
                case Inspector.TYPE_BEND: return bendComp
                case Inspector.TYPE_TREMOLOBAR: return tremoloBarComp
                case Inspector.TYPE_TREMOLO: return tremoloComp
                case Inspector.TYPE_MEASURE_REPEAT: return measureRepeatComp
                }

                return null
            }
        }

        sourceComponent: root.model ? prv.componentByType(root.model.modelType) : null

        onLoaded: {
            loader.item.model = root.model
        }

        Component {
            id: noteComp
            NoteSettingsTabPanel { }
        }

        Component {
            id: fermataComp
            FermataSettingsPanel { }
        }

        Component {
            id: glissandoComp
            GlissandoSettingsPanel { }
        }

        Component {
            id: tempoComp
            TempoSettingsPanel { }
        }

        Component {
            id: barlineComp
            BarlineSettingsPanel { }
        }

        Component {
            id: sectionBreakComp
            SectionBreakSettingsPanel { }
        }

        Component {
            id: markerComp
            MarkerSettingsPanel { }
        }

        Component {
            id: jumpComp
            JumpSettingsPanel { }
        }

        Component {
            id: keySignatureComp
            KeySignatureSettingsPanel { }
        }

        Component {
            id: accidentalComp
            AccidentalSettingsPanel { }
        }

        Component {
            id: fretDiagramComp
            FretDiagramSettingsPanel { }
        }

        Component {
            id: pedalComp
            PedalSettingsPanel { }
        }

        Component {
            id: spacerComp
            SpacerSettingsPanel { }
        }

        Component {
            id: clefComp
            ClefSettingsPanel { }
        }

        Component {
            id: hairpinComp
            HairpinTabPanel { }
        }

        Component {
            id: crescendoComp
            CrescendoTabPanel { }
        }

        Component {
            id: staffTypeComp
            StaffTypeSettingsPanel { }
        }

        Component {
            id: textFrameComp
            TextFrameSettingsPanel { }
        }

        Component {
            id: verticalFrameComp
            VerticalFrameSettingsPanel { }
        }

        Component {
            id: horizontalFrameComp
            HorizontalFrameSettingsPanel { }
        }

        Component {
            id: articulationComp
            ArticulationSettingsPanel { }
        }

        Component {
            id: ornamentComp
            OrnamentSettingsPanel { }
        }

        Component {
            id: ambitusComp
            AmbitusSettingsPanel { }
        }

        Component {
            id: imageComp
            ImageSettingsPanel { }
        }

        Component {
            id: chordSymbolComp
            ChordSymbolSettingsPanel { }
        }

        Component {
            id: bracketComp
            BracketSettingsPanel { }
        }

        Component {
            id: braceComp
            BraceSettingsPanel { }
        }

        Component {
            id: timeSignatureComp
            TimeSignatureSettingsPanel { }
        }

        Component {
            id: mmRestComp
            MMRestSettingsPanel { }
        }

        Component {
            id: bendComp
            BendSettingsPanel { }
        }

        Component {
            id: tremoloBarComp
            TremoloBarSettingsPanel { }
        }

        Component {
            id: tremoloComp
            TremoloSettingsPanel { }
        }

        Component {
            id: measureRepeatComp
            MeasureRepeatSettingsPanel { }
        }
    }
}
