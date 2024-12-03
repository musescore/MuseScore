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
import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

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

Loader {
    id: root

    property QtObject model: null

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
            case Inspector.TYPE_NOTE: return noteComp
            case Inspector.TYPE_BEAM: return noteComp
            case Inspector.TYPE_NOTEHEAD: return noteComp
            case Inspector.TYPE_STEM: return noteComp
            case Inspector.TYPE_HOOK: return noteComp
            case Inspector.TYPE_FERMATA: return fermataComp
            case Inspector.TYPE_GLISSANDO: return glissandoComp
            case Inspector.TYPE_VIBRATO: return vibratoComp
            case Inspector.TYPE_SLUR:
            case Inspector.TYPE_TIE:
            case Inspector.TYPE_LAISSEZ_VIB:
            case Inspector.TYPE_PARTIAL_TIE: return slurAndTieComp
            case Inspector.TYPE_TEMPO: return tempoComp
            case Inspector.TYPE_A_TEMPO: return aTempoComp
            case Inspector.TYPE_TEMPO_PRIMO: return tempoPrimoComp
            case Inspector.TYPE_BARLINE: return barlineComp
            case Inspector.TYPE_SECTIONBREAK: return sectionBreakComp
            case Inspector.TYPE_MARKER: return markerComp
            case Inspector.TYPE_JUMP: return jumpComp
            case Inspector.TYPE_KEYSIGNATURE: return keySignatureComp
            case Inspector.TYPE_ACCIDENTAL: return accidentalComp
            case Inspector.TYPE_FRET_DIAGRAM: return fretDiagramComp
            case Inspector.TYPE_SPACER: return spacerComp
            case Inspector.TYPE_CLEF: return clefComp
            case Inspector.TYPE_HAIRPIN:
            case Inspector.TYPE_CRESCENDO:
            case Inspector.TYPE_DIMINUENDO: return hairpinLineComp
            case Inspector.TYPE_PEDAL:
            case Inspector.TYPE_OTTAVA:
            case Inspector.TYPE_PALM_MUTE:
            case Inspector.TYPE_LET_RING:
            case Inspector.TYPE_VOLTA:
            case Inspector.TYPE_NOTELINE:
            case Inspector.TYPE_TEXT_LINE: return lineComp
            case Inspector.TYPE_GRADUAL_TEMPO_CHANGE: return gradualTempoChangeComp
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
            case Inspector.TYPE_TIME_SIGNATURE: return timeSignatureComp
            case Inspector.TYPE_MMREST: return mmRestComp
            case Inspector.TYPE_BEND: return bendComp
            case Inspector.TYPE_TREMOLOBAR: return tremoloBarComp
            case Inspector.TYPE_TREMOLO: return tremoloComp
            case Inspector.TYPE_MEASURE_REPEAT: return measureRepeatComp
            case Inspector.TYPE_TUPLET: return tupletComp
            case Inspector.TYPE_INSTRUMENT_NAME: return instrumentNameComp
            case Inspector.TYPE_LYRICS: return lyricsComp
            case Inspector.TYPE_REST: return restComp
            case Inspector.TYPE_REST_BEAM: return restComp
            case Inspector.TYPE_DYNAMIC: return dynamicComp
            case Inspector.TYPE_EXPRESSION: return expressionComp
            case Inspector.TYPE_STRING_TUNINGS: return stringTuningsComp
            case Inspector.TYPE_SYMBOL: return symbolComp
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
}
