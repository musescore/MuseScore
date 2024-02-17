//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SCORE_H__
#define __SCORE_H__

/**
 \file
 Definition of Score class.
*/

#include "config.h"
#include "input.h"
#include "instrument.h"
#include "select.h"
#include "synthesizerstate.h"
#include "mscoreview.h"
#include "spannermap.h"
#include "layoutbreak.h"
#include "property.h"
#include "sym.h"

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
class ScoreOrder;
struct Interval;
struct TEvent;
struct LayoutContext;

enum class Tid;
enum class ClefType : signed char;
enum class BeatType : char;
enum class Key;
enum class HairpinType : signed char;
enum class SegmentType;
enum class OttavaType : char;
enum class Voicing : signed char;
enum class HDuration : signed char;

enum class POS : char { CURRENT, LEFT, RIGHT };

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
//   LayoutMode
//    PAGE   The normal page view, honors page and line breaks.
//    LINE   The panoramic view, one long system
//    FLOAT  The "reflow" mode, ignore page and line breaks
//    SYSTEM The "never ending page", page break are turned into line break
//---------------------------------------------------------

enum class LayoutMode : char {
      PAGE, FLOAT, LINE, SYSTEM
      };

//---------------------------------------------------------
//   MeasureBaseList
//---------------------------------------------------------

class MeasureBaseList {
      int _size;
      MeasureBase* _first;
      MeasureBase* _last;

      void push_back(MeasureBase* e);
      void push_front(MeasureBase* e);

   public:
      MeasureBaseList();
      MeasureBase* first() const { return _first; }
      MeasureBase* last()  const { return _last; }
      void clear()               { _first = _last = 0; _size = 0; }
      void add(MeasureBase*);
      void remove(MeasureBase*);
      void insert(MeasureBase*, MeasureBase*);
      void remove(MeasureBase*, MeasureBase*);
      void change(MeasureBase* o, MeasureBase* n);
      int size() const { return _size; }
      };

//---------------------------------------------------------
//   MidiMapping
//---------------------------------------------------------

class MidiMapping {
      Part* _part;
      std::unique_ptr<Channel> _articulation;
      signed char _port;
      signed char _channel;
      Channel* masterChannel;
      PartChannelSettingsLink link;

      MidiMapping() = default; // should be created only within MasterScore
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
      QPointF pos;
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
      uint tags;
      };

//---------------------------------------------------------
//   UpdateMode
//    There is an implied order from least invasive update
//    to most invasive update. LayoutAll is fallback and
//    recreates all.
//---------------------------------------------------------

enum class UpdateMode {
      DoNothing,
      Update,           // do screen refresh of QRectF "refresh"
      UpdateAll,        // do complete screen refresh
      Layout,           // do partial layout for tick range
      };

//---------------------------------------------------------
//   CmdState
//
//    the following variables are reset on startCmd()
//    modified during cmd processing and used in endCmd() to
//    determine what to layout and what to repaint:
//---------------------------------------------------------

class CmdState {
      UpdateMode _updateMode { UpdateMode::DoNothing };
      Fraction _startTick {-1, 1};            // start tick for mode LayoutTick
      Fraction _endTick   {-1, 1};              // end tick for mode LayoutTick
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
      bool updateAll() const   { return int(_updateMode) >= int(UpdateMode::UpdateAll); }
      bool updateRange() const { return _updateMode == UpdateMode::Update; }
      void setTick(const Fraction& t);
      void setStaff(int staff);
      void setElement(const Element* e);
      void unsetElement(const Element* e);
      Fraction startTick() const { return _startTick; }
      Fraction endTick() const   { return _endTick; }
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

class UpdateState {
   public:
      QRectF refresh;               ///< area to update, canvas coordinates
      bool _playNote   { false };   ///< play selected note after command
      bool _playChord  { false };   ///< play whole chord for the selected note
      bool _selectionChanged { false };
      QList<ScoreElement*> _deleteList;
      };

//---------------------------------------------------------
//   ScoreContentState
//---------------------------------------------------------

class ScoreContentState {
      const Score* score;
      int num;
   public:
      ScoreContentState() : score(nullptr), num(0) {}
      ScoreContentState(const Score* s, int stateNum) : score(s), num(stateNum) {}

      bool operator==(const ScoreContentState& s2) const { return score == s2.score && num == s2.num; }
      bool operator!=(const ScoreContentState& s2) const { return !(*this == s2); }

      bool isNewerThan(const ScoreContentState& s2) const { return score == s2.score && num > s2.num; }
      };

class MasterScore;

//-----------------------------------------------------------------------------
//   Movements
//    A movement is a unit of a larger work that may stand
//    by itself as a complete composition.
//    A MuseScore score file can contain several movements represented as
//    MasterScore's. A MasterScore can have several parts represented
//    as Score. MasterScores are connected in a double linked list.
//-----------------------------------------------------------------------------

class Movements : public std::vector<MasterScore*> {
      UndoStack* _undo;
      QList<Page*> _pages;          // pages are build from systems
      MStyle _style;

   public:
      Movements();
      ~Movements();
      int pageIdx(Page* page) const           { return _pages.indexOf(page); }
      int npages() const                      { return _pages.size();        }
      const QList<Page*>& pages() const       { return _pages;               }
      QList<Page*>& pages()                   { return _pages;               }
      UndoStack* undo() const                 { return _undo;                }
      MStyle& style()                         { return _style;               }
      const MStyle& style() const             { return _style;               }
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

class Score : public QObject, public ScoreElement {
      Q_OBJECT

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
      int _pageNumberOffset { 0 };        ///< Offset for page numbers.

      UpdateState _updateState;

      MeasureBaseList _measures;          // here are the notes
      QList<Part*> _parts;
      QList<Staff*> _staves;

      SpannerMap _spanner;
      std::set<Spanner*> _unmanagedSpanner;

      //
      // objects generated by layout:
      //
      QList<Page*> _pages;          // pages are build from systems
      QList<System*> _systems;      // measures are accumulated to systems

      InputState _is;
      MStyle _style;

