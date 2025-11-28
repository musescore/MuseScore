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
pragma ComponentBehavior: Bound   

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

    readonly property string viewObjectName: root.item ? root.item.objectName : ""

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

    Component {
        id: noteComp
        NoteSettings { 
            model: root.model as NoteSettingsProxyModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: fermataComp
        FermataSettings { 
            model: root.model as FermataSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: glissandoComp
        GlissandoSettings { 
            model: root.model as GlissandoSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: vibratoComp
        VibratoSettings { 
            model: root.model as VibratoSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: slurAndTieComp
        SlurAndTieSettings { 
            model: root.model as SlurAndTieSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: tempoComp
        TempoSettings { 
            model: root.model as TempoSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: aTempoComp
        TempoRestorePreviousSettings {  
            model: root.model as TempoSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: tempoPrimoComp
        TempoRestorePreviousSettings {  
            model: root.model as TempoSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: barlineComp
        BarlineSettings { 
            model: root.model as BarlineSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: sectionBreakComp
        SectionBreakSettings { 
            model: root.model as SectionBreakSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: markerComp
        MarkerSettings { 
            model: root.model as MarkerSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: jumpComp
        JumpSettings { 
            model: root.model as JumpSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: keySignatureComp
        KeySignatureSettings { 
            model: root.model as KeySignatureSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: accidentalComp
        AccidentalSettings { 
            model: root.model as AccidentalSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: fretDiagramComp
        FretDiagramSettings { 
            model: root.model as FretDiagramSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: spacerComp
        SpacerSettings { 
            model: root.model as SpacerSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: clefComp
        ClefSettings { 
            model: root.model as ClefSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: lineComp
        LineSettings { 
            model: root.model as TextLineSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: hairpinLineComp
        HairpinLineSettings { 
            model: root.model as HairpinLineSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: gradualTempoChangeComp
        GradualTempoChangeSettings {    
            model: root.model as GradualTempoChangeSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: staffTypeComp
        StaffTypeSettings { 
            model: root.model as StaffTypeSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: textFrameComp
        TextFrameSettings { 
            model: root.model as TextFrameSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: verticalFrameComp
        VerticalFrameSettings { 
            model: root.model as VerticalFrameSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: horizontalFrameComp
        HorizontalFrameSettings { 
            model: root.model as HorizontalFrameSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: fretFrameComp
        FretFrameSettings { 
            model: root.model as FretFrameSettingsProxyModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: articulationComp
        ArticulationSettings {  
            model: root.model as ArticulationSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: ornamentComp
        OrnamentSettings {  
            model: root.model as OrnamentSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: ambitusComp
        AmbitusSettings { 
            model: root.model as AmbitusSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: imageComp
        ImageSettings { 
            model: root.model as ImageSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: chordSymbolComp
        ChordSymbolSettings { 
            model: root.model as ChordSymbolSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: bracketComp
        BracketSettings {   
            model: root.model as BracketSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: timeSignatureComp
        TimeSignatureSettings { 
            model: root.model as TimeSignatureSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: mmRestComp
        MMRestSettings { 
            model: root.model as MMRestSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: bendComp
        BendSettings { 
            model: root.model as BendSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: tremoloBarComp
        TremoloBarSettings { 
            model: root.model as TremoloBarSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: tremoloComp
        TremoloSettings { 
            model: root.model as TremoloSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: measureRepeatComp
        MeasureRepeatSettings { 
            model: root.model as MeasureRepeatSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: tupletComp
        TupletSettings { 
            model: root.model as TupletSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: instrumentNameComp
        InstrumentNameSettings { 
            model: root.model as InstrumentNameSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: lyricsComp
        LyricsSettings { 
            model: root.model as LyricsSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: restComp
        RestSettings { 
            model: root.model as RestSettingsProxyModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: dynamicComp
        DynamicsSettings { 
            model: root.model as DynamicsSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: expressionComp
        ExpressionSettings { 
            model: root.model as ExpressionSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: stringTuningsComp
        StringTuningsSettings { 
            model: root.model as StringTuningsSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: symbolComp
        SymbolSettings {    
            model: root.model as SymbolSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }

    Component {
        id: playCountTextComp
        PlayCountSettings { 
            model: root.model as PlayCountTextSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }
    }
}
