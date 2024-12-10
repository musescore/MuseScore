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

/**
 \file
 Definition of Score class.
*/

#include <set>
#include <memory>
#include <optional>

#include "global/async/channel.h"
#include "global/types/ret.h"

#include "modularity/ioc.h"
#include "draw/iimageprovider.h"
#include "global/iapplication.h"
#include "../iengravingfontsprovider.h"

#include "../types/constants.h"

#include "../rendering/iscorerenderer.h"
#include "../rendering/layoutoptions.h"
#include "../rendering/paddingtable.h"

#include "../style/style.h"
#include "../style/pagestyle.h"

#include "../compat/midi/compatmidirenderinternal.h"

#include "chordlist.h"
#include "input.h"
#include "mscore.h"
#include "property.h"
#include "scoreorder.h"
#include "select.h"
#include "spannermap.h"
#include "systemlock.h"
#include "synthesizerstate.h"
#include "rootitem.h"
#include "cmd.h"

namespace mu::engraving {
class IMimeData;
}

namespace mu::engraving::read114 {
class Read114;
}

namespace mu::engraving::read302 {
class Read302;
}

namespace mu::engraving::read400 {
class Read400;
}

namespace mu::engraving::read410 {
class Read410;
}

namespace mu::engraving::write {
class Writer;
}

namespace mu::engraving::compat {
class WriteScoreHook;
}

namespace mu::engraving {
class Articulation;
class Audio;
class Box;
class Bracket;
class Chord;
class ChordRest;
class Clef;
class Dynamic;
class Element;
class EventsHolder;
class Excerpt;
class FiguredBass;
class Hairpin;
class Harmony;
class InstrumentTemplate;
class InputState;
class KeyList;
class KeySigEvent;
class LinkedObjects;
class Lyrics;
class MasterScore;
class Measure;
class MeasureBase;
class MuseScoreView;
class Note;
class Page;
class Part;
class RehearsalMark;
class RepeatList;
class Rest;
class Score;
class ScoreElement;
class IEngravingFont;
class Segment;
class Slur;
class Spanner;
class Staff;
class System;
class TDuration;
class TempoMap;
class Text;
class TimeSig;
class TimeSigMap;
class Tuplet;
class UndoCommand;
class UndoStack;

class ShadowNote;

struct Interval;
struct NoteVal;
struct ShowAnchors;

enum class BeatType : char;
enum class Key;
enum class HairpinType : signed char;
enum class SegmentType;
enum class OttavaType : char;
enum class Voicing : signed char;
enum class HDuration : signed char;
enum class AccidentalType;
enum class LayoutBreakType;

enum class LoopBoundaryType : signed char {
    Unknown = -1,
    LoopIn = 0,
    LoopOut = 1
};

enum class Pad : char {
    NOTE00,
    NOTE0,
    NOTE1,
    NOTE2,
    NOTE4,
    NOTE8,
    NOTE16,
    NOTE32,
    NOTE64,
    NOTE128,
    NOTE256,
    NOTE512,
    NOTE1024,
    //--------------------
    REST,
    DOT,
    DOT2,
    DOT3,
    DOT4
};

//---------------------------------------------------------
//   MidiInputEvent
//---------------------------------------------------------

struct MidiInputEvent {
    int pitch = 0;
    bool chord = false;
    int velocity = 0;
};

//---------------------------------------------------------
//   Position
//---------------------------------------------------------

struct Position {
    Segment* segment = nullptr;
    staff_idx_t staffIdx = muse::nidx;
    int line = 0;
    int fret = INVALID_FRET_INDEX;
    PointF pos;
};

//---------------------------------------------------------
//   PlayMode
//---------------------------------------------------------

enum class PlayMode : char {
    SYNTHESIZER,
    AUDIO
};

struct ShowAnchors {
    ShowAnchors() = default;
    ShowAnchors(voice_idx_t vIdx, staff_idx_t stfIdx, const Fraction& sTickMain, const Fraction& eTickMain,
                const Fraction& sTickExt, const Fraction& eTickExt)
        : voiceIdx(vIdx), staffIdx(stfIdx), startTickMainRegion(sTickMain), endTickMainRegion(eTickMain),
        startTickExtendedRegion(sTickExt), endTickExtendedRegion(eTickExt) {}

    void reset()
    {
        voiceIdx = muse::nidx;
        staffIdx = muse::nidx;
        startTickMainRegion = Fraction(-1, 1);
        endTickMainRegion = Fraction(-1, 1);
        startTickExtendedRegion = Fraction(-1, 1);
        endTickExtendedRegion = Fraction(-1, 1);
    }

    voice_idx_t voiceIdx = muse::nidx;
    staff_idx_t staffIdx = muse::nidx;
    Fraction startTickMainRegion = Fraction(-1, 1);
    Fraction endTickMainRegion = Fraction(-1, 1);
    Fraction startTickExtendedRegion = Fraction(-1, 1);
    Fraction endTickExtendedRegion = Fraction(-1, 1);
};

//---------------------------------------------------------------------------------------
//   @@ Score
//   @P composer        string            composer of the score (read only)
//   @P duration        int               duration of score in seconds (read only)
//   @P excerpts        array[Excerpt]    the list of the excerpts (linked parts)
//   @P firstMeasure    Measure           the first measure of the score (read only)
//   @P firstMeasureMM  Measure           the first multi-measure rest measure of the score (read only)
//   @P harmonyCount    int               number of harmony items (read only)
//   @P hasHarmonies    bool              score has chord symbols (read only)
//   @P hasLyrics       bool              score has lyrics (read only)
//   @P keysig          int               key signature at the start of the score (read only)
//   @P lastMeasure     Measure           the last measure of the score (read only)
//   @P lastMeasureMM   Measure           the last multi-measure rest measure of the score (read only)
//   @P lastSegment     Segment           the last score segment (read-only)
//   @P lyricCount      int               number of lyric items (read only)
//   @P name            string            name of the score
//   @P nmeasures       int               number of measures (read only)
//   @P npages          int               number of pages (read only)
//   @P nstaves         int               number of staves (read only)
//   @P ntracks         int               number of tracks (staves * 4) (read only)
// not to be documented?
//   @P parts           array[Part]       the list of parts (read only)
//
//    a Score has always an associated MasterScore
//---------------------------------------------------------------------------------------

class Score : public EngravingObject, public muse::Injectable
{
    OBJECT_ALLOCATOR(engraving, Score)
    DECLARE_CLASSOF(ElementType::SCORE)