      bool _created { false };            ///< file is never saved, has generated name
      bool _startedEmpty { false };       ///< The score was created from an empty template (typically ":/data/My_First_Score.mscx") during this session, so it doesn't need to be saved if it hasn't been modified.
      QString _tmpName;                   ///< auto saved with this name if not empty
      QString _importedFilePath;          // file from which the score was imported, or empty

      bool _showInvisible         { true  };
      bool _showUnprintable       { true  };
      bool _showFrames            { true  };
      bool _showPageborders       { false };
      bool _markIrregularMeasures { true  };
      bool _showInstrumentNames   { true  };
      bool _showVBox              { true  };
      bool _printing              { false };      ///< True if we are drawing to a printer
      bool _autosaveDirty         { true  };
      bool _savedCapture          { false };      ///< True if we saved an image capture
      bool _saved                 { false };    ///< True if project was already saved; only on first
                                                ///< save a backup file will be created, subsequent
                                                ///< saves will not overwrite the backup file.
      bool _defaultsRead        { false };      ///< defaults were read at MusicXML import, allow export of defaults in convertermode
      bool _isPalette           { false };
      ScoreOrder* _scoreOrder   { nullptr };    ///< used for score ordering

      int _mscVersion { MSCVERSION };   ///< version of current loading *.msc file

      QMap<QString, QString> _metaTags;

      constexpr static double _defaultTempo = 2.0; //default tempo is equal 120 bpm

      Selection _selection;
      SelectionFilter _selectionFilter;
      Audio* _audio { 0 };
      PlayMode _playMode { PlayMode::SYNTHESIZER };

      qreal _noteHeadWidth { 0.0 };       // cached value
      QString accInfo;                    ///< information about selected element(s) for use by screen-readers
      QString accMessage;                 ///< temporary status message for use by screen-readers

      //------------------

      ChordRest* nextMeasure(ChordRest* element, bool selectBehavior = false, bool mmRest = false);
      ChordRest* prevMeasure(ChordRest* element, bool mmRest = false);
      void cmdSetBeamMode(Beam::Mode);
      void cmdResetAllStyle();
      void cmdResetTextStyleOverrides();
      void cmdFlip();
      Note* getSelectedNote();
      ChordRest* upStaff(ChordRest* cr);
      ChordRest* downStaff(ChordRest* cr);
      ChordRest* nextTrack(ChordRest* cr);
      ChordRest* prevTrack(ChordRest* cr);

      void padToggle(Pad p, const EditData& ed);
      void addTempo();
      void addMetronome();

      void cmdResetBeamMode();

      void cmdInsertClef(ClefType);
      void cmdAddGrace(NoteType, int);
      void removeChordRest(ChordRest* cr, bool clearSegment);
      void cmdMoveRest(Rest*, Direction);
      void cmdMoveLyrics(Lyrics*, Direction);
      void cmdIncDecDuration(int nSteps, bool stepDotted = false);
      void cmdAddBracket();
      void cmdAddParentheses();
      void cmdAddBraces();
      void resetUserStretch();

      void createMMRest(Measure*, Measure*, const Fraction&);

      void beamGraceNotes(Chord*, bool);

      void checkSlurs();
      void checkScore();

      bool rewriteMeasures(Measure* fm, Measure* lm, const Fraction&, int staffIdx);
      bool rewriteMeasures(Measure* fm, const Fraction& ns, int staffIdx);
      void swingAdjustParams(Chord*, int&, int&, int, int);
      bool isSubdivided(ChordRest*, int);
      void addAudioTrack();
      QList<Fraction> splitGapToMeasureBoundaries(ChordRest*, Fraction);
      void pasteChordRest(ChordRest* cr, const Fraction& tick, const Interval&);

      void selectSingle(Element* e, int staffIdx);
      void selectAdd(Element* e);
      void selectRange(Element* e, int staffIdx);

      void cmdAddPitch(const EditData&, int note, bool addFlag, bool insert);
      void cmdAddFret(int fret);
      void cmdToggleVisible();

      void putNote(const Position&, bool replace);

      void resetSystems(bool layoutAll, LayoutContext& lc);
      void collectLinearSystem(LayoutContext& lc);
      void resetTempo();
      void resetTempoRange(const Fraction& tick1, const Fraction& tick2);

      void deleteSpannersFromRange(const Fraction& t1, const Fraction& t2, int trackStart, int trackEnd, const SelectionFilter& filter);
      void deleteAnnotationsFromRange(Segment* segStart, Segment* segEnd, int trackStart, int trackEnd, const SelectionFilter& filter);
      std::vector<ChordRest*> deleteRange(Segment* segStart, Segment* segEnd, int trackStart, int trackEnd,
                                          const SelectionFilter& filter);

      void update(bool resetCmdState);

   protected:
      int _fileDivision; ///< division of current loading *.msc file
      LayoutMode _layoutMode { LayoutMode::PAGE };
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
      inline virtual Movements* movements();
      inline virtual const Movements* movements() const;

   signals:
      void posChanged(POS, unsigned);
      void playlistChanged();

   public:
      Score();
      Score(MasterScore*, bool forcePartStyle = true);
      Score(MasterScore*, const MStyle&);
      Score(const Score&) = delete;
      Score& operator=(const Score&) = delete;
      virtual ~Score();
      Score* clone();

      virtual bool isMaster() const  { return false;        }
      virtual bool readOnly() const;

      static void onElementDestruction(Element* se);

      virtual inline QList<Excerpt*>& excerpts();
      virtual inline const QList<Excerpt*>& excerpts() const;

      virtual ElementType type() const override { return ElementType::SCORE; }

      void rebuildBspTree();
      bool noStaves() const         { return _staves.empty(); }
      void insertPart(Part*, int);
      void removePart(Part*);
      void insertStaff(Staff*, int);
      void cmdRemoveStaff(int staffIdx);
      void removeStaff(Staff*);
      void addMeasure(MeasureBase*, MeasureBase*);
      void readStaff(XmlReader&);
      bool read(XmlReader&);
      void linkMeasures(Score* score);

      Excerpt* excerpt()            { return _excerpt; }
      void setExcerpt(Excerpt* e)   { _excerpt = e;     }

      System* collectSystem(LayoutContext&);
      void layoutSystemElements(System* system, LayoutContext& lc);
      void getNextMeasure(LayoutContext&);      // get next measure for layout

