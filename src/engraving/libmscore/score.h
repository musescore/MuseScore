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

#ifndef __SCORE_H__
#define __SCORE_H__

/**
 \file
 Definition of Score class.
*/

#include <set>
#include <QFileInfo>
#include <QQueue>
#include <QSet>

#include "config.h"
#include "input.h"
#include "instrument.h"
#include "scoreorder.h"
#include "select.h"
#include "synthesizerstate.h"
#include "mscoreview.h"
#include "spannermap.h"
#include "layoutbreak.h"
#include "property.h"
#include "chordlist.h"

#include "io/mscwriter.h"
#include "io/mscreader.h"
#include "engraving/draw/iimageprovider.h"
#include "layout/layout.h"
#include "layout/layoutoptions.h"

class QMimeData;

namespace mu::engraving {
class EngravingProject;
class AccessibleScore;
}

namespace mu::engraving::compat {
class ScoreAccess;
class ReadScoreHook;
class WriteScoreHook;
class Read302;
}

namespace Ms {
namespace Avs {
class AvsOmr;
}
class Articulation;
class Audio;
class BarLine;
class Beam;
class Bracket;
class BSymbol;
class Chord;
class ChordRest;
class Clef;
class Dynamic;
class ElementList;
class EventMap;
class Excerpt;
class FiguredBass;
class Fingering;
class Hairpin;
class Harmony;
class Instrument;
class KeyList;
class KeySig;
class KeySigEvent;
class LinkedElements;
class Lyrics;
class MasterSynthesizer;
class Measure;
class MeasureBase;
class MuseScoreView;
class Note;
class Omr;
class Page;
class Parameter;
class Part;
class RepeatList;
class Rest;
class Revisions;
class ScoreFont;
class Segment;
class Selection;
class SigEvent;
class Slur;
class Spanner;
class Staff;
class System;
class TempoMap;
class Text;
class TimeSig;
class TimeSigMap;
class Tuplet;
class Undo;
class UndoCommand;
class UndoStack;
class Volta;
class XmlWriter;
class Channel;
struct Interval;
struct TEvent;

enum class Tid;
enum class ClefType : signed char;
enum class BeatType : char;
enum class Key;
enum class HairpinType : signed char;
enum class SegmentType;
enum class OttavaType : char;
enum class Voicing : signed char;
enum class HDuration : signed char;

enum class POS : char {
    CURRENT, LEFT, RIGHT
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
    DOTDOT,
    DOT3,
    DOT4
};

//---------------------------------------------------------
//   MeasureBaseList
//---------------------------------------------------------

class MeasureBaseList
{
    int _size;
    MeasureBase* _first = nullptr;
    MeasureBase* _last = nullptr;

    void push_back(MeasureBase* e);
    void push_front(MeasureBase* e);

public:
    MeasureBaseList();
    MeasureBase* first() const { return _first; }
    MeasureBase* last()  const { return _last; }
    void clear() { _first = _last = 0; _size = 0; }
    void add(MeasureBase*);
    void remove(MeasureBase*);
    void insert(MeasureBase*, MeasureBase*);
    void remove(MeasureBase*, MeasureBase*);
    void change(MeasureBase* o, MeasureBase* n);
    int size() const { return _size; }
    bool empty() const { return _size == 0; }
    void fixupSystems();
};

//---------------------------------------------------------
//   MidiMapping
//---------------------------------------------------------

class MidiMapping
{
    Part* _part;
    std::unique_ptr<Channel> _articulation;
    signed char _port;
    signed char _channel;
    Channel* masterChannel;
    PartChannelSettingsLink link;

    MidiMapping() = default;   // should be created only within MasterScore
    friend class MasterScore;

public:
    Part* part() { return _part; }
    const Part* part() const { return _part; }
    Channel* articulation() { return _articulation.get(); }
    const Channel* articulation() const { return _articulation.get(); }
    signed char port() const { return _port; }
    signed char channel() const { return _channel; }
};

//---------------------------------------------------------
//   MidiInputEvent
//---------------------------------------------------------

struct MidiInputEvent {
    int pitch;
    bool chord;
    int velocity;
};

//---------------------------------------------------------
//   Position
//---------------------------------------------------------

struct Position {
    Segment* segment { 0 };
    int staffIdx     { -1 };
    int line         { 0 };
    int fret         { INVALID_FRET_INDEX };
    mu::PointF pos;
};

//---------------------------------------------------------
//   LayoutFlag bits
//---------------------------------------------------------

enum class LayoutFlag : char {
    NO_FLAGS       = 0,
    FIX_PITCH_VELO = 1,
    PLAY_EVENTS    = 2,
    REBUILD_MIDI_MAPPING = 4,
};

typedef QFlags<LayoutFlag> LayoutFlags;

//---------------------------------------------------------
//   PlayMode
//---------------------------------------------------------

enum class PlayMode : char {
    SYNTHESIZER,
    AUDIO
};

//---------------------------------------------------------
//   Layer
//---------------------------------------------------------

struct Layer {
    QString name;
    uint tags = 0;
};

//---------------------------------------------------------
//   UpdateMode
//    There is an implied order from least invasive update
//    to most invasive update. LayoutAll is fallback and
//    recreates all.
//---------------------------------------------------------

enum class UpdateMode {
    DoNothing,
    Update,             // do screen refresh of RectF "refresh"
    UpdateAll,          // do complete screen refresh
    Layout,             // do partial layout for tick range
};

//---------------------------------------------------------
//   CmdState
//
//    the following variables are reset on startCmd()
//    modified during cmd processing and used in endCmd() to
//    determine what to layout and what to repaint:
//---------------------------------------------------------

class CmdState
{
    UpdateMode _updateMode { UpdateMode::DoNothing };
    Fraction _startTick { -1, 1 };            // start tick for mode LayoutTick
    Fraction _endTick   { -1, 1 };              // end tick for mode LayoutTick
    int _startStaff = -1;
    int _endStaff = -1;
    const Element* _el = nullptr;
    const MeasureBase* _mb = nullptr;
    bool _oneElement = true;
    bool _oneMeasureBase = true;

    bool _locked = false;