    muse::Inject<muse::draw::IImageProvider> imageProvider = { this };
    muse::Inject<IEngravingConfiguration> configuration = { this };
    muse::Inject<IEngravingFontsProvider> engravingFonts = { this };
    muse::Inject<muse::IApplication> application = { this };
    muse::Inject<IEngravingElementsProvider> elementsProvider = { this };

    // internal
    muse::Inject<rendering::IScoreRenderer> renderer = { this };

public:
    Score(const Score&) = delete;
    Score& operator=(const Score&) = delete;
    virtual ~Score();
    Score* clone();

    static Score* paletteScore();
    bool isPaletteScore() const;

    virtual bool isMaster() const { return false; }
    virtual bool readOnly() const;

    static void onElementDestruction(EngravingItem* se);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void dumpScoreTree();  // for debugging purposes

    RootItem* rootItem() const { return m_rootItem; }
    compat::DummyElement* dummy() const { return m_rootItem->dummy(); }

    ShadowNote* shadowNote() const;

    muse::async::Channel<LoopBoundaryType, unsigned> loopBoundaryTickChanged() const;
    void notifyLoopBoundaryTickChanged(LoopBoundaryType type, unsigned ticks);

    muse::async::Channel<EngravingItem*> elementDestroyed();

    muse::async::Channel<float> layoutProgressChannel() const;

    void rebuildBspTree();
    bool noStaves() const { return m_staves.empty(); }
    void insertPart(Part*, size_t targetPartIdx);
    void appendPart(Part*);
    void removePart(Part*);
    void insertStaff(Staff*, staff_idx_t);
    void appendStaff(Staff*);
    void cmdRemoveStaff(staff_idx_t staffIdx);
    void removeStaff(Staff*);
    void addMeasure(MeasureBase*, MeasureBase*);
    void linkMeasures(Score* score);
    void setResetAutoplace() { m_resetAutoplace = true; }
    void setResetCrossBeams() { m_resetCrossBeams = true; }

    Excerpt* excerpt() { return m_excerpt; }
    void setExcerpt(Excerpt* e) { m_excerpt = e; }

    // methods for resetting elements for pre-4.0 score migration
    void resetAutoplace();
    void resetCrossBeams();

    void cmdAddBracket();
    void cmdAddParentheses();
    void cmdAddParentheses(EngravingItem* el);
    void cmdAddBraces();
    void cmdAddFret(int fret);
    void cmdSetBeamMode(BeamMode);
    void cmdRemovePart(Part*);
    void cmdAddTie(bool addToChord = false);
    Tie* cmdToggleTie();
    void cmdToggleLaissezVib();
    static std::vector<Note*> cmdTieNoteList(const Selection& selection, bool noteEntryMode);
    void cmdAddOttava(OttavaType);
    std::vector<Hairpin*> addHairpins(HairpinType);
    void addNoteLine();
    void padToggle(Pad p, bool toggleForSelectionOnly = false);

    bool resolveNoteInputParams(int noteIdx, bool addFlag, NoteInputParams& out) const;

    void cmdAddPitch(const EditData&, const NoteInputParams& params, bool addFlag, bool insert);
    void cmdAddPitch(int note, bool addFlag, bool insert);

    void cmdAddStretch(double);
    void cmdAddGrace(NoteType, int);
    void cmdResetNoteAndRestGroupings();
    void cmdResetAllPositions(bool undoable = true);
    void cmdDoubleDuration() { cmdIncDecDuration(-1, false); }
    void cmdHalfDuration() { cmdIncDecDuration(1, false); }
    void cmdIncDurationDotted() { cmdIncDecDuration(-1, true); }
    void cmdDecDurationDotted() { cmdIncDecDuration(1, true); }
    void cmdIncDecDuration(int nSteps, bool stepDotted = false);
    void cmdToggleLayoutBreak(LayoutBreakType);
    void cmdMoveMeasureToPrevSystem();
    void cmdMoveMeasureToNextSystem();
    void cmdToggleSystemLock();
    void cmdApplyLockToSelection();
    void cmdToggleScoreLock();
    void cmdMakeIntoSystem();
    void cmdAddStaffTypeChange(Measure* measure, staff_idx_t staffIdx, StaffTypeChange* stc);
    void cmdAddMeasureRepeat(Measure*, int numMeasures, staff_idx_t staffIdx);
    bool makeMeasureRepeatGroup(Measure*, int numMeasures, staff_idx_t staffIdx);
    void cmdFlip();
    void resetUserStretch();
    void cmdResetToDefaultLayout();
    void cmdResetBeamMode();
    void cmdResetTextStyleOverrides();
    void cmdResetAllStyles(const StyleIdSet& exceptTheseOnes = {});
    void cmdResetMeasuresLayout();
    bool canInsertClef(ClefType) const;
    void cmdInsertClef(ClefType);
    void removeChordRest(ChordRest* cr, bool clearSegment);
    ChordRest* nextMeasure(ChordRest* element, bool selectBehavior = false, bool mmRest = false);
    ChordRest* prevMeasure(ChordRest* element, bool mmRest = false);
    ChordRest* upStaff(ChordRest* cr);
    ChordRest* downStaff(ChordRest* cr);
    void cmdPadNoteIncreaseTAB();
    void cmdPadNoteDecreaseTAB();
    void cmdToggleMmrest();
    void cmdToggleHideEmpty();
    void cmdSetVisible();
    void cmdUnsetVisible();
    void cmdMoveRest(Rest*, DirectionV);
    void cmdMoveLyrics(Lyrics*, DirectionV);

    void realtimeAdvance(bool allowTransposition);

    void addRemoveSystemLocks(int interval, bool lock);

    bool transpose(Note* n, Interval, bool useSharpsFlats);
    void transposeKeys(staff_idx_t staffStart, staff_idx_t staffEnd, const Fraction& tickStart, const Fraction& tickEnd, bool flip = false);
    bool transpose(TransposeMode mode, TransposeDirection, Key transposeKey, int transposeInterval, bool trKeys, bool transposeChordNames,
                   bool useDoubleSharpsFlats);

    bool appendMeasuresFromScore(Score* score, const Fraction& startTick, const Fraction& endTick);
    bool appendScore(Score*, bool addPageBreak = false, bool addSectionBreak = true);

    const std::vector<Staff*>& staves() const { return m_staves; }
    size_t nstaves() const { return m_staves.size(); }
    size_t ntracks() const { return m_staves.size() * VOICES; }