      void resetAllPositions();

      void cmdRemovePart(Part*);
      void cmdAddTie(bool addToChord = false);
      void cmdToggleTie();
      static std::vector<Note*> cmdTieNoteList(const Selection& selection, bool noteEntryMode);
      void cmdAddOttava(OttavaType);
      void cmdAddStretch(qreal);
      void cmdResetNoteAndRestGroupings();
      void cmdResetAllPositions(bool undoable = true);
      void cmdDoubleDuration()      { cmdIncDecDuration(-1, false); }
      void cmdHalfDuration()        { cmdIncDecDuration( 1, false); }
      void cmdIncDurationDotted()   { cmdIncDecDuration(-1, true); }
      void cmdDecDurationDotted()   { cmdIncDecDuration( 1, true); }
      void cmdToggleLayoutBreak(LayoutBreak::Type);

      void addRemoveBreaks(int interval, bool lock);

      bool transpose(Note* n, Interval, bool useSharpsFlats);
      void transposeKeys(int staffStart, int staffEnd, const Fraction& tickStart, const Fraction& tickEnd, const Interval&, bool useInstrument = false, bool flip = false);
      bool transpose(TransposeMode mode, TransposeDirection, Key transposeKey, int transposeInterval,
      bool trKeys, bool transposeChordNames, bool useDoubleSharpsFlats);

      bool appendMeasuresFromScore(Score* score, const Fraction& startTick, const Fraction& endTick);
      bool appendScore(Score*, bool addPageBreak = false, bool addSectionBreak = true);

      void write(XmlWriter&, bool onlySelection);
      void writeMovement(XmlWriter&, bool onlySelection);

      QList<Staff*>& staves()                { return _staves; }
      const QList<Staff*>& staves() const    { return _staves; }
      int nstaves() const                    { return _staves.size(); }
      int ntracks() const                    { return _staves.size() * VOICES; }

      int staffIdx(const Part*) const;
      Staff* staff(int n) const              { return ((n >= 0) && (n < _staves.size())) ? _staves.at(n) : nullptr; }

      Measure* pos2measure(const QPointF&, int* staffIdx, int* pitch, Segment**, QPointF* offset) const;
      void dragPosition(const QPointF&, int* staffIdx, Segment**, qreal spacingFactor = 0.5) const;

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
      void undoRemovePart(Part* part, int idx);
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
      inline virtual UndoStack* undoStack() const;
      void undo(UndoCommand*, EditData* = 0) const;
      void undoRemoveMeasures(Measure*, Measure*, bool preserveTies = false);
      void undoAddBracket(Staff* staff, int level, BracketType type, int span);
      void undoRemoveBracket(Bracket*);
      void undoInsertTime(const Fraction& tick, const Fraction& len);
      void undoChangeStyleVal(Sid idx, const QVariant& v);
      void undoChangePageNumberOffset(int po);

      void updateInstrumentChangeTranspositions(Ms::KeySigEvent& key, Ms::Staff* staff, const Ms::Fraction& tick);

      Note* setGraceNote(Chord*,  int pitch, NoteType type, int len);

      Segment* setNoteRest(Segment*, int track, NoteVal nval, Fraction, Direction stemDirection = Direction::AUTO, bool forceAccidental = false, bool rhythmic = false, InputState* externalInputState = nullptr);
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

      ChordRest* addClone(ChordRest* cr, const Fraction& tick, const TDuration& d);
      Rest* setRest(const Fraction& tick, int track, const Fraction&, bool useDots, Tuplet* tuplet, bool useFullMeasureRest = true);
      std::vector<Rest*> setRests(const Fraction& tick, int track, const Fraction&, bool useDots, Tuplet* tuplet,
                                  bool useFullMeasureRest = true);

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
      Note* addNote(Chord*, const NoteVal& noteVal, bool forceAccidental = false, InputState* externalInputState = nullptr);

      NoteVal noteValForPosition(Position pos, AccidentalType at, bool &error);

      void deleteItem(Element*);
      void deleteMeasures(MeasureBase* firstMeasure, MeasureBase* lastMeasure, bool preserveTies = false);
      void cmdDeleteSelection();
      void cmdFullMeasureRest();

      void putNote(const QPointF&, bool replace, bool insert);
      void insertChord(const Position&);
      void localInsertChord(const Position&);
      void globalInsertChord(const Position&);

      void cloneVoice(int strack, int dtrack, Segment* sf, const Fraction& lTick, bool link = true, bool spanner = true);

      void repitchNote(const Position& pos, bool replace);
      void regroupNotesAndRests(const Fraction&  startTick, const Fraction& endTick, int track);
      bool checkTimeDelete(Segment*, Segment*);
      void timeDelete(Measure*, Segment*, const Fraction&);

      void startCmd();                          // start undoable command
      void endCmd(bool rollback = false);       // end undoable command
      void update() { update(true); }
      void undoRedo(bool undo, EditData*);

      void cmdRemoveTimeSig(TimeSig*);
      void cmdAddTimeSig(Measure*, int staffIdx, TimeSig*, bool local);

      virtual inline void setUpdateAll();
      inline void setLayoutAll(int staff = -1, const Element* e = nullptr);
      inline void setLayout(const Fraction& tick, int staff, const Element* e = nullptr);
      inline void setLayout(const Fraction& tick1, const Fraction& tick2, int staff1, int staff2, const Element* e = nullptr);
      virtual inline CmdState& cmdState();
      virtual inline const CmdState& cmdState() const;
      virtual inline void addLayoutFlags(LayoutFlags);
      virtual inline void setInstrumentsChanged(bool);
      void addRefresh(const QRectF&);

      void cmdRelayout();
      void cmdToggleAutoplace(bool all);
      void cmdApplyInputState();

      bool playNote() const                 { return _updateState._playNote; }
      void setPlayNote(bool v)              { _updateState._playNote = v;    }
      bool playChord() const                { return _updateState._playChord; }
      void setPlayChord(bool v)             { _updateState._playChord = v;    }
      bool selectionChanged() const         { return _updateState._selectionChanged; }
      void setSelectionChanged(bool val)    { _updateState._selectionChanged = val;  }
      void deleteLater(ScoreElement* e)     { _updateState._deleteList.push_back(e); }
      void deletePostponed();