    void setMeasureBase(const MeasureBase* mb);

public:
    LayoutFlags layoutFlags;

    bool _excerptsChanged     { false };
    bool _instrumentsChanged  { false };

    void reset();
    UpdateMode updateMode() const { return _updateMode; }
    void setUpdateMode(UpdateMode m);
    void _setUpdateMode(UpdateMode m);
    bool layoutRange() const { return _updateMode == UpdateMode::Layout; }
    bool updateAll() const { return int(_updateMode) >= int(UpdateMode::UpdateAll); }
    bool updateRange() const { return _updateMode == UpdateMode::Update; }
    void setTick(const Fraction& t);
    void setStaff(int staff);
    void setElement(const Element* e);
    void unsetElement(const Element* e);
    Fraction startTick() const { return _startTick; }
    Fraction endTick() const { return _endTick; }
    int startStaff() const { return _startStaff; }
    int endStaff() const { return _endStaff; }
    const Element* element() const;

    void lock() { _locked = true; }
    void unlock() { _locked = false; }
#ifndef NDEBUG
    void dump();
#endif
};

//---------------------------------------------------------
//   UpdateState
//---------------------------------------------------------

class UpdateState
{
public:
    mu::RectF refresh;                 ///< area to update, canvas coordinates
    bool _playNote   { false };     ///< play selected note after command
    bool _playChord  { false };     ///< play whole chord for the selected note
    bool _selectionChanged { false };
    QList<ScoreElement*> _deleteList;
};

//---------------------------------------------------------
//   ScoreContentState
//---------------------------------------------------------

class ScoreContentState
{
    const Score* score;
    int num;
public:
    ScoreContentState()
        : score(nullptr), num(0) {}
    ScoreContentState(const Score* s, int stateNum)
        : score(s), num(stateNum) {}

    bool operator==(const ScoreContentState& s2) const { return score == s2.score && num == s2.num; }
    bool operator!=(const ScoreContentState& s2) const { return !(*this == s2); }

    bool isNewerThan(const ScoreContentState& s2) const { return score == s2.score && num > s2.num; }
};

class MasterScore;

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

class Score : public QObject, public ScoreElement
{
    Q_OBJECT

    INJECT(engraving, mu::draw::IImageProvider, imageProvider)
public:
    enum class FileError : char {
        FILE_NO_ERROR,
        FILE_ERROR,
        FILE_NOT_FOUND,
        FILE_OPEN_ERROR,
        FILE_BAD_FORMAT,
        FILE_UNKNOWN_TYPE,
        FILE_NO_ROOTFILE,
        FILE_TOO_OLD,
        FILE_TOO_NEW,
        FILE_OLD_300_FORMAT,
        FILE_CORRUPTED,
        FILE_CRITICALLY_CORRUPTED,
        FILE_USER_ABORT,
        FILE_IGNORE_ERROR
    };

private:

    friend class mu::engraving::compat::ReadScoreHook;
    friend class mu::engraving::compat::Read302;
    friend class mu::engraving::Layout;

    static std::set<Score*> validScores;
    int _linkId { 0 };
    MasterScore* _masterScore { 0 };
    QList<MuseScoreView*> viewer;
    Excerpt* _excerpt  { 0 };

    std::vector<Text*> _headersText;
    std::vector<Text*> _footersText;

    QString _mscoreVersion;
    int _mscoreRevision;

    QString _layerTags[32];
    QString _layerTagComments[32];
    QList<Layer> _layer;
    int _currentLayer { 0 };

    ScoreFont* _scoreFont;
    int _pageNumberOffset { 0 };          ///< Offset for page numbers.

    UpdateState _updateState;

    MeasureBaseList _measures;            // here are the notes
    QList<Part*> _parts;
    QList<Staff*> _staves;

    SpannerMap _spanner;
    std::set<Spanner*> _unmanagedSpanner;

    //
    // objects generated by layout:
    //
    QList<Page*> _pages;            // pages are build from systems
    QList<System*> _systems;        // measures are accumulated to systems

    InputState _is;
    MStyle _style;
    ChordList _chordList;

    bool _created { false };            ///< file is never saved, has generated name
    bool _startedEmpty { false };       ///< The score was created from an empty template (typically ":/data/My_First_Score.mscx")
                                        /// during this session, so it doesn't need to be saved if it hasn't been modified.
    QString _tmpName;                   ///< auto saved with this name if not empty
    QString _importedFilePath;          // file from which the score was imported, or empty

    bool _showInvisible         { true };
    bool _showUnprintable       { true };
    bool _showFrames            { true };
    bool _showPageborders       { false };
    bool _markIrregularMeasures { true };
    bool _showInstrumentNames   { true };
    bool _printing              { false };        ///< True if we are drawing to a printer
    bool _autosaveDirty         { true };
    bool _savedCapture          { false };        ///< True if we saved an image capture
    bool _saved                 { false };      ///< True if project was already saved; only on first
                                                ///< save a backup file will be created, subsequent
                                                ///< saves will not overwrite the backup file.
    bool _defaultsRead        { false };        ///< defaults were read at MusicXML import, allow export of defaults in convertermode
    bool _isPalette           { false };
    ScoreOrder _scoreOrder;                     ///< used for score ordering

    int _mscVersion { MSCVERSION };     ///< version of current loading *.msc file

    QMap<QString, QString> _metaTags;

    constexpr static double _defaultTempo = 2.0;   //default tempo is equal 120 bpm

    Selection _selection;
    SelectionFilter _selectionFilter;
    Audio* _audio { 0 };
    PlayMode _playMode { PlayMode::SYNTHESIZER };

    qreal _noteHeadWidth { 0.0 };         // cached value
    QString accInfo;                      ///< information about selected element(s) for use by screen-readers
    QString accMessage;                   ///< temporary status message for use by screen-readers

    mu::engraving::AccessibleScore* m_accessible = nullptr;

    mu::engraving::Layout m_layout;
    mu::engraving::LayoutOptions m_layoutOptions;

    ChordRest* nextMeasure(ChordRest* element, bool selectBehavior = false, bool mmRest = false);
    ChordRest* prevMeasure(ChordRest* element, bool mmRest = false);
    void cmdResetAllStyle();