    staff_idx_t staffIdx(const Staff*) const;
    staff_idx_t staffIdx(const Part*) const;
    Staff* staff(size_t n) const { return (n < m_staves.size()) ? m_staves.at(n) : nullptr; }
    Staff* staffById(const muse::ID& staffId) const;
    Part* partById(const muse::ID& partId) const;

    void clearSystemObjectStaves();
    void addSystemObjectStaff(Staff* staff);
    void removeSystemObjectStaff(Staff* staff);
    const std::vector<Staff*>& systemObjectStaves() const { return m_systemObjectStaves; }
    bool isSystemObjectStaff(Staff* staff) const;

    Measure* pos2measure(const PointF&, staff_idx_t* staffIdx, int* pitch, Segment**, PointF* offset) const;
    void dragPosition(const PointF&, staff_idx_t* staffIdx, Segment**, double spacingFactor = 0.5, bool allowTimeAnchor = false) const;

    void undoAddElement(EngravingItem* element, bool addToLinkedStaves = true, bool ctrlModifier = false,
                        EngravingItem* elementToRelink = nullptr);
    void undoAddCR(ChordRest* element, Measure*, const Fraction& tick);
    void undoRemoveElement(EngravingItem* element, bool removeLinked = true);
    void undoChangeSpannerElements(Spanner* spanner, EngravingItem* startElement, EngravingItem* endElement);
    void undoChangeElement(EngravingItem* oldElement, EngravingItem* newElement);
    void undoChangePitch(Note* note, int pitch, int tpc1, int tpc2);
    void undoChangeFretting(Note* note, int pitch, int string, int fret, int tpc1, int tpc2);
    void spellNotelist(std::vector<Note*>& notes);
    void undoChangeTpc(Note* note, int tpc);
    void undoChangeChordRestLen(ChordRest* cr, const TDuration&);
    void undoTransposeHarmony(Harmony*, int, int);
    void undoExchangeVoice(Measure* measure, voice_idx_t val1, voice_idx_t val2, staff_idx_t staff1, staff_idx_t staff2);
    void undoRemovePart(Part* part, size_t partIdx = muse::nidx);
    void undoInsertPart(Part* part, size_t targetPartIndex);
    void undoRemoveStaff(Staff* staff);
    void undoInsertStaff(Staff* staff, staff_idx_t idx, bool createRests=true);
    void undoChangeVisible(EngravingItem* item, bool visible);
    void undoChangeTuning(Note*, double);
    void undoChangeUserMirror(Note*, DirectionH);
    void undoChangeKeySig(Staff* ostaff, const Fraction& tick, KeySigEvent);
    void undoChangeClef(Staff* ostaff, EngravingItem*, ClefType st, bool forInstrumentChange = false, Clef* clefToRelink = nullptr);
    bool undoPropertyChanged(EngravingItem* item, Pid propId, const PropertyValue& propValue,
                             PropertyFlags propFlags = PropertyFlags::NOSTYLE);
    void undoPropertyChanged(EngravingObject*, Pid, const PropertyValue& v, PropertyFlags ps = PropertyFlags::NOSTYLE);
    virtual UndoStack* undoStack() const;
    void undo(UndoCommand*, EditData* = nullptr) const;
    void undoRemoveMeasures(Measure*, Measure*, bool preserveTies = false, bool moveStaffTypeChanges = true);
    void undoChangeMeasureRepeatCount(Measure* m, int count, staff_idx_t staffIdx);
    void undoAddBracket(Staff* staff, size_t level, BracketType type, size_t span);
    void undoRemoveBracket(Bracket*);
    void undoInsertTime(const Fraction& tick, const Fraction& len);
    void undoChangeStyleVal(Sid idx, const PropertyValue& v);
    void undoChangeStyleValues(std::unordered_map<Sid, PropertyValue> values);
    void undoChangePageNumberOffset(int po);
    void undoChangeParent(EngravingItem* element, EngravingItem* parent, staff_idx_t _staff);

    void updateInstrumentChangeTranspositions(KeySigEvent& key, Staff* staff, const Fraction& tick);

    Note* setGraceNote(Chord*,  int pitch, NoteType type, int len);

    GuitarBend* addGuitarBend(GuitarBendType type, Note* note, Note* endNote = nullptr);

    Segment* setNoteRest(Segment*, track_idx_t track, NoteVal nval, Fraction, DirectionV stemDirection = DirectionV::AUTO,
                         bool forceAccidental = false, const std::set<SymId>& articulationIds = {}, bool rhythmic = false,
                         InputState* externalInputState = nullptr);
    void changeCRlen(ChordRest* cr, const TDuration&);
    void changeCRlen(ChordRest* cr, const Fraction&, bool fillWithRest=true);
    void createCRSequence(const Fraction& f, ChordRest* cr, const Fraction& tick);

    Fraction makeGap(Segment*, track_idx_t track, const Fraction&, Tuplet*, bool keepChord = false);
    bool makeGap1(const Fraction& baseTick, staff_idx_t staffIdx, const Fraction& len, int voiceOffset[VOICES]);
    bool makeGapVoice(Segment* seg, track_idx_t track, Fraction len, const Fraction& tick);

    Rest* addRest(const Fraction& tick, track_idx_t track, TDuration, Tuplet*);
    Rest* addRest(Segment* seg, track_idx_t track, TDuration d, Tuplet*);
    Chord* addChord(const Fraction& tick, TDuration d, Chord* oc, bool genTie, Tuplet* tuplet);
    MeasureRepeat* addMeasureRepeat(const Fraction& tick, track_idx_t track, int numMeasures);

    Tuplet* addTuplet(ChordRest* destinationChordRest, Fraction ratio, TupletNumberType numberType, TupletBracketType bracketType);

    ChordRest* addClone(ChordRest* cr, const Fraction& tick, const TDuration& d);
    Rest* setRest(const Fraction& tick, track_idx_t track, const Fraction&, bool useDots, Tuplet* tuplet, bool useFullMeasureRest = true);
    std::vector<Rest*> setRests(const Fraction& tick, track_idx_t track, const Fraction&, bool useDots, Tuplet* tuplet,
                                bool useFullMeasureRest = true);

    void upDown(bool up, UpDownMode);
    void upDownDelta(int pitchDelta);
    ChordRest* searchNote(const Fraction& tick, track_idx_t track) const;

    // undo/redo ops
    void toggleArticulation(SymId);
    bool toggleArticulation(EngravingItem*, Articulation* atr);
    void toggleOrnament(SymId);
    void toggleAccidental(AccidentalType);
    void changeAccidental(AccidentalType);
    void changeAccidental(Note* oNote, AccidentalType);