      void changeVoice(int);

      void colorItem(Element*);
      QList<Part*>& parts()                { return _parts; }
      const QList<Part*>& parts() const    { return _parts; }

      void appendPart(Part* p);
      void appendPart(const InstrumentTemplate*);
      void updateStaffIndex();
      void sortStaves(QList<int>& dst);
      void mapExcerptTracks(QList<int>& l);

      bool showInvisible() const       { return _showInvisible; }
      bool showUnprintable() const     { return _showUnprintable; }
      bool showFrames() const          { return _showFrames; }
      bool showPageborders() const     { return _showPageborders; }
      bool markIrregularMeasures() const { return _markIrregularMeasures; }
      bool showInstrumentNames() const { return _showInstrumentNames; }
      bool showVBox() const            { return _showVBox; }
      void setShowInvisible(bool v);
      void setShowUnprintable(bool v);
      void setShowFrames(bool v);
      void setShowPageborders(bool v);
      void setMarkIrregularMeasures(bool v);
      void setShowInstrumentNames(bool v) { _showInstrumentNames = v; }
      void setShowVBox(bool v)            { _showVBox = v;            }

      bool saveFile(QFileInfo& info);
      bool saveFile(QIODevice* f, bool msczFormat, bool onlySelection = false);
      bool saveCompressedFile(QFileInfo&, bool onlySelection, bool createThumbnail = true);
      bool saveCompressedFile(QIODevice*, const QString& fileName, bool onlySelection, bool createThumbnail = true);

      void print(QPainter* printer, int page);
      ChordRest* getSelectedChordRest() const;
      QSet<ChordRest*> getSelectedChordRests() const;
      void getSelectedChordRest2(ChordRest** cr1, ChordRest** cr2) const;

      void select(Element* obj, SelectType = SelectType::SINGLE, int staff = 0);
      void selectSimilar(Element* e, bool sameStaff);
      void selectSimilarInRange(Element* e);
      static void collectMatch(void* data, Element* e);
      static void collectNoteMatch(void* data, Element* e);
      void deselect(Element* obj);
      void deselectAll()                    { _selection.deselectAll(); }
      void updateSelection()                { _selection.update(); }
      Element* getSelectedElement() const   { return _selection.element(); }
      const Selection& selection() const    { return _selection; }
      Selection& selection()                { return _selection; }
      SelectionFilter& selectionFilter()     { return _selectionFilter; }
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

      void cmd(const QAction*, EditData&);
      int fileDivision(int t) const { return ((qint64)t * MScore::division + _fileDivision/2) / _fileDivision; }
      void setFileDivision(int t) { _fileDivision = t; }

      QString importedFilePath() const           { return _importedFilePath; }
      void setImportedFilePath(const QString& filePath);

      bool dirty() const;
      ScoreContentState state() const;
      void setCreated(bool val)      { _created = val;        }
      bool created() const           { return _created;       }
      void setStartedEmpty(bool val) { _startedEmpty = val;   }
      bool startedEmpty() const      { return _startedEmpty;  }
      bool savedCapture() const      { return _savedCapture;  }
      bool saved() const             { return _saved;         }
      void setSaved(bool v)          { _saved = v;            }
      void setSavedCapture(bool v)   { _savedCapture = v;     }
      bool printing() const          { return _printing;      }
      void setPrinting(bool val)     { _printing = val;      }
      void setAutosaveDirty(bool v)  { _autosaveDirty = v;    }
      bool autosaveDirty() const     { return _autosaveDirty; }
      virtual bool playlistDirty() const;
      virtual void setPlaylistDirty();

      void spell();
      void spell(int startStaff, int endStaff, Segment* startSegment, Segment* endSegment);
      void spell(Note*);
      Fraction nextSeg(const Fraction& tick, int track);

      virtual MStyle& style()              { return _style;                  }
      virtual const MStyle& style() const  { return _style;                  }

      void setStyle(const MStyle& s, const bool overlap = false);
      bool loadStyle(const QString&, bool ign = false, const bool overlap = false);
      bool saveStyle(const QString&);

      QVariant styleV(Sid idx) const  { return style().value(idx);   }
      Spatium  styleS(Sid idx) const  { Q_ASSERT(!strcmp(MStyle::valueType(idx),"Ms::Spatium")); return style().value(idx).value<Spatium>();  }
      qreal    styleP(Sid idx) const  { Q_ASSERT(!strcmp(MStyle::valueType(idx),"Ms::Spatium")); return style().pvalue(idx); }
      QString  styleSt(Sid idx) const { Q_ASSERT(!strcmp(MStyle::valueType(idx),"QString"));     return style().value(idx).toString(); }
      bool     styleB(Sid idx) const  { Q_ASSERT(!strcmp(MStyle::valueType(idx),"bool"));        return style().value(idx).toBool();  }
      qreal    styleD(Sid idx) const  { Q_ASSERT(!strcmp(MStyle::valueType(idx),"double"));      return style().value(idx).toDouble();  }
      int      styleI(Sid idx) const  { Q_ASSERT(!strcmp(MStyle::valueType(idx),"int"));         return style().value(idx).toInt();  }

      void setStyleValue(Sid sid, QVariant value) { style().set(sid, value);     }
      QString getTextStyleUserName(Tid tid);
      qreal spatium() const                    { return styleD(Sid::spatium);    }
      void setSpatium(qreal v)                 { setStyleValue(Sid::spatium, v); }

      bool genCourtesyTimesig() const          { return styleB(Sid::genCourtesyTimesig); }
      bool genCourtesyClef() const             { return styleB(Sid::genCourtesyClef); }

      // These position are in ticks and not uticks
      Fraction playPos() const                      { return pos(POS::CURRENT);   }
      void setPlayPos(const Fraction& tick)         { setPos(POS::CURRENT, tick); }
      Fraction loopInTick() const                   { return pos(POS::LEFT);      }
      Fraction loopOutTick() const                  { return pos(POS::RIGHT);     }
      void setLoopInTick(const Fraction& tick)      { setPos(POS::LEFT, tick);    }
      void setLoopOutTick(const Fraction& tick)     { setPos(POS::RIGHT, tick);   }

