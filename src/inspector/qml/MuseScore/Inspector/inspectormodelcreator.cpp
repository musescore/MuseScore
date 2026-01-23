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
#include "inspectormodelcreator.h"

#include "notation/notes/notesettingsproxymodel.h"
#include "notation/notes/noteheads/noteheadsettingsmodel.h"
#include "notation/notes/chords/chordsettingsmodel.h"
#include "notation/notes/beams/beamsettingsmodel.h"
#include "notation/notes/hooks/hooksettingsmodel.h"
#include "notation/notes/stems/stemsettingsmodel.h"
#include "notation/fermatas/fermatasettingsmodel.h"
#include "notation/tempos/temposettingsmodel.h"
#include "notation/lines/glissandosettingsmodel.h"
#include "notation/barlines/barlinesettingsmodel.h"
#include "notation/sectionbreaks/sectionbreaksettingsmodel.h"
#include "notation/markers/markersettingsmodel.h"
#include "notation/jumps/jumpsettingsmodel.h"
#include "notation/keysignatures/keysignaturesettingsmodel.h"
#include "notation/accidentals/accidentalsettingsmodel.h"
#include "notation/fretdiagrams/fretdiagramsettingsmodel.h"
#include "notation/lines/pedalsettingsmodel.h"
#include "notation/spacers/spacersettingsmodel.h"
#include "notation/clefs/clefsettingsmodel.h"
#include "notation/lines/hairpinsettingsmodel.h"
#include "notation/lines/hairpinlinesettingsmodel.h"
#include "notation/lines/ottavasettingsmodel.h"
#include "notation/lines/voltasettingsmodel.h"
#include "notation/lines/vibratosettingsmodel.h"
#include "notation/lines/slurandtiesettingsmodel.h"
#include "notation/lines/gradualtempochangesettingsmodel.h"
#include "notation/lines/notelinesettingsmodel.h"
#include "notation/stafftype/stafftypesettingsmodel.h"
#include "notation/frames/textframesettingsmodel.h"
#include "notation/frames/verticalframesettingsmodel.h"
#include "notation/frames/horizontalframesettingsmodel.h"
#include "notation/frames/fretframesettingsproxymodel.h"
#include "notation/frames/fretframe/fretframechordssettingsmodel.h"
#include "notation/frames/fretframe/fretframesettingsmodel.h"
#include "notation/articulations/articulationsettingsmodel.h"
#include "notation/ornaments/ornamentsettingsmodel.h"
#include "notation/ambituses/ambitussettingsmodel.h"
#include "notation/images/imagesettingsmodel.h"
#include "notation/chordsymbols/chordsymbolsettingsmodel.h"
#include "notation/brackets/bracketsettingsmodel.h"
#include "notation/timesignatures/timesignaturesettingsmodel.h"
#include "notation/mmrests/mmrestsettingsmodel.h"
#include "notation/bends/bendsettingsmodel.h"
#include "notation/tremolobars/tremolobarsettingsmodel.h"
#include "notation/tremolos/tremolosettingsmodel.h"
#include "notation/measurerepeats/measurerepeatsettingsmodel.h"
#include "notation/tuplets/tupletsettingsmodel.h"
#include "notation/instrumentname/instrumentnamesettingsmodel.h"
#include "notation/lyrics/lyricssettingsmodel.h"
#include "notation/lines/lyricslinesettingsmodel.h"
#include "notation/rests/beams/restbeamsettingsmodel.h"
#include "notation/rests/restsettingsmodel.h"
#include "notation/rests/restsettingsproxymodel.h"
#include "notation/dynamics/dynamicsettingsmodel.h"
#include "notation/expressions/expressionsettingsmodel.h"
#include "notation/stringtunings/stringtuningssettingsmodel.h"
#include "notation/symbols/symbolsettingsmodel.h"
#include "notation/playcounttext/playcounttextsettingsmodel.h"
#include "notation/lines/chordbracketsettingsmodel.h"

#include "general/playback/internal/arpeggioplaybackmodel.h"
#include "general/playback/internal/breathplaybackmodel.h"

using namespace mu::inspector;

