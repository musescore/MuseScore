/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

import "../common"
import "notes"
import "fermatas"
import "tempos"
import "barlines"
import "sectionbreaks"
import "markers"
import "jumps"
import "keysignatures"
import "accidentals"
import "fretdiagrams"
import "spacers"
import "clefs"
import "lines"
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
import "tuplets"
import "instrumentname"
import "lyrics"
import "rests"
import "dynamics"
import "expressions"
import "stringtunings"
import "symbols"
import "playcounttext"

Loader {
    id: root

    property AbstractInspectorModel model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0

    property string viewObjectName: root.item ? root.item.objectName : ""

    function forceFocusIn() {
        root.item.focusOnFirst()
    }

    QtObject {
        id: prv

        function componentByType(type) {
            switch (type) {
            case AbstractInspectorModel.TYPE_NOTE: return noteComp
            case AbstractInspectorModel.TYPE_BEAM: return noteComp
            case AbstractInspectorModel.TYPE_NOTEHEAD: return noteComp
            case AbstractInspectorModel.TYPE_STEM: return noteComp
            case AbstractInspectorModel.TYPE_HOOK: return noteComp
            case AbstractInspectorModel.TYPE_FERMATA: return fermataComp
            case AbstractInspectorModel.TYPE_GLISSANDO: return glissandoComp
            case AbstractInspectorModel.TYPE_VIBRATO: return vibratoComp
            case AbstractInspectorModel.TYPE_SLUR:
            case AbstractInspectorModel.TYPE_TIE:
            case AbstractInspectorModel.TYPE_LAISSEZ_VIB:
            case AbstractInspectorModel.TYPE_PARTIAL_TIE:
            case AbstractInspectorModel.TYPE_HAMMER_ON_PULL_OFF: return slurAndTieComp
            case AbstractInspectorModel.TYPE_TEMPO: return tempoComp
            case AbstractInspectorModel.TYPE_A_TEMPO: return aTempoComp
            case AbstractInspectorModel.TYPE_TEMPO_PRIMO: return tempoPrimoComp
            case AbstractInspectorModel.TYPE_BARLINE: return barlineComp
            case AbstractInspectorModel.TYPE_SECTIONBREAK: return sectionBreakComp
            case AbstractInspectorModel.TYPE_MARKER: return markerComp
            case AbstractInspectorModel.TYPE_JUMP: return jumpComp
            case AbstractInspectorModel.TYPE_KEYSIGNATURE: return keySignatureComp
            case AbstractInspectorModel.TYPE_ACCIDENTAL: return accidentalComp
            case AbstractInspectorModel.TYPE_FRET_DIAGRAM: return fretDiagramComp
            case AbstractInspectorModel.TYPE_SPACER: return spacerComp
            case AbstractInspectorModel.TYPE_CLEF: return clefComp
            case AbstractInspectorModel.TYPE_HAIRPIN:
            case AbstractInspectorModel.TYPE_CRESCENDO:
            case AbstractInspectorModel.TYPE_DIMINUENDO: return hairpinLineComp
            case AbstractInspectorModel.TYPE_PEDAL:
            case AbstractInspectorModel.TYPE_OTTAVA:
            case AbstractInspectorModel.TYPE_PALM_MUTE:
            case AbstractInspectorModel.TYPE_LET_RING:
            case AbstractInspectorModel.TYPE_VOLTA:
            case AbstractInspectorModel.TYPE_NOTELINE:
            case AbstractInspectorModel.TYPE_TEXT_LINE: return lineComp
            case AbstractInspectorModel.TYPE_GRADUAL_TEMPO_CHANGE: return gradualTempoChangeComp
            case AbstractInspectorModel.TYPE_STAFF_TYPE_CHANGES: return staffTypeComp
            case AbstractInspectorModel.TYPE_TEXT_FRAME: return textFrameComp
            case AbstractInspectorModel.TYPE_VERTICAL_FRAME: return verticalFrameComp
            case AbstractInspectorModel.TYPE_HORIZONTAL_FRAME: return horizontalFrameComp
            case AbstractInspectorModel.TYPE_FRET_FRAME: return fretFrameComp
            case AbstractInspectorModel.TYPE_ARTICULATION: return articulationComp
            case AbstractInspectorModel.TYPE_TAPPING: return articulationComp
            case AbstractInspectorModel.TYPE_ORNAMENT: return ornamentComp
            case AbstractInspectorModel.TYPE_AMBITUS: return ambitusComp
            case AbstractInspectorModel.TYPE_IMAGE: return imageComp
            case AbstractInspectorModel.TYPE_CHORD_SYMBOL: return chordSymbolComp
            case AbstractInspectorModel.TYPE_BRACKET: return bracketComp
            case AbstractInspectorModel.TYPE_TIME_SIGNATURE: return timeSignatureComp
            case AbstractInspectorModel.TYPE_MMREST: return mmRestComp
            case AbstractInspectorModel.TYPE_BEND: return bendComp
            case AbstractInspectorModel.TYPE_TREMOLOBAR: return tremoloBarComp
            case AbstractInspectorModel.TYPE_TREMOLO: return tremoloComp
            case AbstractInspectorModel.TYPE_MEASURE_REPEAT: return measureRepeatComp
            case AbstractInspectorModel.TYPE_TUPLET: return tupletComp
            case AbstractInspectorModel.TYPE_INSTRUMENT_NAME: return instrumentNameComp
            case AbstractInspectorModel.TYPE_LYRICS: return lyricsComp
            case AbstractInspectorModel.TYPE_REST: return restComp
            case AbstractInspectorModel.TYPE_REST_BEAM: return restComp
            case AbstractInspectorModel.TYPE_DYNAMIC: return dynamicComp
            case AbstractInspectorModel.TYPE_EXPRESSION: return expressionComp
            case AbstractInspectorModel.TYPE_STRING_TUNINGS: return stringTuningsComp
            case AbstractInspectorModel.TYPE_SYMBOL: return symbolComp
            case AbstractInspectorModel.TYPE_PLAY_COUNT_TEXT: return playCountTextComp
            }

            return null
        }
    }

    sourceComponent: root.model ? prv.componentByType(root.model.modelType) : null

    onLoaded: {
        root.item.model = root.model
        root.item.navigationPanel = root.navigationPanel
        root.item.navigationRowStart = root.navigationRowStart
    }

    Component {
        id: noteComp
        NoteSettings { }
    }

    Component {
        id: fermataComp
        FermataSettings { }
    }

    Component {
        id: glissandoComp
        GlissandoSettings { }
    }

    Component {
        id: vibratoComp
        VibratoSettings { }
    }

    Component {
        id: slurAndTieComp
        SlurAndTieSettings { }
    }

    Component {
        id: tempoComp
        TempoSettings { }
    }

    Component {
        id: aTempoComp
        TempoRestorePreviousSettings { }
    }

    Component {
        id: tempoPrimoComp
        TempoRestorePreviousSettings { }
    }

    Component {
        id: barlineComp
        BarlineSettings { }
    }

    Component {
        id: sectionBreakComp
        SectionBreakSettings { }
    }

    Component {
        id: markerComp
        MarkerSettings { }
    }

    Component {
        id: jumpComp
        JumpSettings { }
    }

    Component {
        id: keySignatureComp
        KeySignatureSettings { }
    }

    Component {
        id: accidentalComp
        AccidentalSettings { }
    }

    Component {
        id: fretDiagramComp
        FretDiagramSettings { }
    }

    Component {
        id: spacerComp
        SpacerSettings { }
    }

    Component {
        id: clefComp
        ClefSettings { }
    }

    Component {
        id: lineComp
        LineSettings { }
    }

    Component {
        id: hairpinLineComp
        HairpinLineSettings { }
    }

    Component {
        id: gradualTempoChangeComp
        GradualTempoChangeSettings { }
    }

    Component {
        id: staffTypeComp
        StaffTypeSettings { }
    }

    Component {
        id: textFrameComp
        TextFrameSettings { }
    }

    Component {
        id: verticalFrameComp
        VerticalFrameSettings { }
    }

    Component {
        id: horizontalFrameComp
        HorizontalFrameSettings { }
    }

    Component {
        id: fretFrameComp
        FretFrameSettings { }
    }

    Component {
        id: articulationComp
        ArticulationSettings { }
    }

    Component {
        id: ornamentComp
        OrnamentSettings { }
    }

    Component {
        id: ambitusComp
        AmbitusSettings { }
    }

    Component {
        id: imageComp
        ImageSettings { }
    }

    Component {
        id: chordSymbolComp
        ChordSymbolSettings { }
    }

    Component {
        id: bracketComp
        BracketSettings { }
    }

    Component {
        id: timeSignatureComp
        TimeSignatureSettings { }
    }

    Component {
        id: mmRestComp
        MMRestSettings { }
    }

    Component {
        id: bendComp
        BendSettings { }
    }

    Component {
        id: tremoloBarComp
        TremoloBarSettings { }
    }

    Component {
        id: tremoloComp
        TremoloSettings { }
    }

    Component {
        id: measureRepeatComp
        MeasureRepeatSettings { }
    }

    Component {
        id: tupletComp
        TupletSettings {}
    }

    Component {
        id: instrumentNameComp
        InstrumentNameSettings {}
    }

    Component {
        id: lyricsComp
        LyricsSettings {}
    }

    Component {
        id: restComp
        RestSettings {}
    }

    Component {
        id: dynamicComp
        DynamicsSettings {}
    }

    Component {
        id: expressionComp
        ExpressionsSettings {}
    }

    Component {
        id: stringTuningsComp
        StringTuningsSettings {}
    }

    Component {
        id: symbolComp
        SymbolSettings {}
    }

    Component {
        id: playCountTextComp
        PlayCountSettings {}
    }
}