      inline Fraction pos(POS pos) const;
      inline void setPos(POS pos, Fraction tick);

      bool noteEntryMode() const                   { return inputState().noteEntryMode(); }
      void setNoteEntryMode(bool val)              { inputState().setNoteEntryMode(val); }
      NoteEntryMethod noteEntryMethod() const      { return inputState().noteEntryMethod();        }
      void setNoteEntryMethod(NoteEntryMethod m)   { inputState().setNoteEntryMethod(m);           }
      bool usingNoteEntryMethod(NoteEntryMethod m) { return inputState().usingNoteEntryMethod(m);  }
      Fraction inputPos() const;
      int inputTrack() const                   { return inputState().track(); }
      const InputState& inputState() const     { return _is;                  }
      InputState& inputState()                 { return _is;                  }
      void setInputState(const InputState& st) { _is = st;                    }
      void setInputTrack(int t)                { inputState().setTrack(t);    }

      void spatiumChanged(qreal oldValue, qreal newValue);
      void styleChanged();

      void cmdPaste(const QMimeData* ms, MuseScoreView* view, Fraction scale = Fraction(1, 1));
      bool pasteStaff(XmlReader&, Segment* dst, int staffIdx, Fraction scale = Fraction(1, 1));
      void readAddConnector(ConnectorInfoReader* info, bool pasteMode) override;
      void pasteSymbols(XmlReader& e, ChordRest* dst);
      void renderMidi(EventMap* events, const SynthesizerState& synthState);
      void renderMidi(EventMap* events, bool metronome, bool expandRepeats, const SynthesizerState& synthState);

      BeatType tick2beatType(const Fraction& tick);

      int mscVersion() const    { return _mscVersion; }
      void setMscVersion(int v) { _mscVersion = v; }

      void addLyrics(const Fraction& tick, int staffIdx, const QString&);

      void updateSwing();
      void createPlayEvents(Measure const * start = nullptr, Measure const * const end = nullptr);

      void updateCapo();
      void updateVelo();
      void updateChannel();

      void cmdConcertPitchChanged(bool, bool /*useSharpsFlats*/);

      virtual inline TempoMap* tempomap() const;
      virtual inline TimeSigMap* sigmap() const;

      void setTempo(Segment*, qreal);
      void setTempo(const Fraction& tick, qreal bps);
      void removeTempo(const Fraction& tick);
      void setPause(const Fraction& tick, qreal seconds);
      qreal tempo(const Fraction& tick) const;

      bool defaultsRead() const                      { return _defaultsRead;    }
      void setDefaultsRead(bool b)                   { _defaultsRead = b;       }
      Text* getText(Tid subtype);

      bool isPalette() const { return _isPalette; }
      void setPaletteMode(bool palette) { _isPalette = palette; }

      bool enableVerticalSpread() const;
      void setEnableVerticalSpread(bool val);
      qreal maxSystemDistance() const;
      ScoreOrder* scoreOrder() const        { return _scoreOrder;  }
      void setScoreOrder(ScoreOrder* order) { _scoreOrder = order; }

      void lassoSelect(const QRectF&);
      void lassoSelectEnd();

      Page* searchPage(const QPointF&) const;
      QList<System*> searchSystem(const QPointF& p, const System* preferredSystem = nullptr, qreal spacingFactor = 0.5, qreal preferredSpacingFactor = 1.0) const;
      Measure* searchMeasure(const QPointF& p, const System* preferredSystem = nullptr, qreal spacingFactor = 0.5, qreal preferredSpacingFactor = 1.0) const;

      bool getPosition(Position* pos, const QPointF&, int voice) const;

      void cmdDeleteTuplet(Tuplet*, bool replaceWithRest);

//      void moveBracket(int staffIdx, int srcCol, int dstCol);
      Measure* getCreateMeasure(const Fraction& tick);

      void adjustBracketsDel(int sidx, int eidx);
      void adjustBracketsIns(int sidx, int eidx);
      void adjustKeySigs(int sidx, int eidx, KeyList km);

      virtual inline const RepeatList& repeatList() const;
      virtual inline const RepeatList& repeatList2() const;
      qreal utick2utime(int tick) const;
      int utime2utick(qreal utime) const;

      void nextInputPos(ChordRest* cr, bool);
      void cmdMirrorNoteHead();

      qreal loWidth() const;
      qreal loHeight() const;

      virtual int npages() const                { return _pages.size(); }
      virtual int pageIdx(Page* page) const     { return _pages.indexOf(page); }
      virtual const QList<Page*>& pages() const { return _pages;                }
      virtual QList<Page*>& pages()             { return _pages;                }

      const QList<System*>& systems() const    { return _systems;              }
      QList<System*>& systems()                { return _systems;              }

      MeasureBaseList* measures()             { return &_measures; }
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

      void connectTies(bool silent=false);
      void connectArpeggios();
      void fixupLaissezVibrer();

      qreal point(const Spatium sp) const { return sp.val() * spatium(); }

      void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      void scanElementsInRange(void* data, void (*func)(void*, Element*), bool all = true);
      int fileDivision() const { return _fileDivision; } ///< division of current loading *.msc file
      void splitStaff(int staffIdx, int splitPoint);
      QString tmpName() const           { return _tmpName;      }
      void setTmpName(const QString& s) { _tmpName = s;         }
      bool processMidiInput();
      Lyrics* addLyrics();
      FiguredBass* addFiguredBass();
      void expandVoice(Segment* s, int track);
      void expandVoice();

      Element* selectMove(const QString& cmd);
      Element* move(const QString& cmd);
      void cmdEnterRest(const TDuration& d);
      void enterRest(const TDuration& d, InputState* externalInputState = nullptr);
      void cmdAddInterval(int, const std::vector<Note*>&);
      void cmdCreateTuplet(ChordRest*, Tuplet*);
      void removeAudio();

      void doLayout();
      void doLayoutRange(const Fraction&, const Fraction&);
      void layoutLinear(bool layoutAll, LayoutContext& lc);