    void addElement(EngravingItem*);
    void doUndoAddElement(EngravingItem*);
    void removeElement(EngravingItem*);
    void doUndoRemoveElement(EngravingItem*);
    bool containsElement(const EngravingItem*) const;

    enum class AddToChord {
        None,
        AtPreviousPosition,
        AtCurrentPosition,
    };

    Note* addPitch(NoteVal&, bool addToPreviousChord, InputState* externalInputState = nullptr);
    Note* addPitch(NoteVal&, AddToChord addFlag = AddToChord::None, InputState* externalInputState = nullptr);

    Note* addTiedMidiPitch(int pitch, bool addFlag, Chord* prevChord, bool allowTransposition);
    NoteVal noteVal(int pitch, bool allowTransposition) const;
    Note* addMidiPitch(int pitch, bool addFlag, bool allowTransposition);
    Note* addNote(Chord*, const NoteVal& noteVal, bool forceAccidental = false, const std::set<SymId>& articulationIds = {},
                  InputState* externalInputState = nullptr);
    Note* addNoteToTiedChord(Chord*, const NoteVal& noteVal, bool forceAccidental = false, const std::set<SymId>& articulationIds = {});

    NoteVal noteValForPosition(Position pos, AccidentalType at, bool& error);

    Slur* addSlur(ChordRest* firstChordRest, ChordRest* secondChordRest, const Slur* slurTemplate);
    TextBase* addText(TextStyleType type, EngravingItem* destinationElement = nullptr);

    void deleteItem(EngravingItem*);
    void deleteMeasures(MeasureBase* firstMeasure, MeasureBase* lastMeasure, bool preserveTies = false);
    void restoreInitialKeySigAndTimeSig();
    void reconnectSlurs(MeasureBase* mbStart, MeasureBase* mbLast);
    void cmdDeleteSelection();
    std::vector<ChordRest*> deleteRange(Segment* segStart, Segment* segEnd, track_idx_t trackStart, track_idx_t trackEnd,
                                        const SelectionFilter& filter);
    void cmdFullMeasureRest();

    muse::Ret putNote(const PointF&, bool replace, bool insert);
    muse::Ret insertChordByInsertingTime(const Position&);

    void cloneVoice(track_idx_t strack, track_idx_t dtrack, Segment* sf, const Fraction& lTick, bool link = true, bool spanner = true);

    muse::Ret repitchNote(const Position& pos, bool replace);
    void regroupNotesAndRests(const Fraction& startTick, const Fraction& endTick, track_idx_t track);

    void startCmd(const TranslatableString& actionName);             // start undoable command
    void endCmd(bool rollback = false, bool layoutAllParts = false); // end undoable command
    void update() { update(true); }
    void lockUpdates(bool locked);
    void undoRedo(bool undo, EditData*);

    virtual muse::async::Channel<ScoreChangesRange> changesChannel() const;

    void cmdRemoveTimeSig(TimeSig*);
    void cmdAddTimeSig(Measure*, staff_idx_t staffIdx, TimeSig*, bool local);

    virtual void setUpdateAll();
    void setLayoutAll(staff_idx_t staff = muse::nidx, const EngravingItem* e = nullptr);
    void setLayout(const Fraction& tick, staff_idx_t staff, const EngravingItem* e = nullptr);
    void setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e = nullptr);
    virtual CmdState& cmdState();
    virtual const CmdState& cmdState() const;
    virtual void addLayoutFlags(LayoutFlags);
    virtual void setInstrumentsChanged(bool);
    void addRefresh(const RectF&);

    void cmdToggleAutoplace(bool all);

    bool playNote() const { return m_updateState.playNote; }
    void setPlayNote(bool v) { m_updateState.playNote = v; }
    bool playChord() const { return m_updateState.playChord; }
    void setPlayChord(bool v) { m_updateState.playChord = v; }
    bool selectionEmpty() const { return m_selection.staffStart() == m_selection.staffEnd(); }
    bool selectionChanged() const { return m_updateState.selectionChanged; }
    void setSelectionChanged(bool val) { m_updateState.selectionChanged = val; }
    void deleteLater(EngravingObject* e) { m_updateState.deleteList.push_back(e); }
    void deletePostponed();

    void changeSelectedElementsVoice(voice_idx_t);
    void changeSelectedElementsVoiceAssignment(VoiceAssignment);

    const std::vector<Part*>& parts() const;
    int visiblePartCount() const;

    using StaffAccepted = std::function<bool (const Staff&)>;
    std::set<staff_idx_t> staffIdxSetFromRange(const track_idx_t trackFrom, const track_idx_t trackTo,
                                               StaffAccepted staffAccepted = StaffAccepted()) const;

    void appendPart(const InstrumentTemplate*);
    void updateStaffIndex();
    void sortSystemObjects(std::vector<staff_idx_t>& dst);
    void sortStaves(std::vector<staff_idx_t>& dst);

    bool isShowInvisible() const { return m_showInvisible; }
    bool showUnprintable() const { return m_showUnprintable; }
    bool showFrames() const { return m_showFrames; }
    bool showPageborders() const { return m_showPageborders; }
    bool showSoundFlags() const { return m_showSoundFlags; }
    bool markIrregularMeasures() const { return m_markIrregularMeasures; }
    bool showInstrumentNames() const { return m_showInstrumentNames; }
    void setShowInvisible(bool v);
    void setShowUnprintable(bool v);
    void setShowFrames(bool v);
    void setShowPageborders(bool v);
    void setShowSoundFlags(bool v);
    void setMarkIrregularMeasures(bool v);
    void setShowInstrumentNames(bool v) { m_showInstrumentNames = v; }

    void hideAnchors() { m_showAnchors.reset(); }
    void setShowAnchors(const ShowAnchors& showAnchors);
    const ShowAnchors& showAnchors() const { return m_showAnchors; }

    void print(muse::draw::Painter* printer, int page);
    ChordRest* getSelectedChordRest() const;
    std::set<ChordRest*> getSelectedChordRests() const;
    void getSelectedStartEndChordRests(ChordRest*& cr1, ChordRest*& cr2) const;