    Note* getSelectedNote();
    ChordRest* upStaff(ChordRest* cr);
    ChordRest* downStaff(ChordRest* cr);
    ChordRest* nextTrack(ChordRest* cr, bool skipMeasureRepeatRests = true);
    ChordRest* prevTrack(ChordRest* cr, bool skipMeasureRepeatRests = true);

    void addTempo();
    void addMetronome();

    void cmdInsertClef(ClefType);
    void removeChordRest(ChordRest* cr, bool clearSegment);
    void cmdMoveRest(Rest*, Direction);
    void cmdMoveLyrics(Lyrics*, Direction);

    void checkScore();

    bool rewriteMeasures(Measure* fm, Measure* lm, const Fraction&, int staffIdx);
    bool rewriteMeasures(Measure* fm, const Fraction& ns, int staffIdx);
    void swingAdjustParams(Chord*, int&, int&, int, int);
    bool isSubdivided(ChordRest*, int);
    QList<Fraction> splitGapToMeasureBoundaries(ChordRest*, Fraction);
    void pasteChordRest(ChordRest* cr, const Fraction& tick, const Interval&);

    void selectSingle(Element* e, int staffIdx);
    void selectAdd(Element* e);
    void selectRange(Element* e, int staffIdx);

    void cmdToggleVisible();

    void putNote(const Position&, bool replace);

    void resetTempo();
    void resetTempoRange(const Fraction& tick1, const Fraction& tick2);

    void deleteSpannersFromRange(const Fraction& t1, const Fraction& t2, int trackStart, int trackEnd, const SelectionFilter& filter);
    void deleteAnnotationsFromRange(Segment* segStart, Segment* segEnd, int trackStart, int trackEnd, const SelectionFilter& filter);
    ChordRest* deleteRange(Segment* segStart, Segment* segEnd, int trackStart, int trackEnd, const SelectionFilter& filter);

    void update(bool resetCmdState);

    ID newStaffId() const;
    ID newPartId() const;

    void assignIdIfNeed(Staff& staff) const;
    void assignIdIfNeed(Part& part) const;

protected:
    int _fileDivision;   ///< division of current loading *.msc file
    SynthesizerState _synthesizerState;

    void createPlayEvents(Chord*);
    void createGraceNotesPlayEvents(const Fraction& tick, Chord* chord, int& ontime, int& trailtime);
    void cmdPitchUp();
    void cmdPitchDown();
    void cmdPitchUpOctave();
    void cmdPitchDownOctave();
    void cmdPadNoteIncreaseTAB(const EditData& ed);
    void cmdPadNoteDecreaseTAB(const EditData& ed);
    void cmdToggleMmrest();
    void cmdToggleHideEmpty();
    void cmdSetVisible();
    void cmdUnsetVisible();

    friend class MasterScore;
    Score();
    Score(MasterScore*, bool forcePartStyle = true);
    Score(MasterScore*, const MStyle&);

signals:
    void posChanged(POS, unsigned);
    void playlistChanged();

public:
    Score(const Score&) = delete;
    Score& operator=(const Score&) = delete;
    virtual ~Score();
    Score* clone();

    static Score* paletteScore();
    bool isPaletteScore() const;

    virtual bool isMaster() const { return false; }
    virtual bool readOnly() const;

    static void onElementDestruction(Element* se);

    // Score Tree functions
    ScoreElement* treeParent() const override;
    ScoreElement* treeChild(int idx) const override;
    int treeChildCount() const override;
    void dumpScoreTree();  // for debugging purposes

    virtual ElementType type() const override { return ElementType::SCORE; }

    void rebuildBspTree();
    bool noStaves() const { return _staves.empty(); }
    void insertPart(Part*, int);
    void appendPart(Part*);
    void removePart(Part*);
    void insertStaff(Staff*, int);
    void appendStaff(Staff*);
    void cmdRemoveStaff(int staffIdx);
    void removeStaff(Staff*);
    void addMeasure(MeasureBase*, MeasureBase*);
    void readStaff(XmlReader&);
    void linkMeasures(Score* score);

    Excerpt* excerpt() { return _excerpt; }
    void setExcerpt(Excerpt* e) { _excerpt = e; }

    void resetAllPositions();

    void cmdAddBracket();
    void cmdAddParentheses();
    void cmdAddBraces();
    void cmdAddFret(int fret);
    void cmdSetBeamMode(Beam::Mode);
    void cmdRemovePart(Part*);
    void cmdAddTie(bool addToChord = false);
    void cmdToggleTie();
    static std::vector<Note*> cmdTieNoteList(const Selection& selection, bool noteEntryMode);
    void cmdAddOttava(OttavaType);
    void addHairpin(HairpinType);
    void addNoteLine();
    void padToggle(Pad p, const EditData& ed);
    void cmdAddPitch(const EditData&, int note, bool addFlag, bool insert);
    void cmdAddStretch(qreal);
    void cmdAddGrace(NoteType, int);
    void cmdResetNoteAndRestGroupings();
    void cmdResetAllPositions(bool undoable = true);
    void cmdDoubleDuration() { cmdIncDecDuration(-1, false); }
    void cmdHalfDuration() { cmdIncDecDuration(1, false); }
    void cmdIncDurationDotted() { cmdIncDecDuration(-1, true); }
    void cmdDecDurationDotted() { cmdIncDecDuration(1, true); }
    void cmdIncDecDuration(int nSteps, bool stepDotted = false);
    void cmdToggleLayoutBreak(LayoutBreak::Type);
    void cmdAddMeasureRepeat(Measure*, int numMeasures, int staffIdx);
    bool makeMeasureRepeatGroup(Measure*, int numMeasures, int staffIdx);
    void cmdFlip();
    void resetUserStretch();
    void cmdResetBeamMode();
    void cmdResetTextStyleOverrides();

    void addRemoveBreaks(int interval, bool lock);

    bool transpose(Note* n, Interval, bool useSharpsFlats);
    void transposeKeys(int staffStart, int staffEnd, const Fraction& tickStart, const Fraction& tickEnd, const Interval&,
                       bool useInstrument = false, bool flip = false);
    bool transpose(TransposeMode mode, TransposeDirection, Key transposeKey, int transposeInterval, bool trKeys, bool transposeChordNames,
                   bool useDoubleSharpsFlats);