      void layoutChords1(Segment* segment, int staffIdx);
      qreal layoutChords2(std::vector<Note*>& notes, bool up);
      void layoutChords3(std::vector<Note*>&, const Staff*, Segment*);

      SynthesizerState& synthesizerState()     { return _synthesizerState; }
      void setSynthesizerState(const SynthesizerState& s);

      void updateHairpin(Hairpin*);       // add/modify hairpin to pitchOffset list

      MasterScore* masterScore() const    { return _masterScore; }
      void setMasterScore(MasterScore* s) { _masterScore = s;    }
      void createRevision();
      void writeSegments(XmlWriter& xml, int strack, int etrack, Segment* sseg, Segment* eseg, bool, bool);

      const QMap<QString, QString>& metaTags() const   { return _metaTags; }
      QMap<QString, QString>& metaTags()               { return _metaTags; }
      void setMetaTags(const QMap<QString,QString>& t) { _metaTags = t; }

      //@ returns as a string the metatag named 'tag'
      QString metaTag(const QString& tag) const;
      //@ sets the metatag named 'tag' to 'val'
      void setMetaTag(const QString& tag, const QString& val);

      void cmdSplitMeasure(ChordRest*);
      void splitMeasure(Segment*);
      void cmdJoinMeasure(Measure*, Measure*);
      int pageNumberOffset() const          { return _pageNumberOffset; }
      void setPageNumberOffset(int v)       { _pageNumberOffset = v; }

      QString mscoreVersion() const         { return _mscoreVersion; }
      int mscoreRevision() const            { return _mscoreRevision; }
      void setMscoreVersion(const QString& val) { _mscoreVersion = val; }
      void setMscoreRevision(int val)           { _mscoreRevision = val; }

      uint currentLayerMask() const         { return _layer[_currentLayer].tags; }
      void setCurrentLayer(int val)         { _currentLayer = val;  }
      int currentLayer() const              { return _currentLayer; }
      QString* layerTags()                  { return _layerTags;    }
      QString* layerTagComments()           { return _layerTagComments;    }
      QList<Layer>& layer()                 { return _layer;       }
      const QList<Layer>& layer() const     { return _layer;       }
      bool tagIsValid(uint tag) const       { return tag & _layer[_currentLayer].tags; }

      void addViewer(MuseScoreView* v)      { viewer.append(v);    }
      void removeViewer(MuseScoreView* v)   { viewer.removeAll(v); }
      const QList<MuseScoreView*>& getViewer() const { return viewer;       }

      LayoutMode layoutMode() const         { return _layoutMode; }
      void setLayoutMode(LayoutMode lm)     { _layoutMode = lm;   }

      bool floatMode() const                { return layoutMode() == LayoutMode::FLOAT; }
      bool pageMode() const                 { return layoutMode() == LayoutMode::PAGE; }
      bool lineMode() const                 { return layoutMode() == LayoutMode::LINE; }
      bool systemMode() const               { return layoutMode() == LayoutMode::SYSTEM; }

      Tuplet* searchTuplet(XmlReader& e, int id);
      void cmdSelectAll();
      void cmdSelectSection();
      void respace(std::vector<ChordRest*>* elements);
      void transposeSemitone(int semitone);
      void transposeDiatonicAlterations(TransposeDirection direction);
      void insertMeasure(ElementType type, MeasureBase*, bool createEmptyMeasures = false, bool moveSignaturesClef = true);
      Audio* audio() const         { return _audio;    }
      void setAudio(Audio* a)      { _audio = a;       }
      PlayMode playMode() const    { return _playMode; }
      void setPlayMode(PlayMode v) { _playMode = v;    }

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
      void cmdAddSpanner(Spanner* spanner, const QPointF& pos, bool firstStaffOnly = false);
      void cmdAddSpanner(Spanner* spanner, int staffIdx, Segment* startSegment, Segment* endSegment);
      void checkSpanner(const Fraction& startTick, const Fraction& lastTick);
      const std::set<Spanner*> unmanagedSpanners() { return _unmanagedSpanner; }
      void addUnmanagedSpanner(Spanner*);
      void removeUnmanagedSpanner(Spanner*);

      Hairpin* addHairpin(HairpinType, const Fraction& tickStart, const Fraction& tickEnd, int track);
      Hairpin* addHairpin(HairpinType, ChordRest* cr1, ChordRest* cr2 = nullptr, bool toCr2End = true);

      ChordRest* findCR(Fraction tick, int track) const;
      ChordRest* findCRinStaff(const Fraction& tick, int staffIdx) const;
      void insertTime(const Fraction& tickPos, const Fraction&tickLen);

      ScoreFont* scoreFont() const            { return _scoreFont;     }
      void setScoreFont(ScoreFont* f)         { _scoreFont = f;        }

      qreal noteHeadWidth() const     { return _noteHeadWidth; }
      void setNoteHeadWidth( qreal n) { _noteHeadWidth = n; }

      QList<int> uniqueStaves() const;
      void transpositionChanged(Part*, Interval, Fraction tickStart = { 0, 1 }, Fraction tickEnd = { -1, 1 } );

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

      void setAccessibleInfo(QString s)   { accInfo = s.remove(":").remove(";"); }
      QString accessibleInfo() const      { return accInfo;          }

      void setAccessibleMessage(QString s) { accMessage = s; } // retain ':' and ';'
      QString accessibleMessage() const    { return accMessage; }

      QImage createThumbnail();
      QString createRehearsalMarkText(RehearsalMark* current) const;
      QString nextRehearsalMarkText(RehearsalMark* previous, RehearsalMark* current) const;

      //@ ??
//      Q_INVOKABLE void cropPage(qreal margins);
      bool sanityCheck(const QString& name = QString());

      bool checkKeys();
      bool checkClefs();

      void switchToPageMode();

