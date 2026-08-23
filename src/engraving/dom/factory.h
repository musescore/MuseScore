/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
namespace mu::engraving {
class Instrument;
class RangeLock;
class TremoloSingleChord;
class TremoloTwoChord;

enum class SegmentType;
enum class TripletFeelType : unsigned char;

class Factory
{
public:

    static EngravingItem* createItem(ElementType type, DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static EngravingItem* createItemByName(const AsciiStringView& name, DummyParentOr<EngravingItem> parent,
                                           bool isAccessibleEnabled = true);

    static Accidental* createAccidental(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Accidental> makeAccidental(DummyParentOr<EngravingItem> parent);

    static Ambitus* createAmbitus(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Ambitus> makeAmbitus(DummyParentOr<Segment> parent);

    static Arpeggio* createArpeggio(DummyParentOr<Chord> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Arpeggio> makeArpeggio(DummyParentOr<Chord> parent);

    static ChordBracket* createChordBracket(DummyParentOr<Chord> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<ChordBracket> makeChordBracket(DummyParentOr<Chord> parent);

    static Articulation* createArticulation(DummyParentOr<ChordRest> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Articulation> makeArticulation(DummyParentOr<ChordRest> parent);

    static Tapping* createTapping(DummyParentOr<ChordRest> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Tapping> makeTapping(DummyParentOr<ChordRest> parent);

    static Ornament* createOrnament(DummyParentOr<ChordRest> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Ornament> makeOrnament(DummyParentOr<ChordRest> parent);

    static BarLine* createBarLine(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static BarLine* copyBarLine(const BarLine& src);
    static std::shared_ptr<BarLine> makeBarLine(DummyParentOr<Segment> parent);

    static Beam* createBeam(Score* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Beam> makeBeam(Score* parent);

    static Bend* createBend(DummyParentOr<Note> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Bend> makeBend(DummyParentOr<Note> parent);

    static Bracket* createBracket(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Bracket> makeBracket(DummyParentOr<EngravingItem> parent);
    static BracketItem* createBracketItem(DummyParentOr<EngravingItem> parent);
    static BracketItem* createBracketItem(DummyParentOr<EngravingItem> parent, BracketType bracketType, size_t span);

    static Breath* createBreath(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Breath> makeBreath(DummyParentOr<Segment> parent);

    static Chord* createChord(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static Chord* copyChord(const Chord& src, bool link = false);
    static std::shared_ptr<Chord> makeChord(DummyParentOr<Segment> parent);

    static ChordLine* createChordLine(DummyParentOr<Chord> parent, bool isAccessibleEnabled = true);
    static ChordLine* copyChordLine(const ChordLine& src);
    static std::shared_ptr<ChordLine> makeChordLine(DummyParentOr<Chord> parent);

    static Clef* createClef(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static Clef* copyClef(const Clef& src);
    static std::shared_ptr<Clef> makeClef(DummyParentOr<Segment> parent);

    static Fermata* createFermata(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Fermata> makeFermata(DummyParentOr<Segment> parent);

    static FiguredBass* createFiguredBass(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<FiguredBass> makeFiguredBass(DummyParentOr<Segment> parent);

    static FretDiagram* createFretDiagram(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static FretDiagram* copyFretDiagram(const FretDiagram& src);
    static std::shared_ptr<FretDiagram> makeFretDiagram(DummyParentOr<Segment> parent);

    static HarpPedalDiagram* createHarpPedalDiagram(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static HarpPedalDiagram* copyHarpPedalDiagram(const HarpPedalDiagram& src);
    static std::shared_ptr<HarpPedalDiagram> makeHarpPedalDiagram(DummyParentOr<Segment> parent);

    static KeySig* createKeySig(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static KeySig* copyKeySig(const KeySig& src);
    static std::shared_ptr<KeySig> makeKeySig(DummyParentOr<Segment> parent);

    static LaissezVib* createLaissezVib(DummyParentOr<Note> parent, bool isAccessibleEnabled = true);
    static LaissezVib* copyLaissezVib(const LaissezVib& src);

    static LayoutBreak* createLayoutBreak(DummyParentOr<MeasureBase> parent, bool isAccessibleEnabled = true);
    static LayoutBreak* copyLayoutBreak(const LayoutBreak& src);
    static std::shared_ptr<LayoutBreak> makeLayoutBreak(DummyParentOr<MeasureBase> parent);

    static StaffVisibilityIndicator* createStaffVisibilityIndicator(DummyParentOr<System> parent, bool isAccessibleEnabled = true);

    static SystemLockIndicator* createSystemLockIndicator(DummyParentOr<System> parent, const RangeLock* lock,
                                                          bool isAccessibleEnabled = true);
    static SystemLockIndicator* copySystemLockIndicator(const SystemLockIndicator& src);

    static Lyrics* createLyrics(DummyParentOr<ChordRest> parent, bool isAccessibleEnabled = true);
    static Lyrics* copyLyrics(const Lyrics& src);

    static LyricsLine* createLyricsLine(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static LyricsLine* copyLyricsLine(const LyricsLine& src);

    static Measure* createMeasure(Score* parent, bool isAccessibleEnabled = true);
    static Measure* copyMeasure(const Measure& src);

    static MeasureRepeat* createMeasureRepeat(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static MeasureRepeat* copyMeasureRepeat(const MeasureRepeat& src);

    static Note* createNote(DummyParentOr<Chord> parent, bool isAccessibleEnabled = true);
    static Note* copyNote(const Note& src, bool link = false);
    static std::shared_ptr<Note> makeNote(DummyParentOr<Chord> parent);

    static NoteDot* createNoteDot(DummyParentOr<Note> parent, bool isAccessibleEnabled = true);
    static NoteDot* createNoteDot(Rest* parent, bool isAccessibleEnabled = true);
    static NoteDot* copyNoteDot(const NoteDot& src);

    static NoteLine* createNoteLine(DummyParentOr<Note> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<NoteLine> makeNoteLine(DummyParentOr<Note> parent);

    static Page* createPage(Score* parent, bool isAccessibleEnabled = true);

    static PageLockIndicator* createPageLockIndicator(DummyParentOr<System> parent, const RangeLock* lock, bool isAccessibleEnabled = true);
    static PageLockIndicator* copyPageLockIndicator(const PageLockIndicator& src);

    static Parenthesis* createParenthesis(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static Parenthesis* copyParenthesis(const Parenthesis& src);

    static PartialTie* createPartialTie(DummyParentOr<Note> parent, bool isAccessibleEnabled = true);
    static PartialTie* copyPartialTie(const PartialTie& src);

    static PartialLyricsLine* createPartialLyricsLine(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static PartialLyricsLine* copyPartialLyricsLine(const PartialLyricsLine& src);

    static Rest* createRest(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static Rest* createRest(DummyParentOr<Segment> parent, const TDuration& t, bool isAccessibleEnabled = true);
    static Rest* copyRest(const Rest& src, bool link = false);
    static MMRest* createMMRest(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);

    static DeadSlapped* createDeadSlapped(Rest* parent, bool isAccessibleEnabled = true);
    static DeadSlapped* copyDeadSlapped(const DeadSlapped& src);

    static Segment* createSegment(DummyParentOr<Measure> parent, bool isAccessibleEnabled = true);
    static Segment* createSegment(DummyParentOr<Measure> parent, SegmentType type, const Fraction& t, bool isAccessibleEnabled = true);

    static Slur* createSlur(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Slur> makeSlur(DummyParentOr<EngravingItem> parent);

    static Spacer* createSpacer(DummyParentOr<Measure> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Spacer> makeSpacer(DummyParentOr<Measure> parent);

    static Staff* createStaff(Part* parent);

    static StaffLines* createStaffLines(DummyParentOr<Measure> parent, bool isAccessibleEnabled = true);
    static StaffLines* copyStaffLines(const StaffLines& src);

    static StaffState* createStaffState(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static StaffTypeChange* createStaffTypeChange(DummyParentOr<MeasureBase> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<StaffTypeChange> makeStaffTypeChange(DummyParentOr<MeasureBase> parent);

    static StaffText* createStaffText(DummyParentOr<Segment> parent, TextStyleType textStyleType = TextStyleType::STAFF,
                                      bool isAccessibleEnabled = true);
    static StaveSharingLabel* createStaveSharingLabel(DummyParentOr<Segment> parent,
                                                      TextStyleType textStyleType = TextStyleType::STAVE_SHARING,
                                                      bool isAccessibleEnabled = true);

    static SoundFlag* createSoundFlag(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static Expression* createExpression(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);

    static RehearsalMark* createRehearsalMark(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);

    static Stem* createStem(DummyParentOr<Chord> parent, bool isAccessibleEnabled = true);
    static Stem* copyStem(const Stem& src);

    static StemSlash* createStemSlash(DummyParentOr<Chord> parent, bool isAccessibleEnabled = true);
    static StemSlash* copyStemSlash(const StemSlash& src);

    static System* createSystem(Score* parent, bool isAccessibleEnabled = true);
    static SystemText* createSystemText(DummyParentOr<Segment> parent, TextStyleType textStyleType = TextStyleType::SYSTEM,
                                        ElementType type = ElementType::SYSTEM_TEXT, bool isAccessibleEnabled = true);

    static InstrumentChange* createInstrumentChange(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static InstrumentChange* createInstrumentChange(DummyParentOr<Segment> parent, const Instrument& instrument,
                                                    bool isAccessibleEnabled = true);

    static Sticking* createSticking(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);

    static Fingering* createFingering(DummyParentOr<Note> parent, bool isAccessibleEnabled = true);
    static Fingering* createFingering(DummyParentOr<Note> parent, TextStyleType textStyleType, bool isAccessibleEnabled = true);

    static Harmony* createHarmony(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static TempoText* createTempoText(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);

    static Text* createText(DummyParentOr<EngravingItem> parent, TextStyleType tid = TextStyleType::DEFAULT,
                            bool isAccessibleEnabled = true);
    static Text* copyText(const Text& src);

    static Tie* createTie(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static Tie* copyTie(const Tie& src);

    static TimeSig* createTimeSig(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static TimeSig* copyTimeSig(const TimeSig& src);
    static std::shared_ptr<TimeSig> makeTimeSig(DummyParentOr<Segment> parent);

    static TremoloTwoChord* createTremoloTwoChord(DummyParentOr<Chord> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<TremoloTwoChord> makeTremoloTwoChord(DummyParentOr<Chord> parent);
    static TremoloTwoChord* copyTremoloTwoChord(const TremoloTwoChord& src);

    static TremoloSingleChord* createTremoloSingleChord(DummyParentOr<Chord> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<TremoloSingleChord> makeTremoloSingleChord(DummyParentOr<Chord> parent);
    static TremoloSingleChord* copyTremoloSingleChord(const TremoloSingleChord& src);

    static TremoloBar* createTremoloBar(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<TremoloBar> makeTremoloBar(DummyParentOr<EngravingItem> parent);

    static TripletFeel* createTripletFeel(DummyParentOr<Segment> parent, TripletFeelType type, bool isAccessibleEnabled = true);

    static Tuplet* createTuplet(DummyParentOr<Measure> parent, bool isAccessibleEnabled = true);
    static Tuplet* copyTuplet(const Tuplet& src);

    static Hairpin* createHairpin(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Hairpin> makeHairpin(DummyParentOr<EngravingItem> parent);

    static HammerOnPullOff* createHammerOnPullOff(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<HammerOnPullOff> makeHammerOnPullOff(DummyParentOr<EngravingItem> parent);

    static Glissando* createGlissando(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Glissando> makeGlissando(DummyParentOr<EngravingItem> parent);

    static GuitarBend* createGuitarBend(DummyParentOr<Note> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<GuitarBend> makeGuitarBend(DummyParentOr<Note> parent);

    static Jump* createJump(DummyParentOr<Measure> parent, bool isAccessibleEnabled = true);

    static Trill* createTrill(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static Vibrato* createVibrato(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static TextLine* createTextLine(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<TextLine> makeTextLine(DummyParentOr<EngravingItem> parent);

    static Ottava* createOttava(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static LetRing* createLetRing(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static Marker* createMarker(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static Marker* createMarker(DummyParentOr<EngravingItem> parent, TextStyleType tid, bool isAccessibleEnabled = true);

    static std::shared_ptr<Marker> makeMarker(DummyParentOr<EngravingItem> parent);

    static GradualTempoChange* createGradualTempoChange(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static PalmMute* createPalmMute(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static WhammyBar* createWhammyBar(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<WhammyBar> makeWhammyBar(DummyParentOr<EngravingItem> parent);

    static Rasgueado* createRasgueado(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static HarmonicMark* createHarmonicMark(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static PickScrape* createPickScrape(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static Volta* createVolta(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static Pedal* createPedal(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static Dynamic* createDynamic(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);

    static VBox* createVBox(Score* parent, bool isAccessibleEnabled = true);

    static VBox* createVBox(const ElementType& type, Score* parent, bool isAccessibleEnabled = true);

    static VBox* createTitleVBox(Score* parent, bool isAccessibleEnabled = true);

    static HBox* createHBox(Score* parent, bool isAccessibleEnabled = true);

    static TBox* createTBox(Score* parent, bool isAccessibleEnabled = true);

    static FBox* createFBox(Score* parent, bool isAccessibleEnabled = true);

    static Image* createImage(DummyParentOr<EngravingItem> parent);

    static Symbol* createSymbol(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);
    static FSymbol* createFSymbol(DummyParentOr<EngravingItem> parent, bool isAccessibleEnabled = true);

    static PlayCountText* createPlayCountText(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);

    static PlayTechAnnotation* createPlayTechAnnotation(DummyParentOr<Segment> parent, PlayingTechniqueType techniqueType,
                                                        TextStyleType styleType, bool isAccessibleEnabled = true);

    static Capo* createCapo(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<Capo> makeCapo(DummyParentOr<Segment> parent);

    static TimeTickAnchor* createTimeTickAnchor(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);

    static StringTunings* createStringTunings(DummyParentOr<Segment> parent, bool isAccessibleEnabled = true);
    static StringTunings* copyStringTunings(const StringTunings& src);
    static std::shared_ptr<StringTunings> makeStringTunings(DummyParentOr<Segment> parent);

private:
    static EngravingItem* doCreateItem(ElementType type, DummyParentOr<EngravingItem> parent);
};
}