    void select(EngravingItem* item, SelectType = SelectType::SINGLE, staff_idx_t staff = 0);
    void select(const std::vector<EngravingItem*>& items, SelectType = SelectType::SINGLE, staff_idx_t staff = 0);
    void selectSimilar(EngravingItem* e, bool sameStaff);
    void selectSimilarInRange(EngravingItem* e);
    static void collectMatch(void* data, EngravingItem* e);
    static void collectNoteMatch(void* data, EngravingItem* e);
    void deselect(EngravingItem* obj);
    void deselectAll() { m_selection.deselectAll(); }
    void updateSelection() { m_selection.update(); }
    EngravingItem* getSelectedElement() const { return m_selection.element(); }
    const Selection& selection() const { return m_selection; }
    Selection& selection() { return m_selection; }
    SelectionFilter& selectionFilter() { return m_selectionFilter; }
    void setSelection(const Selection& s);

    Fraction pos();
    Measure* tick2measure(const Fraction& tick) const;
    Measure* tick2measureMM(const Fraction& tick) const;
    MeasureBase* tick2measureBase(const Fraction& tick) const;
    Segment* tick2segment(const Fraction& tick, bool first, SegmentType st, bool useMMrest = false) const;
    Segment* tick2segment(const Fraction& tick) const;
    Segment* tick2segment(const Fraction& tick, bool first) const;
    Segment* tick2segmentMM(const Fraction& tick, bool first, SegmentType st) const;
    Segment* tick2segmentMM(const Fraction& tick) const;
    Segment* tick2segmentMM(const Fraction& tick, bool first) const;
    Segment* tick2leftSegment(const Fraction& tick, bool useMMrest = false, SegmentType segType = SegmentType::ChordRest) const;
    Segment* tick2rightSegment(const Fraction& tick, bool useMMrest = false, SegmentType segType = SegmentType::ChordRest) const;
    Segment* tick2leftSegmentMM(const Fraction& tick) { return tick2leftSegment(tick, /* useMMRest */ true); }

    void setUpTempoMapLater();
    void setUpTempoMap();

    EngravingItem* nextElement();
    EngravingItem* prevElement();
    ChordRest* cmdNextPrevSystem(ChordRest*, bool);
    Box* cmdNextPrevFrame(MeasureBase*, bool) const;
    EngravingItem* cmdNextPrevSection(EngravingItem*, bool) const;
    MeasureBase* getNextPrevSectionBreak(MeasureBase*, bool) const;
    EngravingItem* getScoreElementOfMeasureBase(MeasureBase*) const;

    int fileDivision(int t) const { return static_cast<int>(((int64_t)t * Constants::DIVISION + m_fileDivision / 2) / m_fileDivision); }
    void setFileDivision(int t) { m_fileDivision = t; }

    bool dirty() const;
    bool savedCapture() const { return m_savedCapture; }
    void setSavedCapture(bool v) { m_savedCapture = v; }
    bool printing() const { return m_printing; }
    void setPrinting(bool val) { m_printing = val; }
    virtual bool playlistDirty() const;
    virtual void setPlaylistDirty();

    bool isOpen() const;
    void setIsOpen(bool open);

    void spell();
    void spell(staff_idx_t startStaff, staff_idx_t endStaff, Segment* startSegment, Segment* endSegment);
    void spell(Note*);
    void changeEnharmonicSpelling(bool both);

    Fraction nextSeg(const Fraction& tick, int track);

    ChordList* chordList() { return &m_chordList; }
    const ChordList* chordList() const { return &m_chordList; }
    void checkChordList();

    MStyle& style() { return m_style; }
    const MStyle& style() const { return m_style; }

    PageSizeGetAccessor pageSize() const { return PageSizeGetAccessor(m_style); }
    PageSizeSetAccessor pageSize() { return PageSizeSetAccessor(m_style); }

    void resetStyleValue(Sid styleToReset);
    void resetStyleValues(const StyleIdSet& styleIdSet);

    void setStyle(const MStyle& s, const bool overlap = false);
    bool loadStyle(const String&, bool ign = false, const bool overlap = false);
    bool saveStyle(const String&);

    TranslatableString getTextStyleUserName(TextStyleType tid);

    // These position are in ticks and not uticks
    Fraction loopInTick() const { return loopBoundaryTick(LoopBoundaryType::LoopIn); }
    Fraction loopOutTick() const { return loopBoundaryTick(LoopBoundaryType::LoopOut); }
    void setLoopInTick(const Fraction& tick) { setLoopBoundaryTick(LoopBoundaryType::LoopIn, tick); }
    void setLoopOutTick(const Fraction& tick) { setLoopBoundaryTick(LoopBoundaryType::LoopOut, tick); }

    Fraction loopBoundaryTick(LoopBoundaryType type) const;
    void setLoopBoundaryTick(LoopBoundaryType type, Fraction tick);

    bool noteEntryMode() const { return inputState().noteEntryMode(); }
    bool usingNoteEntryMethod(NoteEntryMethod m) { return inputState().usingNoteEntryMethod(m); }
    const InputState& inputState() const { return m_is; }
    InputState& inputState() { return m_is; }
    void setInputState(const InputState& st) { m_is = st; }

    void spatiumChanged(double oldValue, double newValue);
    void styleChanged() override;

    std::vector<EngravingItem*> cmdPaste(const IMimeData* ms, MuseScoreView* view, Fraction scale = Fraction(1, 1));
    bool pasteStaff(XmlReader&, Segment* dst, staff_idx_t staffIdx, Fraction scale = Fraction(1, 1));
    void pasteSymbols(XmlReader& e, ChordRest* dst);

    static void transposeChord(Chord* c, const Fraction& tick);

    BeatType tick2beatType(const Fraction& tick) const;

    int mscVersion() const { return m_mscVersion; }
    void setMscVersion(int v) { m_mscVersion = v; }

    void addLyrics(const Fraction& tick, staff_idx_t staffIdx, const String&);

    void updateSwing();

    void updateCapo();
    void updateChannel();

    void cmdConcertPitchChanged(bool);

    virtual TempoMap* tempomap() const;
    virtual TimeSigMap* sigmap() const;

    void setTempo(Segment*, BeatsPerSecond bps);
    void setTempo(const Fraction& tick, BeatsPerSecond bps);
    void removeTempo(const Fraction& tick);
    void setPause(const Fraction& tick, double seconds);
    BeatsPerSecond tempo(const Fraction& tick) const;

    Text* getText(TextStyleType subtype) const;

    ScoreOrder scoreOrder() const;
    void setScoreOrder(ScoreOrder order);
    void updateBracesAndBarlines(Part* part, size_t index);
    void setBracketsAndBarlines();
    void remapBracketsAndBarlines();

    void lassoSelect(const RectF&);
    void lassoSelectEnd();