      virtual QVariant getProperty(Pid) const override;
      virtual bool setProperty(Pid, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;

      virtual inline QQueue<MidiInputEvent>* midiInputQueue();
      virtual inline std::list<MidiInputEvent>* activeMidiPitches();

      virtual QString title() const;

      void cmdTimeDelete();
      void localTimeDelete();
      void globalTimeDelete();

      bool isTopScore() const;

      Text* headerText(int index) const               { return _headersText[index];     }
      Text* footerText(int index) const               { return _footersText[index];     }
      void setHeaderText(Text* t, int index)          { _headersText.at(index) = t;     }
      void setFooterText(Text* t, int index)          { _footersText.at(index) = t;     }

      void cmdAddPitch(int note, bool addFlag, bool insert);
      void forAllLyrics(std::function<void(Lyrics*)> f);

      System* getNextSystem(LayoutContext&);
      void hideEmptyStaves(System* system, bool isFirstSystem);
      void layoutLyrics(System*);
      void createBeams(LayoutContext&, Measure*);

      constexpr static double defaultTempo()  { return _defaultTempo; }

      friend class ChangeSynthesizerState;
      friend class Chord;
      };

static inline Score* toScore(ScoreElement* e) {
      Q_ASSERT(!e || e->isScore());
      return static_cast<Score*>(e);
      }
static inline const Score* toScore(const ScoreElement* e) {
      Q_ASSERT(!e || e->isScore());
      return static_cast<const Score*>(e);
      }

//---------------------------------------------------------
//   MasterScore
//---------------------------------------------------------

class MasterScore : public Score {
      Q_OBJECT
      TimeSigMap* _sigmap;
      TempoMap* _tempomap;
      RepeatList* _repeatList;
      RepeatList* _repeatList2;
      bool _expandRepeats     { MScore::playRepeats };
      bool _playlistDirty     { true };
      QList<Excerpt*> _excerpts;
      std::vector<PartChannelSettingsLink> _playbackSettingsLinks;
      Score* _playbackScore = nullptr;
      Revisions* _revisions;
      MasterScore* _next      { 0 };
      MasterScore* _prev      { 0 };
      Movements* _movements   { 0 };

      bool _readOnly          { false };

      CmdState _cmdState;     // modified during cmd processing

      Omr* _omr               { 0 };
      bool _showOmr           { false };

      std::shared_ptr<Avs::AvsOmr> _avsOmr { nullptr };

      Fraction _pos[3];                    ///< 0 - current, 1 - left loop, 2 - right loop

      int _midiPortCount      { 0 };                  // A count of JACK/ALSA midi out ports
      QQueue<MidiInputEvent> _midiInputQueue;         // MIDI events that have yet to be processed
      std::list<MidiInputEvent> _activeMidiPitches;   // MIDI keys currently being held down
      std::vector<MidiMapping> _midiMapping;
      bool isSimpleMidiMaping;                        // midi mapping is simple if all ports and channels
                                                      // don't decrease and don't have gaps
      QSet<int> occupiedMidiChannels;                 // each entry is port*16+channel, port range: 0-inf, channel: 0-15
      unsigned int searchMidiMappingFrom;             // makes getting next free MIDI mapping faster

      void parseVersion(const QString&);
      void reorderMidiMapping();
      void rebuildExcerptsMidiMapping();
      void removeDeletedMidiMapping();
      int updateMidiMapping();

      QFileInfo _sessionStartBackupInfo;
      QFileInfo info;

      bool read(XmlReader&);
      void setPrev(MasterScore* s) { _prev = s; }
      void setNext(MasterScore* s) { _next = s; }

   public:
      MasterScore();
      MasterScore(const MStyle&);
      virtual ~MasterScore();
      MasterScore* clone();

      virtual bool isMaster() const override                          { return true;        }
      virtual bool readOnly() const override                          { return _readOnly;   }
      void setReadOnly(bool ro)                                       { _readOnly = ro;     }
      virtual UndoStack* undoStack() const override                   { return _movements->undo(); }
      virtual TimeSigMap* sigmap() const override                     { return _sigmap;     }
      virtual TempoMap* tempomap() const override                     { return _tempomap;   }

      virtual bool playlistDirty() const override                     { return _playlistDirty; }
      virtual void setPlaylistDirty() override;
      void setPlaylistClean()                                         { _playlistDirty = false; }

      void setExpandRepeats(bool expandRepeats);
      bool expandRepeats() const { return _expandRepeats; }
      void updateRepeatListTempo();
      virtual const RepeatList& repeatList() const override;
      virtual const RepeatList& repeatList2() const override;

      virtual QList<Excerpt*>& excerpts() override                    { return _excerpts;   }
      virtual const QList<Excerpt*>& excerpts() const override        { return _excerpts;   }
      virtual QQueue<MidiInputEvent>* midiInputQueue() override       { return &_midiInputQueue;    }
      virtual std::list<MidiInputEvent>* activeMidiPitches() override { return &_activeMidiPitches; }

      MasterScore* next() const                                       { return _next;      }
      MasterScore* prev() const                                       { return _prev;      }
      virtual Movements* movements() override                         { return _movements; }
      virtual const Movements* movements() const override             { return _movements; }
      void setMovements(Movements* m);
      void addMovement(MasterScore* score);

      virtual void setUpdateAll() override;

      void setLayoutAll(int staff = -1, const Element* e = nullptr);
      void setLayout(const Fraction& tick, int staff, const Element* e = nullptr);
      void setLayout(const Fraction& tick1, const Fraction& tick2, int staff1, int staff2, const Element* e = nullptr);

      virtual CmdState& cmdState() override                           { return _cmdState;                     }
      const CmdState& cmdState() const override                       { return _cmdState;                     }
      virtual void addLayoutFlags(LayoutFlags val) override           { _cmdState.layoutFlags |= val;         }
      virtual void setInstrumentsChanged(bool val) override           { _cmdState._instrumentsChanged = val;  }

      void setExcerptsChanged(bool val)                               { _cmdState._excerptsChanged = val;     }
      bool excerptsChanged() const                                    { return _cmdState._excerptsChanged;    }
      bool instrumentsChanged() const                                 { return _cmdState._instrumentsChanged; }

      Revisions* revisions()                                          { return _revisions;                    }

      bool isSavable() const;
      void setTempomap(TempoMap* tm);