    bool appendMeasuresFromScore(Score* score, const Fraction& startTick, const Fraction& endTick);
    bool appendScore(Score*, bool addPageBreak = false, bool addSectionBreak = true);

    void write(XmlWriter&, bool onlySelection, mu::engraving::compat::WriteScoreHook& hook);
    bool writeScore(QIODevice* f, bool msczFormat, bool onlySelection, mu::engraving::compat::WriteScoreHook& hook);

    bool read400(XmlReader& e);
    bool readScore400(XmlReader& e);

    const QList<Staff*>& staves() const { return _staves; }
    int nstaves() const { return _staves.size(); }
    int ntracks() const { return _staves.size() * VOICES; }

    int staffIdx(const Part*) const;
    Staff* staff(int n) const { return ((n >= 0) && (n < _staves.size())) ? _staves.at(n) : nullptr; }
    Staff* staffById(const ID& staffId) const;
    Part* partById(const ID& partId) const;

    Measure* pos2measure(const mu::PointF&, int* staffIdx, int* pitch, Segment**, mu::PointF* offset) const;
    void dragPosition(const mu::PointF&, int* staffIdx, Segment**, qreal spacingFactor = 0.5) const;

    void undoAddElement(Element* element);
    void undoAddCR(ChordRest* element, Measure*, const Fraction& tick);
    void undoRemoveElement(Element* element);
    void undoChangeSpannerElements(Spanner* spanner, Element* startElement, Element* endElement);
    void undoChangeElement(Element* oldElement, Element* newElement);
    void undoChangePitch(Note* note, int pitch, int tpc1, int tpc2);
    void undoChangeFretting(Note* note, int pitch, int string, int fret, int tpc1, int tpc2);
    void spellNotelist(std::vector<Note*>& notes);
    void undoChangeTpc(Note* note, int tpc);
    void undoChangeChordRestLen(ChordRest* cr, const TDuration&);
    void undoTransposeHarmony(Harmony*, int, int);
    void undoExchangeVoice(Measure* measure, int val1, int val2, int staff1, int staff2);
    void undoRemovePart(Part* part, int idx = -1);
    void undoInsertPart(Part* part, int idx);
    void undoRemoveStaff(Staff* staff);
    void undoInsertStaff(Staff* staff, int idx, bool createRests=true);
    void undoChangeInvisible(Element*, bool);
    void undoChangeTuning(Note*, qreal);
    void undoChangeUserMirror(Note*, MScore::DirectionH);
    void undoChangeKeySig(Staff* ostaff, const Fraction& tick, KeySigEvent);
    void undoChangeClef(Staff* ostaff, Element*, ClefType st, bool forInstrumentChange = false);
    bool undoPropertyChanged(Element* e, Pid t, const QVariant& st, PropertyFlags ps = PropertyFlags::NOSTYLE);
    void undoPropertyChanged(ScoreElement*, Pid, const QVariant& v, PropertyFlags ps = PropertyFlags::NOSTYLE);
    virtual UndoStack* undoStack() const;
    void undo(UndoCommand*, EditData* = 0) const;
    void undoRemoveMeasures(Measure*, Measure*, bool preserveTies = false);
    void undoChangeMeasureRepeatCount(Measure* m, int count, int staffIdx);
    void undoAddBracket(Staff* staff, int level, BracketType type, int span);
    void undoRemoveBracket(Bracket*);
    void undoInsertTime(const Fraction& tick, const Fraction& len);
    void undoChangeStyleVal(Sid idx, const QVariant& v);
    void undoChangePageNumberOffset(int po);

    void updateInstrumentChangeTranspositions(Ms::KeySigEvent& key, Ms::Staff* staff, const Ms::Fraction& tick);

    Note* setGraceNote(Chord*,  int pitch, NoteType type, int len);

    Segment* setNoteRest(Segment*, int track, NoteVal nval, Fraction, Direction stemDirection = Direction::AUTO,
                         bool forceAccidental = false, const std::set<SymId>& articulationIds = {}, bool rhythmic = false,
                         InputState* externalInputState = nullptr);
    Segment* setChord(Segment*, int track, Chord* chord, Fraction, Direction stemDirection = Direction::AUTO);
    void changeCRlen(ChordRest* cr, const TDuration&);
    void changeCRlen(ChordRest* cr, const Fraction&, bool fillWithRest=true);
    void createCRSequence(const Fraction& f, ChordRest* cr, const Fraction& tick);

    Fraction makeGap(Segment*, int track, const Fraction&, Tuplet*, bool keepChord = false);
    bool makeGap1(const Fraction& baseTick, int staffIdx, const Fraction& len, int voiceOffset[VOICES]);
    bool makeGapVoice(Segment* seg, int track, Fraction len, const Fraction& tick);

    Rest* addRest(const Fraction& tick, int track, TDuration, Tuplet*);
    Rest* addRest(Segment* seg, int track, TDuration d, Tuplet*);
    Chord* addChord(const Fraction& tick, TDuration d, Chord* oc, bool genTie, Tuplet* tuplet);
    MeasureRepeat* addMeasureRepeat(const Fraction& tick, int track, int numMeasures);

    Tuplet* addTuplet(ChordRest* destinationChordRest, Fraction ratio, TupletNumberType numberType, TupletBracketType bracketType);

    ChordRest* addClone(ChordRest* cr, const Fraction& tick, const TDuration& d);
    Rest* setRest(const Fraction& tick,  int track, const Fraction&, bool useDots, Tuplet* tuplet, bool useFullMeasureRest = true);

    void upDown(bool up, UpDownMode);
    void upDownDelta(int pitchDelta);
    ChordRest* searchNote(const Fraction& tick, int track) const;

    // undo/redo ops
    void addArticulation(SymId);
    bool addArticulation(Element*, Articulation* atr);
    void toggleAccidental(AccidentalType, const EditData& ed);
    void changeAccidental(AccidentalType);
    void changeAccidental(Note* oNote, Ms::AccidentalType);