    Page* searchPage(const PointF&) const;
    std::vector<System*> searchSystem(const PointF& p, const System* preferredSystem = nullptr, double spacingFactor = 0.5,
                                      double preferredSpacingFactor = 1.0) const;
    Measure* searchMeasure(const PointF& p, const System* preferredSystem = nullptr, double spacingFactor = 0.5,
                           double preferredSpacingFactor = 1.0) const;

    bool getPosition(Position* pos, const PointF&, voice_idx_t voice) const;

    void cmdDeleteTuplet(Tuplet*, bool replaceWithRest);
    Measure* getCreateMeasure(const Fraction& tick);

    void adjustBracketsDel(size_t sidx, size_t eidx);
    void adjustBracketsIns(size_t sidx, size_t eidx);
    void adjustKeySigs(track_idx_t sidx, track_idx_t eidx, KeyList km);
    KeyList keyList() const;

    /// To be used together with setExpandRepeats.
    /// For bigger operations, where suboperations might also use it,
    /// where those need to have the same value for expandRepeats.
    virtual const RepeatList& repeatList() const;
    /// For small, one-step operations, where you need to get the relevant repeatList just once
    virtual const RepeatList& repeatList(bool expandRepeats, bool updateTies = true) const;

    double utick2utime(int tick) const;
    int utime2utick(double utime) const;

    void nextInputPos(ChordRest* cr, bool);
    void cmdMirrorNoteHead();

    virtual size_t npages() const { return m_pages.size(); }
    virtual page_idx_t pageIdx(const Page* page) const { return muse::indexOf(m_pages, page); }
    virtual const std::vector<Page*>& pages() const { return m_pages; }
    virtual std::vector<Page*>& pages() { return m_pages; }

    const std::vector<System*>& systems() const { return m_systems; }
    std::vector<System*>& systems() { return m_systems; }

    MeasureBaseList* measures() { return &m_measures; }
    bool checkHasMeasures() const;
    MeasureBase* first() const;
    MeasureBase* firstMM() const;
    MeasureBase* last()  const;
    Measure* firstMeasure() const;
    Measure* firstMeasureMM() const;
    Measure* lastMeasure() const;
    Measure* lastMeasureMM() const;
    MeasureBase* measure(int idx) const;
    Measure* crMeasure(int idx) const;

    Fraction endTick() const;

    Segment* firstSegment(SegmentType s) const;
    Segment* firstSegmentMM(SegmentType s) const;
    Segment* lastSegment() const;
    Segment* lastSegmentMM() const;

    void connectTies(bool silent = false);
    void undoRemoveStaleTieJumpPoints();

    void scanElementsInRange(void* data, void (* func)(void*, EngravingItem*), bool all = true);
    int fileDivision() const { return m_fileDivision; }   ///< division of current loading *.msc file
    void splitStaff(staff_idx_t staffIdx, int splitPoint);
    FiguredBass* addFiguredBass();
    void expandVoice(Segment* s, track_idx_t track);
    void expandVoice();

    EngravingItem* selectMove(const String& cmd);
    EngravingItem* move(const String& cmd);
    void cmdEnterRest(const TDuration& d);
    void enterRest(const TDuration& d, InputState* externalInputState = nullptr);
    void addInterval(int, const std::vector<Note*>&);
    void cmdCreateTuplet(ChordRest*, Tuplet*);
    void removeAudio();

    bool autoLayoutEnabled() const;

    void doLayout();
    void doLayoutRange(const Fraction& st, const Fraction& et);

    SynthesizerState& synthesizerState() { return m_synthesizerState; }
    void setSynthesizerState(const SynthesizerState& s);

    MasterScore* masterScore() const { return m_masterScore; }
    void setMasterScore(MasterScore* s) { m_masterScore = s; }

    const std::map<String, String>& metaTags() const { return m_metaTags; }
    std::map<String, String>& metaTags() { return m_metaTags; }
    void setMetaTags(const std::map<String, String>& t) { m_metaTags = t; }

    //@ returns as a string the metatag named 'tag'
    String metaTag(const String& tag) const;
    //@ sets the metatag named 'tag' to 'val'
    void setMetaTag(const String& tag, const String& val);

    void cmdSplitMeasure(ChordRest*);
    void cmdJoinMeasure(Measure*, Measure*);

    int pageNumberOffset() const { return m_pageNumberOffset; }
    void setPageNumberOffset(int v) { m_pageNumberOffset = v; }

    String appVersion() const { return application()->version().toString(); }
    String mscoreVersion() const { return m_mscoreVersion; }
    int mscoreRevision() const { return m_mscoreRevision; }
    void setMscoreVersion(const String& val) { m_mscoreVersion = val; }
    void setMscoreRevision(int val) { m_mscoreRevision = val; }

    void addViewer(MuseScoreView* v) { m_viewer.push_back(v); }
    void removeViewer(MuseScoreView* v) { m_viewer.remove(v); }
    const std::list<MuseScoreView*>& getViewer() const { return m_viewer; }

    //! NOTE Layout
    const LayoutOptions& layoutOptions() const { return m_layoutOptions; }
    void setLayoutMode(LayoutMode lm) { m_layoutOptions.mode = lm; }
    void setShowVBox(bool v) { m_layoutOptions.isShowVBox = v; }
    double noteHeadWidth() const { return m_layoutOptions.noteHeadWidth; }
    void setNoteHeadWidth(double n) { m_layoutOptions.noteHeadWidth = n; }

    // temporary methods
    bool isLayoutMode(LayoutMode lm) const { return m_layoutOptions.isMode(lm); }
    LayoutMode layoutMode() const { return m_layoutOptions.mode; }
    bool lineMode() const { return m_layoutOptions.isMode(LayoutMode::LINE); }
    bool linearMode() const { return m_layoutOptions.isLinearMode(); }
    // ----

    void cmdSelectAll();
    void cmdSelectSection();
    void transposeSemitone(int semitone);
    void transposeDiatonicAlterations(TransposeDirection direction);

    struct InsertMeasureOptions {
        InsertMeasureOptions() {}

        bool createEmptyMeasures = false;
        bool moveSignaturesClef = true;
        bool needDeselectAll = true;
        bool cloneBoxToAllParts = true;
        bool moveStaffTypeChanges = true;
        bool ignoreBarLines = false;
    };

