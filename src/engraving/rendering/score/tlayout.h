/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_ENGRAVING_TLAYOUT_DEV_H
#define MU_ENGRAVING_TLAYOUT_DEV_H

#include "layoutcontext.h"

#include "../../dom/accidental.h"
#include "../../dom/actionicon.h"
#include "../../dom/ambitus.h"
#include "../../dom/arpeggio.h"
#include "../../dom/articulation.h"

#include "../../dom/barline.h"
#include "../../dom/bend.h"
#include "../../dom/box.h"
#include "../../dom/bracket.h"
#include "../../dom/breath.h"

#include "../../dom/chordline.h"
#include "../../dom/clef.h"
#include "../../dom/capo.h"
#include "../../dom/chord.h"

#include "../../dom/deadslapped.h"
#include "../../dom/dynamic.h"

#include "../../dom/expression.h"

#include "../../dom/fermata.h"
#include "../../dom/figuredbass.h"
#include "../../dom/fingering.h"
#include "../../dom/fret.h"

#include "../../dom/guitarbend.h"

#include "../../dom/harppedaldiagram.h"
#include "../../dom/harmony.h"
#include "../../dom/hairpin.h"
#include "../../dom/hook.h"

#include "../../dom/image.h"
#include "../../dom/instrchange.h"
#include "../../dom/instrumentname.h"

#include "../../dom/jump.h"

#include "../../dom/keysig.h"

#include "../../dom/laissezvib.h"
#include "../../dom/layoutbreak.h"

#include "../../dom/marker.h"
#include "../../dom/measurebase.h"
#include "../../dom/measurenumber.h"
#include "../../dom/measurenumberbase.h"
#include "../../dom/measurerepeat.h"
#include "../../dom/mmrest.h"
#include "../../dom/mmrestrange.h"

#include "../../dom/note.h"
#include "../../dom/notedot.h"

#include "../../dom/parenthesis.h"
#include "../../dom/playcounttext.h"
#include "../../dom/playtechannotation.h"

#include "../../dom/rehearsalmark.h"
#include "../../dom/rest.h"

#include "../../dom/staffstate.h"
#include "../../dom/stafftext.h"
#include "../../dom/stafftypechange.h"
#include "../../dom/stem.h"
#include "../../dom/stemslash.h"
#include "../../dom/sticking.h"
#include "../../dom/systemdivider.h"
#include "../../dom/systemtext.h"
#include "../../dom/soundflag.h"

#include "../../dom/tapping.h"
#include "../../dom/textbase.h"
#include "../../dom/tempotext.h"
#include "../../dom/text.h"
#include "../../dom/timesig.h"
#include "../../dom/anchors.h"
#include "../../dom/tremolobar.h"
#include "../../dom/tripletfeel.h"
#include "../../dom/trill.h"
#include "../../dom/tuplet.h"

#include "../../dom/ornament.h"

namespace mu::engraving {
class EngravingItem;

class BagpipeEmbellishment;
class Beam;

class Chord;

class Glissando;
class GlissandoSegment;
class GradualTempoChangeSegment;
class GradualTempoChange;

class HairpinSegment;
class Hairpin;
class HammerOnPullOff;
class HammerOnPullOffSegment;
class HammerOnPullOffText;
class HarmonicMarkSegment;

class LedgerLine;
class LetRing;
class LetRingSegment;
class LineSegment;
class Lyrics;
class LyricsLine;
class LyricsLineSegment;

class Ottava;
class OttavaSegment;

class PalmMute;
class PalmMuteSegment;
class Parenthesis;
class Pedal;
class PedalSegment;
class PickScrapeSegment;

class NoteLineSegment;

class RasgueadoSegment;

class ShadowNote;
class SLine;
class Slur;
class Spacer;
class SpannerSegment;
class StaffLines;
class StringTunings;

class BSymbol;
class Symbol;
class FSymbol;

class SystemDivider;
class SystemText;

class TabDurationSymbol;
class Tapping;
class TempoText;
class Text;
class TextLine;
class TextLineSegment;
class TextLineBase;
class TextLineBaseSegment;
class Tie;
class TimeSig;
class TimeTickAnchor;
class TremoloSingleChord;
class TremoloTwoChord;
class TremoloBar;
class TrillSegment;
class TripletFeel;
class Trill;
class Tuplet;

class VibratoSegment;
class Vibrato;
class Volta;
class VoltaSegment;

class WhammyBarSegment;
}

namespace mu::engraving::rendering::score {
class TLayout
{
public:

    static void layoutItem(EngravingItem* item, LayoutContext& ctx);  // factory

