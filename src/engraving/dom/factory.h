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

#pragma once

#include <memory>

#include "engravingitem.h"
#include "durationtype.h"
#include "types.h"

namespace mu::engraving {
class Instrument;
class RootItem;

class TremoloTwoChord;
class TremoloSingleChord;

class SoundFlag;
class SystemLock;

class Factory
{
public:

    static EngravingItem* createItem(ElementType type, EngravingItem* parent, bool isAccessibleEnabled = true);
    static EngravingItem* createItemByName(const AsciiStringView& name, EngravingItem* parent, bool isAccessibleEnabled = true);

    static Accidental* createAccidental(EngravingItem* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Accidental> makeAccidental(EngravingItem* parent);

    static Ambitus* createAmbitus(Segment* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Ambitus> makeAmbitus(Segment* parent);

    static Arpeggio* createArpeggio(Chord* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Arpeggio> makeArpeggio(Chord* parent);

    static Articulation* createArticulation(ChordRest* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Articulation> makeArticulation(ChordRest* parent);

    static Ornament* createOrnament(ChordRest* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Ornament> makeOrnament(ChordRest* parent);

    static BarLine* createBarLine(Segment* parent, bool isAccessibleEnabled = true);
    static BarLine* copyBarLine(const BarLine& src);
    static std::shared_ptr<BarLine> makeBarLine(Segment* parent);

    static Beam* createBeam(System* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Beam> makeBeam(System* parent);

    static Bend* createBend(Note* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Bend> makeBend(Note* parent);

    static Bracket* createBracket(EngravingItem* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Bracket> makeBracket(EngravingItem* parent);
    static BracketItem* createBracketItem(EngravingItem* parent);
    static BracketItem* createBracketItem(EngravingItem* parent, BracketType a, int b);

    static Breath* createBreath(Segment* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Breath> makeBreath(Segment* parent);

    static Chord* createChord(Segment* parent, bool isAccessibleEnabled = true);
    static Chord* copyChord(const Chord& src, bool link = false);
    static std::shared_ptr<Chord> makeChord(Segment* parent);

    static ChordLine* createChordLine(Chord* parent, bool isAccessibleEnabled = true);
    static ChordLine* copyChordLine(const ChordLine& src);
    static std::shared_ptr<ChordLine> makeChordLine(Chord* parent);

    static Clef* createClef(Segment* parent, bool isAccessibleEnabled = true);
    static Clef* copyClef(const Clef& src);
    static std::shared_ptr<Clef> makeClef(Segment* parent);

    static Fermata* createFermata(Segment* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Fermata> makeFermata(Segment* parent);

    static FiguredBass* createFiguredBass(Segment* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<FiguredBass> makeFiguredBass(Segment* parent);

    static FretDiagram* createFretDiagram(Segment* parent, bool isAccessibleEnabled = true);
    static FretDiagram* copyFretDiagram(const FretDiagram& src);
    static std::shared_ptr<FretDiagram> makeFretDiagram(Segment* parent);

    static HarpPedalDiagram* createHarpPedalDiagram(Segment* parent, bool isAccessibleEnabled = true);
    static HarpPedalDiagram* copyHarpPedalDiagram(const HarpPedalDiagram& src);
    static std::shared_ptr<HarpPedalDiagram> makeHarpPedalDiagram(Segment* parent);

    static KeySig* createKeySig(Segment* parent, bool isAccessibleEnabled = true);
    static KeySig* copyKeySig(const KeySig& src);
    static std::shared_ptr<KeySig> makeKeySig(Segment* parent);

    static LaissezVib* createLaissezVib(Note* parent, bool isAccessibleEnabled = true);
    static LaissezVib* copyLaissezVib(const LaissezVib& src);

    static LayoutBreak* createLayoutBreak(MeasureBase* parent, bool isAccessibleEnabled = true);
    static LayoutBreak* copyLayoutBreak(const LayoutBreak& src);
    static std::shared_ptr<LayoutBreak> makeLayoutBreak(MeasureBase* parent);

    static SystemLockIndicator* createSystemLockIndicator(System* parent, const SystemLock* lock, bool isAccessibleEnabled = true);
    static SystemLockIndicator* copySystemLockIndicator(const SystemLockIndicator& src);

    static Lyrics* createLyrics(ChordRest* parent, bool isAccessibleEnabled = true);
    static Lyrics* copyLyrics(const Lyrics& src);

    static Measure* createMeasure(System* parent, bool isAccessibleEnabled = true);
    static Measure* copyMeasure(const Measure& src);

    static MeasureRepeat* createMeasureRepeat(Segment* parent, bool isAccessibleEnabled = true);
    static MeasureRepeat* copyMeasureRepeat(const MeasureRepeat& src);

    static Note* createNote(Chord* parent, bool isAccessibleEnabled = true);
    static Note* copyNote(const Note& src, bool link = false);
    static std::shared_ptr<Note> makeNote(Chord* parent);

    static NoteDot* createNoteDot(Note* parent, bool isAccessibleEnabled = true);
    static NoteDot* createNoteDot(Rest* parent, bool isAccessibleEnabled = true);
    static NoteDot* copyNoteDot(const NoteDot& src);

    static NoteLine* createNoteLine(Note* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<NoteLine> makeNoteLine(Note* parent);

    static Page* createPage(RootItem* parent, bool isAccessibleEnabled = true);

    static Parenthesis* createParenthesis(Segment* parent, bool isAccessibleEnabled = true);
    static Parenthesis* copyParenthesis(const Parenthesis& src);

    static PartialTie* createPartialTie(Note* parent, bool isAccessibleEnabled = true);
    static PartialTie* copyPartialTie(const PartialTie& src);

    static PartialLyricsLine* createPartialLyricsLine(EngravingItem* parent, bool isAccessibleEnabled = true);
    static PartialLyricsLine* copyPartialLyricsLine(const PartialLyricsLine& src);

    static Rest* createRest(Segment* parent, bool isAccessibleEnabled = true);
    static Rest* createRest(Segment* parent, const TDuration& t, bool isAccessibleEnabled = true);
    static Rest* copyRest(const Rest& src, bool link = false);
    static MMRest* createMMRest(EngravingItem* parent, bool isAccessibleEnabled = true);

    static DeadSlapped* createDeadSlapped(Rest* parent, bool isAccessibleEnabled = true);
    static DeadSlapped* copyDeadSlapped(const DeadSlapped& src);

    static Segment* createSegment(Measure* parent, bool isAccessibleEnabled = true);
    static Segment* createSegment(Measure* parent, SegmentType type, const Fraction& t, bool isAccessibleEnabled = true);

    static Slur* createSlur(EngravingItem* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Slur> makeSlur(EngravingItem* parent);

    static Spacer* createSpacer(Measure* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Spacer> makeSpacer(Measure* parent);

    static Staff* createStaff(Part* parent);

    static StaffLines* createStaffLines(Measure* parent, bool isAccessibleEnabled = true);
    static StaffLines* copyStaffLines(const StaffLines& src);

    static StaffState* createStaffState(EngravingItem* parent, bool isAccessibleEnabled = true);

    static StaffTypeChange* createStaffTypeChange(MeasureBase* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<StaffTypeChange> makeStaffTypeChange(MeasureBase* parent);

    static StaffText* createStaffText(Segment* parent, TextStyleType textStyleType = TextStyleType::STAFF, bool isAccessibleEnabled = true);

    static SoundFlag* createSoundFlag(EngravingItem* parent, bool isAccessibleEnabled = true);

    static Expression* createExpression(Segment* parent, bool isAccessibleEnabled = true);

    static RehearsalMark* createRehearsalMark(Segment* parent, bool isAccessibleEnabled = true);

    static Stem* createStem(Chord* parent, bool isAccessibleEnabled = true);
    static Stem* copyStem(const Stem& src);

    static StemSlash* createStemSlash(Chord* parent, bool isAccessibleEnabled = true);
    static StemSlash* copyStemSlash(const StemSlash& src);

    static System* createSystem(Page* parent, bool isAccessibleEnabled = true);
    static SystemText* createSystemText(Segment* parent, TextStyleType textStyleType = TextStyleType::SYSTEM,
                                        ElementType type = ElementType::SYSTEM_TEXT, bool isAccessibleEnabled = true);

    static InstrumentChange* createInstrumentChange(Segment* parent, bool isAccessibleEnabled = true);
    static InstrumentChange* createInstrumentChange(Segment* parent, const Instrument& instrument, bool isAccessibleEnabled = true);

    static Sticking* createSticking(Segment* parent, bool isAccessibleEnabled = true);

    static Fingering* createFingering(Note* parent, bool isAccessibleEnabled = true);
    static Fingering* createFingering(Note* parent, TextStyleType textStyleType, bool isAccessibleEnabled = true);

    static Harmony* createHarmony(Segment* parent, bool isAccessibleEnabled = true);

    static TempoText* createTempoText(Segment* parent, bool isAccessibleEnabled = true);

    static Text* createText(EngravingItem* parent, TextStyleType tid = TextStyleType::DEFAULT, bool isAccessibleEnabled = true);
    static Text* copyText(const Text& src);

    static Tie* createTie(EngravingItem* parent, bool isAccessibleEnabled = true);
    static Tie* copyTie(const Tie& src);

    static TimeSig* createTimeSig(Segment* parent, bool isAccessibleEnabled = true);
    static TimeSig* copyTimeSig(const TimeSig& src);
    static std::shared_ptr<TimeSig> makeTimeSig(Segment* parent);

    static TremoloTwoChord* createTremoloTwoChord(Chord* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<TremoloTwoChord> makeTremoloTwoChord(Chord* parent);
    static TremoloTwoChord* copyTremoloTwoChord(const TremoloTwoChord& src);

    static TremoloSingleChord* createTremoloSingleChord(Chord* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<TremoloSingleChord> makeTremoloSingleChord(Chord* parent);
    static TremoloSingleChord* copyTremoloSingleChord(const TremoloSingleChord& src);

    static TremoloBar* createTremoloBar(EngravingItem* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<TremoloBar> makeTremoloBar(EngravingItem* parent);

    static TripletFeel* createTripletFeel(Segment* parent, TripletFeelType type = TripletFeelType::NONE, bool isAccessibleEnabled = true);

    static Tuplet* createTuplet(Measure* parent, bool isAccessibleEnabled = true);
    static Tuplet* copyTuplet(const Tuplet& src);

    static Hairpin* createHairpin(Segment* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Hairpin> makeHairpin(Segment* parent);

    static Glissando* createGlissando(EngravingItem* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Glissando> makeGlissando(EngravingItem* parent);

    static GuitarBend* createGuitarBend(Note* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<GuitarBend> makeGuitarBend(Note* parent);

    static Jump* createJump(Measure* parent, bool isAccessibleEnabled = true);

    static Trill* createTrill(EngravingItem* parent, bool isAccessibleEnabled = true);

    static Vibrato* createVibrato(EngravingItem* parent, bool isAccessibleEnabled = true);

    static TextLine* createTextLine(EngravingItem* parent, bool isAccessibleEnabled = true);

    static Ottava* createOttava(EngravingItem* parent, bool isAccessibleEnabled = true);

    static LetRing* createLetRing(EngravingItem* parent, bool isAccessibleEnabled = true);

    static Marker* createMarker(EngravingItem* parent, bool isAccessibleEnabled = true);

    static Marker* createMarker(EngravingItem* parent, TextStyleType tid, bool isAccessibleEnabled = true);

    static GradualTempoChange* createGradualTempoChange(EngravingItem* parent, bool isAccessibleEnabled = true);

    static PalmMute* createPalmMute(EngravingItem* parent, bool isAccessibleEnabled = true);

    static WhammyBar* createWhammyBar(EngravingItem* parent, bool isAccessibleEnabled = true);

    static Rasgueado* createRasgueado(EngravingItem* parent, bool isAccessibleEnabled = true);

    static HarmonicMark* createHarmonicMark(EngravingItem* parent, bool isAccessibleEnabled = true);

    static PickScrape* createPickScrape(EngravingItem* parent, bool isAccessibleEnabled = true);

    static Volta* createVolta(EngravingItem* parent, bool isAccessibleEnabled = true);

    static Pedal* createPedal(EngravingItem* parent, bool isAccessibleEnabled = true);

    static Dynamic* createDynamic(Segment* parent, bool isAccessibleEnabled = true);

    static Harmony* createHarmony(EngravingItem* parent, bool isAccessibleEnabled = true);

    static VBox* createVBox(System* parent, bool isAccessibleEnabled = true);

    static VBox* createVBox(const ElementType& type, System* parent, bool isAccessibleEnabled = true);

    static VBox* createTitleVBox(System* parent, bool isAccessibleEnabled = true);

    static HBox* createHBox(System* parent, bool isAccessibleEnabled = true);

    static TBox* createTBox(System* parent, bool isAccessibleEnabled = true);

    static FBox* createFBox(System* parent, bool isAccessibleEnabled = true);

    static Image* createImage(EngravingItem* parent);

    static Symbol* createSymbol(EngravingItem* parent, bool isAccessibleEnabled = true);
    static FSymbol* createFSymbol(EngravingItem* parent, bool isAccessibleEnabled = true);

    static PlayTechAnnotation* createPlayTechAnnotation(Segment* parent, PlayingTechniqueType techniqueType, TextStyleType styleType,
                                                        bool isAccessibleEnabled = true);

    static Capo* createCapo(Segment* parent, bool isAccessibleEnabled = true);

    static TimeTickAnchor* createTimeTickAnchor(Segment* parent, bool isAccessibleEnabled = true);

    static StringTunings* createStringTunings(Segment* parent, bool isAccessibleEnabled = true);
    static StringTunings* copyStringTunings(const StringTunings& src);
    static std::shared_ptr<StringTunings> makeStringTunings(Segment* parent);

private:
    static EngravingItem* doCreateItem(ElementType type, EngravingItem* parent);
};
}