    MeasureBase* insertMeasure(ElementType type, MeasureBase* beforeMeasure = nullptr,
                               const InsertMeasureOptions& options = InsertMeasureOptions());
    MeasureBase* insertBox(ElementType type, MeasureBase* beforeMeasure = nullptr,
                           const InsertMeasureOptions& options = InsertMeasureOptions());

    Audio* audio() const { return m_audio; }
    void setAudio(Audio* a) { m_audio = a; }
    PlayMode playMode() const { return m_playMode; }
    void setPlayMode(PlayMode v) { m_playMode = v; }

    int linkId();
    void linkId(int);
    int getLinkId() const { return m_linkId; }

    std::list<Score*> scoreList();

    //@ appends to the score a number of measures
    void appendMeasures(int);

    const std::multimap<int, Spanner*>& spanner() const { return m_spanner.map(); }
    SpannerMap& spannerMap() { return m_spanner; }
    const SpannerMap& spannerMap() const { return m_spanner; }
    bool isSpannerStartEnd(const Fraction& tick, track_idx_t track) const;
    void removeSpanner(Spanner*);
    void addSpanner(Spanner*, bool computeStartEnd = true);
    void cmdAddSpanner(Spanner* spanner, const PointF& pos, bool systemStavesOnly = false);
    void cmdAddSpanner(Spanner* spanner, staff_idx_t staffIdx, Segment* startSegment, Segment* endSegment, bool ctrlModifier = false);
    void checkSpanner(const Fraction& startTick, const Fraction& lastTick, bool removeOrphans = true);
    const std::set<Spanner*>& unmanagedSpanners() const { return m_unmanagedSpanner; }
    void addUnmanagedSpanner(Spanner*);
    void removeUnmanagedSpanner(Spanner*);

    Hairpin* addHairpin(HairpinType type, ChordRest* cr1, ChordRest* cr2 = nullptr);
    void addHairpin(Hairpin* hairpin, ChordRest* cr1, ChordRest* cr2 = nullptr);
    void addHairpinToDynamic(Hairpin* hairpin, Dynamic* dynamic);
    Hairpin* addHairpinToDynamicOnGripDrag(Dynamic* dynamic, bool isLeftGrip, const PointF& pos);

    ChordRest* findCR(Fraction tick, track_idx_t track) const;
    ChordRest* findChordRestEndingBeforeTickInStaff(const Fraction& tick, staff_idx_t staffIdx) const;
    ChordRest* findChordRestEndingBeforeTickInTrack(const Fraction& tick, track_idx_t trackIdx) const;
    void insertTime(const Fraction& tickPos, const Fraction& tickLen);

    std::shared_ptr<IEngravingFont> engravingFont() const { return m_engravingFont; }
    void setEngravingFont(std::shared_ptr<IEngravingFont> f) { m_engravingFont = f; }

    std::list<staff_idx_t> uniqueStaves() const;

    void transpositionChanged(Part* part, Interval oldTransposition, Fraction tickStart = { 0, 1 }, Fraction tickEnd = { -1, 1 });
    void transpositionChanged(Part* part, const Fraction& instrumentTick, Interval oldTransposition);

    void moveUp(ChordRest*);
    void moveDown(ChordRest*);
    EngravingItem* upAlt(EngravingItem*);
    Note* upAltCtrl(Note*) const;
    EngravingItem* downAlt(EngravingItem*);
    Note* downAltCtrl(Note*) const;

    EngravingItem* firstElement(bool frame = true);
    EngravingItem* lastElement(bool frame = true);

    size_t nmeasures() const;
    bool hasLyrics();
    bool hasHarmonies();
    int  lyricCount();
    int  harmonyCount();
    String extractLyrics();
    int keysig();
    int duration();
    int durationWithoutRepeats();

    void cmdInsertClef(Clef* clef, ChordRest* cr);

    void cmdExplode();
    void cmdImplode();
    void cmdSlashFill();
    void cmdSlashRhythm();
    void cmdResequenceRehearsalMarks();
    void cmdExchangeVoice(voice_idx_t, voice_idx_t);
    void cmdRemoveEmptyTrailingMeasures();
    void cmdRealizeChordSymbols(bool lit = true, Voicing v = Voicing(-1), HDuration durationType = HDuration(-1));

    Measure* firstTrailingMeasure(ChordRest** cr = nullptr);
    ChordRest* cmdTopStaff(ChordRest* cr = nullptr);

    std::shared_ptr<muse::draw::Pixmap> createThumbnail();
    String createRehearsalMarkText(RehearsalMark* current) const;
    String nextRehearsalMarkText(RehearsalMark* previous, RehearsalMark* current) const;

    muse::Ret sanityCheckLocal();

    bool checkKeys();

    void switchToPageMode();

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

//    virtual QQueue<MidiInputEvent>* midiInputQueue();
    virtual std::list<MidiInputEvent>& activeMidiPitches();

    /// For MasterScores: returns the filename without extension
    /// For Scores: returns the excerpt name
    virtual String name() const;

    void cmdTimeDelete();
    bool checkTimeDelete(Segment* startSegment, Segment* endSegment);
    void doTimeDelete(Segment* startSegment, Segment* endSegment);
    void doTimeDeleteForMeasure(Measure*, Segment*, const Fraction&);

    Text* headerText(int index) const { return m_headersText[index]; }
    Text* footerText(int index) const { return m_footersText[index]; }
    void setHeaderText(Text* t, int index) { m_headersText.at(index) = t; }
    void setFooterText(Text* t, int index) { m_footersText.at(index) = t; }

    void cmdToggleVisible();
    void forAllLyrics(std::function<void(Lyrics*)> f);

    void createPaddingTable();
    const PaddingTable& paddingTable() const { return m_paddingTable; }

    void autoUpdateSpatium();

    const SystemLocks* systemLocks() const { return &m_systemLocks; }
    void addSystemLock(const SystemLock* lock);
    void removeSystemLock(const SystemLock* lock);
    void clearSystemLocks() { m_systemLocks.clear(); }

    void undoAddSystemLock(const SystemLock* lock);
    void undoRemoveSystemLock(const SystemLock* lock);
    void undoRemoveAllLocks();
    void toggleSystemLock(const std::vector<System*>& systems);
    void makeIntoSystem(MeasureBase* first, MeasureBase* last);
    void removeSystemLocksOnAddLayoutBreak(LayoutBreakType breakType, const MeasureBase* measure);
    void removeLayoutBreaksOnAddSystemLock(const SystemLock* lock);
    void removeSystemLocksOnRemoveMeasures(const MeasureBase* m1, const MeasureBase* m2);
    void updateSystemLocksOnDisableMMRests();
    void updateSystemLocksOnCreateMMRests(Measure* first, Measure* last);