      bool saveFile(bool generateBackup = true);
      FileError read1(XmlReader&, bool ignoreVersionError);
      FileError loadCompressedMsc(QIODevice*, bool ignoreVersionError);
      FileError loadMsc(QString name, bool ignoreVersionError);
      FileError loadMsc(QString name, QIODevice*, bool ignoreVersionError);
      FileError read114(XmlReader&);
      FileError read206(XmlReader&);
      FileError read302(XmlReader&);
      QByteArray readToBuffer();
      QByteArray readCompressedToBuffer();
      int readStyleDefaultsVersion();
      int styleDefaultByMscVersion(const int mscVer) const;

      Omr* omr() const                         { return _omr;     }
      void setOmr(Omr* o)                      { _omr = o;        }
      void removeOmr();
      bool showOmr() const                     { return _showOmr; }
      void setShowOmr(bool v)                  { _showOmr = v;    }

      std::shared_ptr<Avs::AvsOmr> avsOmr() const      { return _avsOmr; }
      void setAvsOmr(std::shared_ptr<Avs::AvsOmr> omr) { _avsOmr = omr;  }

      int midiPortCount() const                { return _midiPortCount;            }
      void setMidiPortCount(int val)           { _midiPortCount = val;             }
      std::vector<MidiMapping>& midiMapping()  { return _midiMapping;         }
      MidiMapping* midiMapping(int channel)    { return &_midiMapping[channel];    }
      void addMidiMapping(Channel* channel, Part* part, int midiPort, int midiChannel);
      void updateMidiMapping(Channel* channel, Part* part, int midiPort, int midiChannel);
      int midiPort(int idx) const              { return _midiMapping[idx].port();    }
      int midiChannel(int idx) const           { return _midiMapping[idx].channel(); }
      void rebuildMidiMapping();
      void checkMidiMapping();
      bool exportMidiMapping()                 { return !isSimpleMidiMaping; }
      int getNextFreeMidiMapping(int p = -1, int ch = -1);
      int getNextFreeDrumMidiMapping();
      void enqueueMidiEvent(MidiInputEvent ev) { _midiInputQueue.enqueue(ev); }
      void rebuildAndUpdateExpressive(Synthesizer* synth);
      void updateExpressive(Synthesizer* synth);
      void updateExpressive(Synthesizer* synth, bool expressive, bool force = false);
      void setSoloMute();

      using Score::pos;
      Fraction pos(POS pos) const { return _pos[int(pos)]; }
      void setPos(POS pos, Fraction tick);

      void addExcerpt(Excerpt*);
      void removeExcerpt(Excerpt*);
      void deleteExcerpt(Excerpt*);

      void setPlaybackScore(Score*);
      Score* playbackScore() { return _playbackScore; }
      const Score* playbackScore() const { return _playbackScore; }
      Channel* playbackChannel(const Channel* c)             { return _midiMapping[c->channel()].articulation(); }
      const Channel* playbackChannel(const Channel* c) const { return _midiMapping[c->channel()].articulation(); }

      MasterScore * unrollRepeats();

      QFileInfo* fileInfo()               { return &info; }
      const QFileInfo* fileInfo() const   { return &info; }
      void setName(const QString&);

      const QFileInfo& sessionStartBackupInfo() const { return _sessionStartBackupInfo; }

      virtual QString title() const override;

      virtual int pageIdx(Page* page) const override     { return movements()->pageIdx(page); }
      virtual const QList<Page*>& pages() const override { return movements()->pages();       }
      virtual QList<Page*>& pages() override             { return movements()->pages();       }
      virtual int npages() const override                { return movements()->npages();      }

      virtual MStyle& style() override                   { return movements()->style();       }
      virtual const MStyle& style() const override       { return movements()->style();       }
      };

//---------------------------------------------------------
//   ScoreLoad
//---------------------------------------------------------

class ScoreLoad {
      static int _loading;

   public:
      ScoreLoad()  { ++_loading;  }
      ~ScoreLoad() { --_loading; }
      static bool loading() { return _loading > 0; }
      };

inline UndoStack* Score::undoStack() const             { return _masterScore->undoStack();      }
inline const RepeatList& Score::repeatList()  const    { return _masterScore->repeatList();     }
inline const RepeatList& Score::repeatList2()  const   { return _masterScore->repeatList2();    }
inline TempoMap* Score::tempomap() const               { return _masterScore->tempomap();       }
inline TimeSigMap* Score::sigmap() const               { return _masterScore->sigmap();         }
inline QList<Excerpt*>& Score::excerpts()              { return _masterScore->excerpts();       }
inline const QList<Excerpt*>& Score::excerpts() const  { return _masterScore->excerpts();       }
inline QQueue<MidiInputEvent>* Score::midiInputQueue()          { return _masterScore->midiInputQueue();    }
inline std::list<MidiInputEvent>* Score::activeMidiPitches()    { return _masterScore->activeMidiPitches(); }

inline void Score::setUpdateAll()                      { _masterScore->setUpdateAll();          }

inline void Score::setLayoutAll(int staff, const Element* e) { _masterScore->setLayoutAll(staff, e); }
inline void Score::setLayout(const Fraction& tick, int staff, const Element* e) { _masterScore->setLayout(tick, staff, e); }
inline void Score::setLayout(const Fraction& tick1, const Fraction& tick2, int staff1, int staff2, const Element* e) { _masterScore->setLayout(tick1, tick2, staff1, staff2, e); }

inline CmdState& Score::cmdState()                     { return _masterScore->cmdState();        }
inline const CmdState& Score::cmdState() const         { return _masterScore->cmdState();        }
inline void Score::addLayoutFlags(LayoutFlags f)       { _masterScore->addLayoutFlags(f);        }
inline void Score::setInstrumentsChanged(bool v)       { _masterScore->setInstrumentsChanged(v); }
inline Movements* Score::movements()                   { return _masterScore->movements();       }
inline const Movements* Score::movements() const       { return _masterScore->movements();       }

inline Fraction Score::pos(POS pos) const              { return _masterScore->pos(pos);          }
inline void Score::setPos(POS pos, Fraction tick)      { _masterScore->setPos(pos, tick);        }

extern MasterScore* gscore;

extern MStyle* styleDefaults114();
extern MStyle* styleDefaults206();
extern MStyle* styleDefaults301();

Q_DECLARE_OPERATORS_FOR_FLAGS(LayoutFlags);

}     // namespace Ms


#endif