AbstractInspectorModel* InspectorModelCreator::newInspectorModel(InspectorModelType modelType, QObject* parent,
                                                                 const muse::modularity::ContextPtr& iocCtx,
                                                                 IElementRepositoryService* repository)
{
    switch (modelType) {
    case InspectorModelType::TYPE_NOTE:
        return new NoteSettingsProxyModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_NOTEHEAD:
        return new NoteheadSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_CHORD:
        return new ChordSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_STEM:
        return new StemSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_HOOK:
        return new HookSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_BEAM:
        return new BeamSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_FERMATA:
        return new FermataSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_TEMPO:
    case InspectorModelType::TYPE_A_TEMPO:
    case InspectorModelType::TYPE_TEMPO_PRIMO:
        return new TempoSettingsModel(parent, iocCtx, repository, modelType);
    case InspectorModelType::TYPE_GLISSANDO:
        return new GlissandoSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_BARLINE:
        return new BarlineSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_PLAY_COUNT_TEXT:
        return new PlayCountTextSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_MARKER:
        return new MarkerSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_SECTIONBREAK:
        return new SectionBreakSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_JUMP:
        return new JumpSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_KEYSIGNATURE:
        return new KeySignatureSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_ACCIDENTAL:
        return new AccidentalSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_FRET_DIAGRAM:
        return new FretDiagramSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_PEDAL:
        return new PedalSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_SPACER:
        return new SpacerSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_CLEF:
        return new ClefSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_HAIRPIN:
        return new HairpinSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_CRESCENDO:
        return new HairpinLineSettingsModel(parent, iocCtx, repository, HairpinLineSettingsModel::Crescendo);
    case InspectorModelType::TYPE_DIMINUENDO:
        return new HairpinLineSettingsModel(parent, iocCtx, repository, HairpinLineSettingsModel::Diminuendo);
    case InspectorModelType::TYPE_OTTAVA:
        return new OttavaSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_VOLTA:
        return new VoltaSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_TEXT_LINE:
        return new TextLineSettingsModel(parent, iocCtx, repository);
    case mu::inspector::InspectorModelType::TYPE_NOTELINE:
        return new NoteLineSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_GRADUAL_TEMPO_CHANGE:
        return new GradualTempoChangeSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_VIBRATO:
        return new VibratoSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_SLUR:
        return new SlurAndTieSettingsModel(parent, iocCtx, repository, SlurAndTieSettingsModel::Slur);
    case InspectorModelType::TYPE_TIE:
        return new SlurAndTieSettingsModel(parent, iocCtx, repository, SlurAndTieSettingsModel::Tie);
    case InspectorModelType::TYPE_LAISSEZ_VIB:
        return new SlurAndTieSettingsModel(parent, iocCtx, repository, SlurAndTieSettingsModel::LaissezVib);
    case InspectorModelType::TYPE_PARTIAL_TIE:
        return new SlurAndTieSettingsModel(parent, iocCtx, repository, SlurAndTieSettingsModel::PartialTie);
    case InspectorModelType::TYPE_HAMMER_ON_PULL_OFF:
        return new SlurAndTieSettingsModel(parent, iocCtx, repository, SlurAndTieSettingsModel::HammerOnPullOff);
    case InspectorModelType::TYPE_STAFF_TYPE_CHANGES:
        return new StaffTypeSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_TEXT_FRAME:
        return new TextFrameSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_VERTICAL_FRAME:
        return new VerticalFrameSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_HORIZONTAL_FRAME:
        return new HorizontalFrameSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_FRET_FRAME:
        return new FretFrameSettingsProxyModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_FRET_FRAME_CHORDS:
        return new FretFrameChordsSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_FRET_FRAME_SETTINGS:
        return new FretFrameSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_ARTICULATION:
        return new ArticulationSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_TAPPING:
        return new ArticulationSettingsModel(parent, iocCtx, repository, InspectorModelType::TYPE_TAPPING);
    case InspectorModelType::TYPE_ORNAMENT:
        return new OrnamentSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_AMBITUS:
        return new AmbitusSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_IMAGE:
        return new ImageSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_CHORD_SYMBOL:
        return new ChordSymbolSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_BRACKET:
        return new BracketSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_TIME_SIGNATURE:
        return new TimeSignatureSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_MMREST:
        return new MMRestSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_BEND:
        return new BendSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_TREMOLOBAR:
        return new TremoloBarSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_TREMOLO:
        return new TremoloSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_MEASURE_REPEAT:
        return new MeasureRepeatSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_TUPLET:
        return new TupletSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_INSTRUMENT_NAME:
        return new InstrumentNameSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_LYRICS:
        return new LyricsSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_LYRICS_LINE:
        return new LyricsLineSettingsModel(parent, iocCtx, repository, LyricsLineSettingsModel::LyricsLine);
    case InspectorModelType::TYPE_PARTIAL_LYRICS_LINE:
        return new LyricsLineSettingsModel(parent, iocCtx, repository, LyricsLineSettingsModel::PartialLyricsLine);
    case InspectorModelType::TYPE_REST:
        return new RestSettingsProxyModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_REST_BEAM:
        return new RestBeamSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_REST_REST:
        return new RestSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_DYNAMIC:
        return new DynamicsSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_EXPRESSION:
        return new ExpressionSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_STRING_TUNINGS:
        return new StringTuningsSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_SYMBOL:
        return new SymbolSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_CHORD_BRACKET:
        return new ChordBracketSettingsModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_BREATH:
        return new BreathPlaybackModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_ARPEGGIO:
        return new ArpeggioPlaybackModel(parent, iocCtx, repository);
    case InspectorModelType::TYPE_UNDEFINED:
        break;
    }

    return nullptr;
}