    friend class Chord;

protected:

    friend class MasterScore;
    Score(const muse::modularity::ContextPtr& iocCtx);
    Score(MasterScore*, bool forcePartStyle = true);
    Score(MasterScore*, const MStyle&);

    // NOTE: Looks like this four are unused
    void cmdPitchUp();
    void cmdPitchDown();
    void cmdPitchUpOctave();
    void cmdPitchDownOctave();

    int m_fileDivision = 0;   // division of current loading *.msc file
    SynthesizerState m_synthesizerState;

private:

    friend class read302::Read302;
    friend class read400::Read400;
    friend class read410::Read410;
    friend class write::Writer;

    static std::set<Score*> validScores;

    Note* getSelectedNote();
    ChordRest* nextTrack(ChordRest* cr, bool skipMeasureRepeatRests = true);
    ChordRest* prevTrack(ChordRest* cr, bool skipMeasureRepeatRests = true);

    void addTempo();
    void addMetronome();

    void checkSlurs();
    void checkScore();

    bool rewriteMeasures(Measure* fm, Measure* lm, const Fraction&, staff_idx_t staffIdx);
    bool rewriteMeasures(Measure* fm, const Fraction& ns, staff_idx_t staffIdx);
    std::list<Fraction> splitGapToMeasureBoundaries(ChordRest*, Fraction);
    void pasteChordRest(ChordRest* cr, const Fraction& tick);

    void doSelect(EngravingItem* e, SelectType type, staff_idx_t staffIdx);
    void selectSingle(EngravingItem* e, staff_idx_t staffIdx);
    void selectAdd(EngravingItem* e);
    void selectRange(EngravingItem* e, staff_idx_t staffIdx);

    muse::Ret putNote(const Position&, bool replace);
    void handleOverlappingChordRest(InputState& inputState);

    void resetTempo();
    void resetTempoRange(const Fraction& tick1, const Fraction& tick2);
    void rebuildTempoAndTimeSigMaps(Measure* m, std::optional<BeatsPerSecond>& tempoPrimo);
    void fixAnacrusisTempo(const std::vector<Measure*>& measures) const;

    void doUndoRemoveStaleTieJumpPoints(Tie* tie);
    void doUndoResetPartialSlur(Slur* slur);

    void deleteOrShortenOutSpannersFromRange(const Fraction& t1, const Fraction& t2, track_idx_t trackStart, track_idx_t trackEnd,
                                             const SelectionFilter& filter);
    void deleteSlursFromRange(const Fraction& t1, const Fraction& t2, track_idx_t trackStart, track_idx_t trackEnd,
                              const SelectionFilter& filter);
    void deleteAnnotationsFromRange(Segment* segStart, Segment* segEnd, track_idx_t trackStart, track_idx_t trackEnd,
                                    const SelectionFilter& filter);

    void update(bool resetCmdState, bool layoutAllParts = false);

    muse::ID newStaffId() const;
    muse::ID newPartId() const;

    void assignIdIfNeed(Staff& staff) const;
    void assignIdIfNeed(Part& part) const;

    void updateStavesNumberForSystems();

    void applyAccidentalToInputNotes();

    int m_linkId = 0;
    MasterScore* m_masterScore = nullptr;
    std::list<MuseScoreView*> m_viewer;
    Excerpt* m_excerpt = nullptr;

    std::vector<Text*> m_headersText;
    std::vector<Text*> m_footersText;

    String m_mscoreVersion;
    int m_mscoreRevision = 0;

    std::shared_ptr<IEngravingFont> m_engravingFont = nullptr;

    int m_pageNumberOffset = 0;          // Offset for page numbers.

    UpdateState m_updateState;

    MeasureBaseList m_measures;            // here are the notes
    std::vector<Part*> m_parts;
    std::vector<Staff*> m_staves;
    std::vector<Staff*> m_systemObjectStaves;
    SystemLocks m_systemLocks;

    SpannerMap m_spanner;
    std::set<Spanner*> m_unmanagedSpanner;

    //
    // objects generated by layout:
    //
    std::vector<Page*> m_pages;            // pages are build from systems
    std::vector<System*> m_systems;        // measures are accumulated to systems

    InputState m_is;
    MStyle m_style;
    ChordList m_chordList;

    bool m_showInvisible = true;
    bool m_showUnprintable = true;
    bool m_showFrames = true;
    bool m_showPageborders = false;
    bool m_showSoundFlags = true;
    bool m_markIrregularMeasures = true;
    bool m_showInstrumentNames = true;
    bool m_printing = false;                // True if we are drawing to a printer
    bool m_savedCapture = false;            // True if we saved an image capture

    ShowAnchors m_showAnchors;

    ScoreOrder m_scoreOrder;                 // used for score ordering
    bool m_resetAutoplace = false;
    bool m_resetCrossBeams = false;
    int m_mscVersion = Constants::MSC_VERSION;     // version of current loading *.msc file

    bool m_isOpen = false;
    bool m_needSetUpTempoMap = true;

    std::map<String, String> m_metaTags;

    Selection m_selection;
    SelectionFilter m_selectionFilter;
    Audio* m_audio = nullptr;
    PlayMode m_playMode = PlayMode::SYNTHESIZER;

    RootItem* m_rootItem = nullptr;
    LayoutOptions m_layoutOptions;

    muse::async::Channel<EngravingItem*> m_elementDestroyed;

    ShadowNote* m_shadowNote = nullptr;

    muse::async::Channel<LoopBoundaryType, unsigned> m_loopBoundaryTickChanged;

    muse::async::Channel<float> m_layoutProgressChannel;

    PaddingTable m_paddingTable;
    double m_minimumPaddingUnit = 0.0;

    bool m_updatesLocked = false;
};

static inline Score* toScore(EngravingObject* e)
{
    assert(!e || e->isScore());
    return static_cast<Score*>(e);
}

static inline const Score* toScore(const EngravingObject* e)
{
    assert(!e || e->isScore());
    return static_cast<const Score*>(e);
}

//---------------------------------------------------------
//   ScoreLoad
//---------------------------------------------------------

class ScoreLoad
{
    static int m_loading;

public:
    ScoreLoad() { ++m_loading; }
    ~ScoreLoad() { --m_loading; }
    static bool loading() { return m_loading > 0; }
};

DECLARE_OPERATORS_FOR_FLAGS(LayoutFlags)
} // namespace mu::engraving