    void addElement(Element*);
    void removeElement(Element*);

    Note* addPitch(NoteVal&, bool addFlag, InputState* externalInputState = nullptr);
    void addPitch(int pitch, bool addFlag, bool insert);
    Note* addTiedMidiPitch(int pitch, bool addFlag, Chord* prevChord);
    Note* addMidiPitch(int pitch, bool addFlag);
    Note* addNote(Chord*, const NoteVal& noteVal, bool forceAccidental = false, const std::set<SymId>& articulationIds = {},
                  InputState* externalInputState = nullptr);

    NoteVal noteValForPosition(Position pos, AccidentalType at, bool& error);

    Slur* addSlur(ChordRest* firstChordRest, ChordRest* secondChordRest, const Slur* slurTemplate);
    TextBase* addText(Tid type);

    void deleteItem(Element*);
    void deleteMeasures(MeasureBase* firstMeasure, MeasureBase* lastMeasure, bool preserveTies = false);
    void cmdDeleteSelection();
    void cmdFullMeasureRest();

    void putNote(const mu::PointF&, bool replace, bool insert);
    void insertChord(const Position&);
    void localInsertChord(const Position&);
    void globalInsertChord(const Position&);

    void cloneVoice(int strack, int dtrack, Segment* sf, const Fraction& lTick, bool link = true, bool spanner = true);

    void repitchNote(const Position& pos, bool replace);
    void regroupNotesAndRests(const Fraction& startTick, const Fraction& endTick, int track);
    bool checkTimeDelete(Segment*, Segment*);
    void timeDelete(Measure*, Segment*, const Fraction&);

    void startCmd();                    // start undoable command
    void endCmd(bool rollback = false); // end undoable command
    void update() { update(true); }
    void undoRedo(bool undo, EditData*);

    void cmdRemoveTimeSig(TimeSig*);
    void cmdAddTimeSig(Measure*, int staffIdx, TimeSig*, bool local);

    virtual void setUpdateAll();
    void setLayoutAll(int staff = -1, const Element* e = nullptr);
    void setLayout(const Fraction& tick, int staff, const Element* e = nullptr);
    void setLayout(const Fraction& tick1, const Fraction& tick2, int staff1, int staff2, const Element* e = nullptr);
    virtual CmdState& cmdState();
    virtual const CmdState& cmdState() const;
    virtual void addLayoutFlags(LayoutFlags);
    virtual void setInstrumentsChanged(bool);
    void addRefresh(const mu::RectF&);

    void cmdRelayout();
    void cmdToggleAutoplace(bool all);

    bool playNote() const { return _updateState._playNote; }
    void setPlayNote(bool v) { _updateState._playNote = v; }
    bool playChord() const { return _updateState._playChord; }
    void setPlayChord(bool v) { _updateState._playChord = v; }
    bool selectionChanged() const { return _updateState._selectionChanged; }
    void setSelectionChanged(bool val) { _updateState._selectionChanged = val; }
    void deleteLater(ScoreElement* e) { _updateState._deleteList.push_back(e); }
    void deletePostponed();

    void changeSelectedNotesVoice(int);

    const QList<Part*>& parts() const { return _parts; }

    void appendPart(const InstrumentTemplate*);
    void updateStaffIndex();
    void sortStaves(QList<int>& dst);
    void mapExcerptTracks(QList<int>& l);

    bool showInvisible() const { return _showInvisible; }
    bool showUnprintable() const { return _showUnprintable; }
    bool showFrames() const { return _showFrames; }
    bool showPageborders() const { return _showPageborders; }
    bool markIrregularMeasures() const { return _markIrregularMeasures; }
    bool showInstrumentNames() const { return _showInstrumentNames; }
    void setShowInvisible(bool v);
    void setShowUnprintable(bool v);
    void setShowFrames(bool v);
    void setShowPageborders(bool v);
    void setMarkIrregularMeasures(bool v);
    void setShowInstrumentNames(bool v) { _showInstrumentNames = v; }

    void print(mu::draw::Painter* printer, int page);
    ChordRest* getSelectedChordRest() const;
    QSet<ChordRest*> getSelectedChordRests() const;
    void getSelectedChordRest2(ChordRest** cr1, ChordRest** cr2) const;

    void select(Element* obj, SelectType = SelectType::SINGLE, int staff = 0);
    void selectSimilar(Element* e, bool sameStaff);
    void selectSimilarInRange(Element* e);
    static void collectMatch(void* data, Element* e);
    static void collectNoteMatch(void* data, Element* e);
    void deselect(Element* obj);
    void deselectAll() { _selection.deselectAll(); }
    void updateSelection() { _selection.update(); }
    Element* getSelectedElement() const { return _selection.element(); }
    const Selection& selection() const { return _selection; }
    Selection& selection() { return _selection; }
    SelectionFilter& selectionFilter() { return _selectionFilter; }
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
    Segment* tick2leftSegment(const Fraction& tick, bool useMMrest = false) const;
    Segment* tick2rightSegment(const Fraction& tick, bool useMMrest = false) const;
    Segment* tick2leftSegmentMM(const Fraction& tick) { return tick2leftSegment(tick, /* useMMRest */ true); }
    void fixTicks();
    void rebuildTempoAndTimeSigMaps(Measure* m);
    Element* nextElement();
    Element* prevElement();
    ChordRest* cmdNextPrevSystem(ChordRest*, bool);
    Box* cmdNextPrevFrame(MeasureBase*, bool) const;
    Element* cmdNextPrevSection(Element*, bool) const;
    MeasureBase* getNextPrevSectionBreak(MeasureBase*, bool) const;
    Element* getScoreElementOfMeasureBase(MeasureBase*) const;

    void cmd(const QString&, EditData&);
    int fileDivision(int t) const { return ((qint64)t * MScore::division + _fileDivision / 2) / _fileDivision; }
    void setFileDivision(int t) { _fileDivision = t; }

    QString importedFilePath() const { return _importedFilePath; }
    void setImportedFilePath(const QString& filePath);