    static void layoutAccidental(const Accidental* item, Accidental::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layoutActionIcon(const ActionIcon* item, ActionIcon::LayoutData* ldata);
    static void layoutAmbitus(const Ambitus* item, Ambitus::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutArpeggio(const Arpeggio* item, Arpeggio::LayoutData* ldata, const LayoutConfiguration& conf,
                               bool includeCrossStaffHeight = false);
    static void layoutArticulation(Articulation* item, Articulation::LayoutData* ldata);
    static void fillArticulationShape(const Articulation* item, Articulation::LayoutData* ldata);

    static void layoutBarLine(const BarLine* item, BarLine::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutBarLine2(BarLine* item, LayoutContext& ctx);
    static void layoutBeam(Beam* item, const LayoutContext& ctx);
    static void layoutBeam1(Beam* item, LayoutContext& ctx);
    static void layoutBend(const Bend* item, Bend::LayoutData* ldata);

    static void layoutBox(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx); // factory
    static void layoutBaseBox(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx); // base class
    static void layoutHBox(const HBox* item, HBox::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutHBox2(HBox* item, const LayoutContext& ctx);
    static void layoutVBox(const VBox* item, VBox::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutFBox(const FBox* item, FBox::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutTBox(const TBox* item, TBox::LayoutData* ldata, const LayoutContext& ctx);

    static void layoutBracket(const Bracket* item, Bracket::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layoutBreath(const Breath* item, Breath::LayoutData* ldata, const LayoutConfiguration& conf);

    static void layoutChord(Chord* item, LayoutContext& ctx);
    static void layoutChordLine(const ChordLine* item, ChordLine::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layoutClef(const Clef* item, Clef::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layoutCapo(const Capo* item, Capo::LayoutData* ldata, const LayoutContext& ctx);

    static void layoutDeadSlapped(const DeadSlapped* item, DeadSlapped::LayoutData* ldata);
    static void layoutDynamic(Dynamic* item, Dynamic::LayoutData* ldata, const LayoutConfiguration& conf);

    static void layoutExpression(const Expression* item, Expression::LayoutData* ldata);

    static void layoutFermata(const Fermata* item, Fermata::LayoutData* ldata);
    static void layoutFiguredBass(const FiguredBass* item, FiguredBass::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutFingering(const Fingering* item, Fingering::LayoutData* ldata);
    static void layoutFretDiagram(const FretDiagram* item, FretDiagram::LayoutData* ldata, const LayoutContext& ctx);

    static void layoutGlissando(Glissando* item, LayoutContext& ctx);
    static void layoutGlissandoSegment(GlissandoSegment* item, LayoutContext& ctx);
    static void layoutGraceNotesGroup(GraceNotesGroup* item, LayoutContext& ctx);
    static void layoutGraceNotesGroup2(const GraceNotesGroup* item, GraceNotesGroup::LayoutData* ldata);
    static void layoutGradualTempoChangeSegment(GradualTempoChangeSegment* item, LayoutContext& ctx);
    static void layoutGradualTempoChange(GradualTempoChange* item, LayoutContext& ctx);
    static void layoutGuitarBend(GuitarBend* item, LayoutContext& ctx);
    static void layoutGuitarBendSegment(GuitarBendSegment* item, LayoutContext& ctx);
    static void fillGuitarBendSegmentShape(const GuitarBendSegment* item, GuitarBendSegment::LayoutData* ldata);

    static void layoutHairpinSegment(HairpinSegment* item, LayoutContext& ctx);
    static void layoutHairpin(Hairpin* item, LayoutContext& ctx);
    static void layoutHammerOnPullOff(HammerOnPullOff* item, LayoutContext& ctx);
    static void layoutHammerOnPullOffSegment(HammerOnPullOffSegment* item, LayoutContext& ctx);
    static void layoutHammerOnPullOffText(HammerOnPullOffText* item, LayoutContext& ctx);
    static void fillHairpinSegmentShape(const HairpinSegment* item, HairpinSegment::LayoutData* ldata);
    static void layoutHarpPedalDiagram(const HarpPedalDiagram* item, HarpPedalDiagram::LayoutData* ldata);
    static void layoutHarmonicMarkSegment(HarmonicMarkSegment* item, LayoutContext& ctx);
    static void layoutHarmony(Harmony* item, Harmony::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutHook(const Hook* item, Hook::LayoutData* ldata);

    static void layoutIndicatorIcon(const IndicatorIcon* item, IndicatorIcon::LayoutData* ldata);

    static void layoutImage(const Image* item, Image::LayoutData* ldata);
    static void layoutInstrumentChange(const InstrumentChange* item, InstrumentChange::LayoutData* ldata);
    static void layoutInstrumentName(const InstrumentName* item, InstrumentName::LayoutData* ldata);

    static void layoutJump(const Jump* item, Jump::LayoutData* ldata);

    static void layoutKeySig(const KeySig* item, KeySig::LayoutData* ldata, const LayoutConfiguration& conf);

    static void layoutLaissezVib(LaissezVib* item);
    static void layoutLayoutBreak(const LayoutBreak* item, LayoutBreak::LayoutData* ldata);
    static void layoutLedgerLine(LedgerLine* item, LayoutContext& ctx);
    static void layoutLetRing(LetRing* item, LayoutContext& ctx);
    static void layoutLetRingSegment(LetRingSegment* item, LayoutContext& ctx);
    static void layoutLineSegment(LineSegment* item, LayoutContext& ctx);  // factory
    static void layoutLyrics(Lyrics* item, LayoutContext& ctx);
    static void layoutLyricsLine(LyricsLine* item, LayoutContext& ctx);
    static void layoutLyricsLineSegment(LyricsLineSegment* item, LayoutContext& ctx);

    static void layoutMarker(const Marker* item, Marker::LayoutData* ldata, LayoutContext& ctx);
    static void layoutMeasureBase(MeasureBase* item, LayoutContext& ctx); // factory
    static void layoutBaseMeasureBase(const MeasureBase* item, MeasureBase::LayoutData* ldata, const LayoutContext& ctx); // base class
    static void layoutMeasureNumber(const MeasureNumber* item, MeasureNumber::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutMeasureRepeat(const MeasureRepeat* item, MeasureRepeat::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutMeasureRepeatExtender(const MeasureRepeat* item, MeasureRepeat::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutMMRest(const MMRest* item, MMRest::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutMMRestRange(const MMRestRange* item, MMRestRange::LayoutData* ldata, const LayoutContext& ctx);

    static void layoutNote(const Note* item, Note::LayoutData* ldata);
    static void fillNoteShape(const Note* item, Note::LayoutData* ldata);
    static void layoutNoteDot(const NoteDot* item, NoteDot::LayoutData* ldata);
    static void layoutNoteAnchoredLine(SLine* item, SLine::LayoutData* ldata, LayoutContext& ctx);
    static void layoutNoteLine(NoteLine* item, LayoutContext& ctx);
    static void layoutNoteLineSegment(NoteLineSegment* item, LayoutContext& ctx);

    static void layoutOrnament(const Ornament* item, Ornament::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layoutOrnamentCueNote(Ornament* item, LayoutContext& ctx);

    static void layoutOttava(Ottava* item, LayoutContext& ctx);
    static void layoutOttavaSegment(OttavaSegment* item, LayoutContext& ctx);

    static void layoutPalmMute(PalmMute* item, LayoutContext& ctx);
    static void layoutPalmMuteSegment(PalmMuteSegment* item, LayoutContext& ctx);
    static void layoutParenthesis(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutPedal(Pedal* item, LayoutContext& ctx);
    static void layoutPedalSegment(PedalSegment* item, LayoutContext& ctx);
    static void layoutPickScrapeSegment(PickScrapeSegment* item, LayoutContext& ctx);
    static void layoutPlayCountText(PlayCountText* item, PlayCountText::LayoutData* ldata);
    static void layoutPlayTechAnnotation(const PlayTechAnnotation* item, PlayTechAnnotation::LayoutData* ldata);

    static void layoutRasgueadoSegment(RasgueadoSegment* item, LayoutContext& ctx);
    static void layoutRehearsalMark(const RehearsalMark* item, RehearsalMark::LayoutData* ldata);
    static void layoutRest(const Rest* item, Rest::LayoutData* ldata, const LayoutContext& ctx);

    static void layoutShadowNote(ShadowNote* item, LayoutContext& ctx);
    static void layoutLine(SLine* item, LayoutContext& ctx); // base class
    static void layoutSlur(Slur* item, LayoutContext& ctx);
    static void layoutSpacer(Spacer* item, LayoutContext& ctx);
    static void layoutSpanner(Spanner* item, LayoutContext& ctx);
    static void layoutStaffLines(StaffLines* item, LayoutContext& ctx);
    static void layoutForWidth(StaffLines* item, double w, LayoutContext& ctx);
    static void layoutStaffState(const StaffState* item, StaffState::LayoutData* ldata);
    static void layoutStaffText(const StaffText* item, StaffText::LayoutData* ldata);
    static void layoutStaffTypeChange(const StaffTypeChange* item, StaffTypeChange::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layoutStem(const Stem* item, Stem::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layoutStemSlash(const StemSlash* item, StemSlash::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layoutSticking(const Sticking* item, Sticking::LayoutData* ldata);
    static void layoutStringTunings(StringTunings* item, LayoutContext& ctx);
    static void layoutSoundFlag(const SoundFlag* item, SoundFlag::LayoutData* ldata);

    static void layoutSymbol(const Symbol* item, Symbol::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutFSymbol(const FSymbol* item, FSymbol::LayoutData* ldata);

    static void layoutSystemDivider(const SystemDivider* item, SystemDivider::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutSystemText(const SystemText* item, SystemText::LayoutData* ldata);

    static void layoutTabDurationSymbol(const TabDurationSymbol* item, TabDurationSymbol::LayoutData* ldata);
    static void layoutTapping(Tapping* item, Tapping::LayoutData* ldata, LayoutContext& ctx);
    static void layoutTappingHalfSlur(TappingHalfSlur* item);
    static void layoutTempoText(const TempoText* item, TempoText::LayoutData* ldata);

    static void layoutTextBase(TextBase* item, LayoutContext& ctx);                 // factory
    static void layoutBaseTextBase(const TextBase* item, TextBase::LayoutData* data); // base class
    static void layoutBaseTextBase(TextBase* item, LayoutContext& ctx);  // base class
    static void layoutBaseTextBase1(TextBase* item, const LayoutContext& ctx);  // base class
    static void layoutBaseTextBase1(const TextBase* item, TextBase::LayoutData* data);
    static void computeTextHighResShape(const TextBase* item, TextBase::LayoutData* ldata);

    static void layoutText(const Text* item, Text::LayoutData* ldata);

    static void layoutTextLine(TextLine* item, LayoutContext& ctx);
    static void layoutTextLineSegment(TextLineSegment* item, LayoutContext& ctx);
    static void layoutTextLineBase(TextLineBase* item, LayoutContext& ctx);
    static void layoutTextLineBaseSegment(TextLineBaseSegment* item, LayoutContext& ctx); // base class
    static void layoutTie(Tie* item, LayoutContext& ctx);
    static void layoutTimeSig(const TimeSig* item, TimeSig::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutTimeTickAnchor(TimeTickAnchor* item, LayoutContext&);
    static void layoutTremoloSingle(TremoloSingleChord* item, LayoutContext& ctx);
    static void layoutTremoloTwo(TremoloTwoChord* item, LayoutContext& ctx);
    static void layoutTremoloBar(const TremoloBar* item, TremoloBar::LayoutData* ldata);
    static void layoutTrill(Trill* item, LayoutContext& ctx);
    static void layoutTrillSegment(TrillSegment* item, LayoutContext& ctx);
    static void fillTrillSegmentShape(const TrillSegment* item, TrillSegment::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layoutTripletFeel(const TripletFeel* item, TripletFeel::LayoutData* ldata);
    static void layoutTuplet(Tuplet* item, LayoutContext& ctx);
    static void fillTupletShape(const Tuplet* item, Tuplet::LayoutData* ldata);

    static void layoutVibrato(Vibrato* item, LayoutContext& ctx);
    static void layoutVibratoSegment(VibratoSegment* item, LayoutContext& ctx);
    static void layoutVolta(Volta* item, LayoutContext& ctx);
    static void layoutVoltaSegment(VoltaSegment* item, LayoutContext& ctx);

    static void layoutWhammyBarSegment(WhammyBarSegment* item, LayoutContext& ctx);

    static void updateBarlineShape(const BarLine* item, BarLine::LayoutData* ldata, const LayoutContext& ctx);

    // layoutSystem;
    static SpannerSegment* layoutSystem(Spanner* item, System* system, LayoutContext& ctx); // factory
    static SpannerSegment* layoutSystem(LyricsLine* line, System* system, LayoutContext& ctx);
    static SpannerSegment* layoutSystem(Volta* line, System* system, LayoutContext& ctx);
    static SpannerSegment* layoutSystem(Slur* line, System* system, LayoutContext& ctx);
    static void layoutSystemsDone(Spanner* item);

private:

    friend class SlurTieLayout;

    static void layoutFiguredBassItem(const FiguredBassItem* item, FiguredBassItem::LayoutData* ldata, const LayoutContext& ctx);

    static SpannerSegment* layoutSystemSLine(SLine* line, System* system, LayoutContext& ctx);
    static double voltaMidEndSegmentStartX(Volta* volta, System* system, LayoutContext& ctx);
    static SpannerSegment* getNextLayoutSystemSegment(Spanner* spanner, System* system,
                                                      std::function<SpannerSegment* (System* parent)> createSegment);

    static Shape textLineBaseSegmentShape(const TextLineBaseSegment* item);

    static void manageHairpinSnapping(HairpinSegment* item, LayoutContext& ctx);

    static void checkRehearsalMarkVSBigTimeSig(const RehearsalMark* item, RehearsalMark::LayoutData* ldata);
};
}

#endif // MU_ENGRAVING_TLAYOUT_DEV_H