    bool dirty() const;
    ScoreContentState state() const;
    void setCreated(bool val) { _created = val; }
    bool created() const { return _created; }
    void setStartedEmpty(bool val) { _startedEmpty = val; }
    bool startedEmpty() const { return _startedEmpty; }
    bool savedCapture() const { return _savedCapture; }
    bool saved() const { return _saved; }
    void setSaved(bool v) { _saved = v; }
    void setSavedCapture(bool v) { _savedCapture = v; }
    bool printing() const { return _printing; }
    void setPrinting(bool val) { _printing = val; }
    void setAutosaveDirty(bool v) { _autosaveDirty = v; }
    bool autosaveDirty() const { return _autosaveDirty; }
    virtual bool playlistDirty() const;
    virtual void setPlaylistDirty();

    void spell();
    void spell(int startStaff, int endStaff, Segment* startSegment, Segment* endSegment);
    void spell(Note*);
    void changeEnharmonicSpelling(bool both);

    Fraction nextSeg(const Fraction& tick, int track);

    ChordList* chordList() { return &_chordList; }
    const ChordList* chordList() const { return &_chordList; }
    void checkChordList() { _chordList.checkChordList(style()); }

    virtual MStyle& style() { return _style; }
    virtual const MStyle& style() const { return _style; }

    void resetAllStyle();
    void resetStyles(const QSet<Sid>& stylesToReset);

    void setStyle(const MStyle& s, const bool overlap = false);
    bool loadStyle(const QString&, bool ign = false, const bool overlap = false);
    bool saveStyle(const QString&);

    const QVariant& styleV(Sid idx) const { return style().styleV(idx); }
    Spatium  styleS(Sid idx) const { return style().styleS(idx); }
    qreal styleP(Sid idx) const { return style().styleP(idx); }
    QString styleSt(Sid idx) const { return style().styleSt(idx); }
    bool styleB(Sid idx) const { return style().styleB(idx); }
    qreal styleD(Sid idx) const { return style().styleD(idx); }
    int styleI(Sid idx) const { return style().styleI(idx); }

    void setStyleValue(Sid sid, QVariant value) { style().set(sid, value); }
    QString getTextStyleUserName(Tid tid);
    qreal spatium() const { return styleD(Sid::spatium); }
    void setSpatium(qreal v) { setStyleValue(Sid::spatium, v); }

    bool genCourtesyTimesig() const { return styleB(Sid::genCourtesyTimesig); }
    bool genCourtesyClef() const { return styleB(Sid::genCourtesyClef); }

    // These position are in ticks and not uticks
    Fraction playPos() const { return pos(POS::CURRENT); }
    void setPlayPos(const Fraction& tick) { setPos(POS::CURRENT, tick); }
    Fraction loopInTick() const { return pos(POS::LEFT); }
    Fraction loopOutTick() const { return pos(POS::RIGHT); }
    void setLoopInTick(const Fraction& tick) { setPos(POS::LEFT, tick); }
    void setLoopOutTick(const Fraction& tick) { setPos(POS::RIGHT, tick); }

    Fraction pos(POS pos) const;
    void setPos(POS pos, Fraction tick);

    bool noteEntryMode() const { return inputState().noteEntryMode(); }
    void setNoteEntryMode(bool val) { inputState().setNoteEntryMode(val); }
    NoteEntryMethod noteEntryMethod() const { return inputState().noteEntryMethod(); }
    void setNoteEntryMethod(NoteEntryMethod m) { inputState().setNoteEntryMethod(m); }
    bool usingNoteEntryMethod(NoteEntryMethod m) { return inputState().usingNoteEntryMethod(m); }
    Fraction inputPos() const;
    int inputTrack() const { return inputState().track(); }
    const InputState& inputState() const { return _is; }
    InputState& inputState() { return _is; }
    void setInputState(const InputState& st) { _is = st; }
    void setInputTrack(int t) { inputState().setTrack(t); }

    void spatiumChanged(qreal oldValue, qreal newValue);
    void styleChanged() override;

    void cmdPaste(const QMimeData* ms, MuseScoreView* view, Fraction scale = Fraction(1, 1));
    bool pasteStaff(XmlReader&, Segment* dst, int staffIdx, Fraction scale = Fraction(1, 1));
    void readAddConnector(ConnectorInfoReader* info, bool pasteMode) override;
    void pasteSymbols(XmlReader& e, ChordRest* dst);
    void renderMidi(EventMap* events, const SynthesizerState& synthState);
    void renderMidi(EventMap* events, bool metronome, bool expandRepeats, const SynthesizerState& synthState);

    BeatType tick2beatType(const Fraction& tick);

    int mscVersion() const { return _mscVersion; }
    void setMscVersion(int v) { _mscVersion = v; }

    void addLyrics(const Fraction& tick, int staffIdx, const QString&);

    void updateSwing();
    void createPlayEvents(Measure const* start = nullptr, Measure const* const end = nullptr);

    void updateCapo();
    void updateVelo();
    void updateChannel();

    void cmdConcertPitchChanged(bool);

    virtual TempoMap* tempomap() const;
    virtual TimeSigMap* sigmap() const;

    void setTempo(Segment*, qreal);
    void setTempo(const Fraction& tick, qreal bps);
    void removeTempo(const Fraction& tick);
    void setPause(const Fraction& tick, qreal seconds);
    qreal tempo(const Fraction& tick) const;

    bool defaultsRead() const { return _defaultsRead; }
    void setDefaultsRead(bool b) { _defaultsRead = b; }
    Text* getText(Tid subtype) const;

    bool isPalette() const { return _isPalette; }
    void setPaletteMode(bool palette) { _isPalette = palette; }

    bool enableVerticalSpread() const;
    void setEnableVerticalSpread(bool val);
    qreal maxSystemDistance() const;

    ScoreOrder scoreOrder() const;
    void setScoreOrder(ScoreOrder order);
    void setBracketsAndBarlines();

    void lassoSelect(const mu::RectF&);
    void lassoSelectEnd(bool);

    Page* searchPage(const mu::PointF&) const;
    QList<System*> searchSystem(const mu::PointF& p, const System* preferredSystem = nullptr, qreal spacingFactor = 0.5,
                                qreal preferredSpacingFactor = 1.0) const;
    Measure* searchMeasure(const mu::PointF& p, const System* preferredSystem = nullptr, qreal spacingFactor = 0.5,
                           qreal preferredSpacingFactor = 1.0) const;

    bool getPosition(Position* pos, const mu::PointF&, int voice) const;

    void cmdDeleteTuplet(Tuplet*, bool replaceWithRest);
    Measure* getCreateMeasure(const Fraction& tick);

    void adjustBracketsDel(int sidx, int eidx);
    void adjustBracketsIns(int sidx, int eidx);
    void adjustKeySigs(int sidx, int eidx, KeyList km);
    KeyList keyList() const;

    virtual const RepeatList& repeatList() const;
    virtual const RepeatList& repeatList2() const;
    qreal utick2utime(int tick) const;
    int utime2utick(qreal utime) const;

    void nextInputPos(ChordRest* cr, bool);
    void cmdMirrorNoteHead();

    virtual int npages() const { return _pages.size(); }
    virtual int pageIdx(Page* page) const { return _pages.indexOf(page); }
    virtual const QList<Page*>& pages() const { return _pages; }
    virtual QList<Page*>& pages() { return _pages; }

    const QList<System*>& systems() const { return _systems; }
    QList<System*>& systems() { return _systems; }

    MeasureBaseList* measures() { return &_measures; }
    bool checkHasMeasures() const;
    MeasureBase* first() const;
    MeasureBase* firstMM() const;
    MeasureBase* last()  const;
    Ms::Measure* firstMeasure() const;
    Ms::Measure* firstMeasureMM() const;
    Ms::Measure* lastMeasure() const;
    Ms::Measure* lastMeasureMM() const;
    MeasureBase* measure(int idx) const;
    Measure* crMeasure(int idx) const;

    Fraction endTick() const;

    Segment* firstSegment(SegmentType s) const;
    Segment* firstSegmentMM(SegmentType s) const;
    Segment* lastSegment() const;
    Segment* lastSegmentMM() const;

    void connectTies(bool silent = false);
    void relayoutForStyles();

    qreal point(const Spatium sp) const { return sp.val() * spatium(); }

    void scanElementsInRange(void* data, void (* func)(void*, Element*), bool all = true);
    int fileDivision() const { return _fileDivision; }   ///< division of current loading *.msc file
    void splitStaff(int staffIdx, int splitPoint);
    QString tmpName() const { return _tmpName; }
    void setTmpName(const QString& s) { _tmpName = s; }
    Lyrics* addLyrics();
    FiguredBass* addFiguredBass();
    void expandVoice(Segment* s, int track);
    void expandVoice();

    Element* selectMove(const QString& cmd);
    Element* move(const QString& cmd);
    void cmdEnterRest(const TDuration& d);
    void enterRest(const TDuration& d, InputState* externalInputState = nullptr);
    void addInterval(int, const std::vector<Note*>&);
    void cmdCreateTuplet(ChordRest*, Tuplet*);
    void removeAudio();

    void doLayout();
    void doLayoutRange(const Fraction& st, const Fraction& et);

    SynthesizerState& synthesizerState() { return _synthesizerState; }
    void setSynthesizerState(const SynthesizerState& s);

    void updateHairpin(Hairpin*);         // add/modify hairpin to pitchOffset list

    MasterScore* masterScore() const { return _masterScore; }
    void setMasterScore(MasterScore* s) { _masterScore = s; }
    void writeSegments(XmlWriter& xml, int strack, int etrack, Segment* sseg, Segment* eseg, bool, bool);

    const QMap<QString, QString>& metaTags() const { return _metaTags; }
    QMap<QString, QString>& metaTags() { return _metaTags; }
    void setMetaTags(const QMap<QString, QString>& t) { _metaTags = t; }

    //@ returns as a string the metatag named 'tag'
    QString metaTag(const QString& tag) const;
    //@ sets the metatag named 'tag' to 'val'
    void setMetaTag(const QString& tag, const QString& val);

    void cmdSplitMeasure(ChordRest*);
    void splitMeasure(Segment*);
    void cmdJoinMeasure(Measure*, Measure*);

    int pageNumberOffset() const { return _pageNumberOffset; }
    void setPageNumberOffset(int v) { _pageNumberOffset = v; }

    QString mscoreVersion() const { return _mscoreVersion; }
    int mscoreRevision() const { return _mscoreRevision; }
    void setMscoreVersion(const QString& val) { _mscoreVersion = val; }
    void setMscoreRevision(int val) { _mscoreRevision = val; }

    uint currentLayerMask() const { return _layer[_currentLayer].tags; }
    void setCurrentLayer(int val) { _currentLayer = val; }
    int currentLayer() const { return _currentLayer; }
    QString* layerTags() { return _layerTags; }
    QString* layerTagComments() { return _layerTagComments; }
    QList<Layer>& layer() { return _layer; }
    const QList<Layer>& layer() const { return _layer; }
    bool tagIsValid(uint tag) const { return tag & _layer[_currentLayer].tags; }

    void addViewer(MuseScoreView* v) { viewer.append(v); }
    void removeViewer(MuseScoreView* v) { viewer.removeAll(v); }
    const QList<MuseScoreView*>& getViewer() const { return viewer; }

    //! NOTE Layout
    const mu::engraving::LayoutOptions& layoutOptions() const { return m_layoutOptions; }
    void setLayoutMode(mu::engraving::LayoutMode lm) { m_layoutOptions.mode = lm; }
    void setShowVBox(bool v) { m_layoutOptions.showVBox = v; }

    // temporary methods
    bool isLayoutMode(mu::engraving::LayoutMode lm) const { return m_layoutOptions.isMode(lm); }
    mu::engraving::LayoutMode layoutMode() const { return m_layoutOptions.mode; }
    bool floatMode() const { return m_layoutOptions.isMode(mu::engraving::LayoutMode::FLOAT); }
    bool pageMode() const { return m_layoutOptions.isMode(mu::engraving::LayoutMode::PAGE); }
    bool lineMode() const { return m_layoutOptions.isMode(mu::engraving::LayoutMode::LINE); }
    bool systemMode() const { return m_layoutOptions.isMode(mu::engraving::LayoutMode::SYSTEM); }
    // ----

    Tuplet* searchTuplet(XmlReader& e, int id);
    void cmdSelectAll();
    void cmdSelectSection();
    void transposeSemitone(int semitone);
    void transposeDiatonicAlterations(TransposeDirection direction);

    void insertMeasure(ElementType type, MeasureBase*, bool createEmptyMeasures = false, bool moveSignaturesClef = true,
                       bool needDeselectAll = true);

    Audio* audio() const { return _audio; }
    void setAudio(Audio* a) { _audio = a; }
    PlayMode playMode() const { return _playMode; }
    void setPlayMode(PlayMode v) { _playMode = v; }

    int linkId();
    void linkId(int);
    int getLinkId() const { return _linkId; }

    QList<Score*> scoreList();
    bool switchLayer(const QString& s);
    //@ appends to the score a number of measures
    void appendMeasures(int);

    const std::multimap<int, Spanner*>& spanner() const { return _spanner.map(); }
    SpannerMap& spannerMap() { return _spanner; }
    bool isSpannerStartEnd(const Fraction& tick, int track) const;
    void removeSpanner(Spanner*);
    void addSpanner(Spanner*);
    void cmdAddSpanner(Spanner* spanner, const mu::PointF& pos, bool firstStaffOnly = false);
    void cmdAddSpanner(Spanner* spanner, int staffIdx, Segment* startSegment, Segment* endSegment);
    void checkSpanner(const Fraction& startTick, const Fraction& lastTick);
    const std::set<Spanner*> unmanagedSpanners() { return _unmanagedSpanner; }
    void addUnmanagedSpanner(Spanner*);
    void removeUnmanagedSpanner(Spanner*);

    Hairpin* addHairpin(HairpinType, const Fraction& tickStart, const Fraction& tickEnd, int track);
    Hairpin* addHairpin(HairpinType, ChordRest* cr1, ChordRest* cr2 = nullptr, bool toCr2End = true);

    ChordRest* findCR(Fraction tick, int track) const;
    ChordRest* findCRinStaff(const Fraction& tick, int staffIdx) const;
    void insertTime(const Fraction& tickPos, const Fraction& tickLen);

    ScoreFont* scoreFont() const { return _scoreFont; }
    void setScoreFont(ScoreFont* f) { _scoreFont = f; }

    qreal noteHeadWidth() const { return _noteHeadWidth; }
    void setNoteHeadWidth(qreal n) { _noteHeadWidth = n; }

    QList<int> uniqueStaves() const;
    void transpositionChanged(Part*, Interval, Fraction tickStart = { 0, 1 }, Fraction tickEnd = { -1, 1 });

    void moveUp(ChordRest*);
    void moveDown(ChordRest*);
    Element* upAlt(Element*);
    Note* upAltCtrl(Note*) const;
    Element* downAlt(Element*);
    Note* downAltCtrl(Note*) const;

    Element* firstElement(bool frame = true);
    Element* lastElement(bool frame = true);

    int nmeasures() const;
    bool hasLyrics();
    bool hasHarmonies();
    int  lyricCount();
    int  harmonyCount();
    QString extractLyrics();
    int keysig();
    int duration();
    int durationWithoutRepeats();

    void cmdInsertClef(Clef* clef, ChordRest* cr);

    void cmdExplode();
    void cmdImplode();
    void cmdSlashFill();
    void cmdSlashRhythm();
    void cmdResequenceRehearsalMarks();
    void cmdExchangeVoice(int, int);
    void cmdRemoveEmptyTrailingMeasures();
    void cmdRealizeChordSymbols(bool lit = true, Voicing v = Voicing(-1), HDuration durationType = HDuration(-1));

    Measure* firstTrailingMeasure(ChordRest** cr = nullptr);
    ChordRest* cmdTopStaff(ChordRest* cr = nullptr);

    mu::engraving::AccessibleScore* accessible() const;
    void setAccessibleInfo(QString s) { accInfo = s.remove(":").remove(";"); }
    QString accessibleInfo() const { return accInfo; }

    void setAccessibleMessage(QString s) { accMessage = s; } // retain ':' and ';'
    QString accessibleMessage() const { return accMessage; }

    std::shared_ptr<mu::draw::Pixmap> createThumbnail();
    QString createRehearsalMarkText(RehearsalMark* current) const;
    QString nextRehearsalMarkText(RehearsalMark* previous, RehearsalMark* current) const;

    bool sanityCheck(const QString& name = QString());

    bool checkKeys();

    void switchToPageMode();

    virtual QVariant getProperty(Pid) const override;
    virtual bool setProperty(Pid, const QVariant&) override;
    virtual QVariant propertyDefault(Pid) const override;

    virtual QQueue<MidiInputEvent>* midiInputQueue();
    virtual std::list<MidiInputEvent>* activeMidiPitches();

    virtual QString title() const;

    void cmdTimeDelete();
    void localTimeDelete();
    void globalTimeDelete();

    Text* headerText(int index) const { return _headersText[index]; }
    Text* footerText(int index) const { return _footersText[index]; }
    void setHeaderText(Text* t, int index) { _headersText.at(index) = t; }
    void setFooterText(Text* t, int index) { _footersText.at(index) = t; }

    void cmdAddPitch(int note, bool addFlag, bool insert);
    void forAllLyrics(std::function<void(Lyrics*)> f);

    constexpr static double defaultTempo() { return _defaultTempo; }

    friend class ChangeSynthesizerState;
    friend class Chord;
};

static inline Score* toScore(ScoreElement* e)
{
    Q_ASSERT(!e || e->isScore());
    return static_cast<Score*>(e);
}

static inline const Score* toScore(const ScoreElement* e)
{
    Q_ASSERT(!e || e->isScore());
    return static_cast<const Score*>(e);
}

//---------------------------------------------------------
//   ScoreLoad
//---------------------------------------------------------

class ScoreLoad
{
    static int _loading;

public:
    ScoreLoad() { ++_loading; }
    ~ScoreLoad() { --_loading; }
    static bool loading() { return _loading > 0; }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(LayoutFlags)
}     // namespace Ms

#endif
