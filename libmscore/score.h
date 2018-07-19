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
#include "select.h"
#include "synthesizerstate.h"
#include "mscoreview.h"
#include "segment.h"
#include "ottava.h"
#include "spannermap.h"
#include "layoutbreak.h"
#include "rehearsalmark.h"
#include <set>

class QPainter;

namespace Ms {

class Articulation;
class Audio;
class BarLine;
class Beam;
class Bracket;
class BSymbol;
class Chord;
class ChordRest;
class Clef;
class Cursor;
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
class Measure;
class MeasureBase;
class MuseScoreView;
class Note;
class Omr;
class Page;
class PageFormat;
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
class TextStyle;
class TimeSig;
class TimeSigMap;
class Tuplet;
class Undo;
class UndoCommand;
class UndoStack;
class Volta;
class Xml;
struct Channel;
struct Interval;
struct PageContext;
struct TEvent;

enum class ClefType : signed char;
enum class BeatType : char;
enum class SymId;
enum class Key;

extern bool showRubberBand;

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
      //--------------------
      REST,
      DOT,
      DOTDOT,
      DOT3
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

struct MidiMapping {
      Part* part;
      Channel* articulation;
      signed char port;
      signed char channel;
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
      int fret         { FRET_NONE };
      QPointF pos;
      };

//---------------------------------------------------------
//   LayoutFlag bits
//---------------------------------------------------------

enum class LayoutFlag : char {
      FIX_TICKS = 1,
      FIX_PITCH_VELO = 2,
      PLAY_EVENTS = 4
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

enum class PasteStatus : char {
      PS_NO_ERROR,
      NO_MIME,
      NO_DEST,
      DEST_TUPLET,
      DEST_NO_CR,
      TUPLET_CROSSES_BAR,
      DEST_LOCAL_TIME_SIGNATURE,
      DEST_TREMOLO
      };

//---------------------------------------------------------
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
//   @  pageFormat      PageFormat        the page format for the score
//   @P parts           array[Part]       the list of parts (read only)
//   @P poet            string            poet of the score (read only)
//   @P subtitle        string            subtitle of the score (read only)
//   @P title           string            title of the score (read only)
//---------------------------------------------------------

class Score : public QObject, public ScoreElement {
      Q_OBJECT
      Q_PROPERTY(QString                        composer          READ composer)
      Q_PROPERTY(int                            duration          READ duration)
      Q_PROPERTY(QQmlListProperty<Ms::Excerpt>  excerpts          READ qmlExcerpts)
      Q_PROPERTY(Ms::Measure*                   firstMeasure      READ firstMeasure)
      Q_PROPERTY(Ms::Measure*                   firstMeasureMM    READ firstMeasureMM)
      Q_PROPERTY(int                            harmonyCount      READ harmonyCount)
      Q_PROPERTY(bool                           hasHarmonies      READ hasHarmonies)
      Q_PROPERTY(bool                           hasLyrics         READ hasLyrics)
      Q_PROPERTY(int                            keysig            READ keysig)
      Q_PROPERTY(Ms::Measure*                   lastMeasure       READ lastMeasure)
      Q_PROPERTY(Ms::Measure*                   lastMeasureMM     READ lastMeasureMM)
      Q_PROPERTY(Ms::Segment*                   lastSegment       READ lastSegment)
      Q_PROPERTY(int                            lyricCount        READ lyricCount)
      Q_PROPERTY(QString                        name              READ name           WRITE setName)
      Q_PROPERTY(int                            nmeasures         READ nmeasures)
      Q_PROPERTY(int                            npages            READ npages)
      Q_PROPERTY(int                            nstaves           READ nstaves)
      Q_PROPERTY(int                            ntracks           READ ntracks)
      Q_PROPERTY(Ms::PageFormat*                pageFormat        READ pageFormat     WRITE undoChangePageFormat)
      Q_PROPERTY(QQmlListProperty<Ms::Part>     parts             READ qmlParts)
      Q_PROPERTY(QString                        poet              READ poet)
      Q_PROPERTY(QString                        subtitle          READ subtitle)
      Q_PROPERTY(QString                        title             READ title)
      Q_PROPERTY(QString                        mscoreVersion     READ mscoreVersion)
      Q_PROPERTY(QString                        mscoreRevision    READ mscoreRevision)

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
            FILE_CORRUPTED,
            FILE_USER_ABORT,
            FILE_IGNORE_ERROR
            };

   private:
      int _linkId;
      Score* _parentScore;          // set if score is an excerpt (part)
      QList<MuseScoreView*> viewer;

      QString _mscoreVersion;
      int _mscoreRevision;

      Revisions* _revisions;
      QList<Excerpt*> _excerpts;

      QString _layerTags[32];
      QString _layerTagComments[32];
      QList<Layer> _layer;
      int _currentLayer;

      ScoreFont* _scoreFont;
      int _pageNumberOffset;        ///< Offset for page numbers.

      MeasureBaseList _measures;          // here are the notes
      SpannerMap _spanner;
      std::set<Spanner*> _unmanagedSpanner;

      //
      // generated objects during layout:
      //
      QList<Page*> _pages;          // pages are build from systems
      QList<System*> _systems;      // measures are akkumulated to systems

      // temp values used during doLayout:
      int curPage;
      int curSystem;
      MeasureBase* curMeasure;

      UndoStack* _undo;

      QQueue<MidiInputEvent> _midiInputQueue;         // MIDI events that have yet to be processed
      std::list<MidiInputEvent> _activeMidiPitches;   // MIDI keys currently being held down
      QList<MidiMapping> _midiMapping;

      RepeatList* _repeatList;
      TimeSigMap* _sigmap;
      TempoMap* _tempomap;

      InputState _is;
      MStyle _style;

      QFileInfo info;
      bool _created;          ///< file is never saved, has generated name
      QString _tmpName;       ///< auto saved with this name if not empty
      QString _importedFilePath;    // file from which the score was imported, or empty

      // the following variables are reset on startCmd()
      //   modified during cmd processing and used in endCmd() to
      //   determine what to layout and what to repaint:

      QRectF refresh;               ///< area to update, canvas coordinates
      LayoutFlags layoutFlags;

      bool _updateAll;
      bool _layoutAll;        ///< do a complete relayout

      bool _undoRedo;         ///< true if in processing a undo/redo
      bool _playNote;         ///< play selected note after command
      bool _playChord;        ///< play whole chord for the selected note

      bool _excerptsChanged;
      bool _instrumentsChanged;
      bool _selectionChanged;

      bool _showInvisible;
      bool _showUnprintable;
      bool _showFrames;
      bool _showPageborders;
      bool _showInstrumentNames;
      bool _showVBox;

      bool _printing;   ///< True if we are drawing to a printer
      bool _playlistDirty;
      bool _autosaveDirty;
      bool _savedCapture          { false };      ///< True if we saved an image capture

//      bool _dirty;      ///< Score data was modified.
      bool _saved;      ///< True if project was already saved; only on first
                        ///< save a backup file will be created, subsequent
                        ///< saves will not overwrite the backup file.

      LayoutMode _layoutMode;

      Qt::KeyboardModifiers keyState;

      QList<Part*> _parts;
      QList<Staff*> _staves;

      int _pos[3];            ///< 0 - current, 1 - left loop, 2 - right loop

      int _fileDivision; ///< division of current loading *.mscx file
      int _mscVersion; ///< version of .mscx file during file read, then changed to MSCVERSION for drag and drop
      int _mscRealVersion;  ///< keep the actual and initial version of current loaded *.mscx file

      QMap<int, LinkedElements*> _elinks;
      QMap<QString, QString> _metaTags;

      bool _defaultsRead;            ///< defaults were read at MusicXML import, allow export of defaults in convertermode

      Selection _selection;
      SelectionFilter _selectionFilter;
      Omr* _omr;
      Audio* _audio;
      bool _showOmr;
      PlayMode _playMode;

      qreal _noteHeadWidth;
      QString accInfo;             ///< information used by the screen-reader
      int _midiPortCount;          // A count of JACK/ALSA midi out ports. Stored in a root score

      //------------------

      ChordRest* nextMeasure(ChordRest* element, bool selectBehavior = false, bool mmRest = false);
      ChordRest* prevMeasure(ChordRest* element, bool mmRest = false);
      void cmdSetBeamMode(Beam::Mode);
      void cmdFlip();
      Note* getSelectedNote();
      ChordRest* upStaff(ChordRest* cr);
      ChordRest* downStaff(ChordRest* cr);
      ChordRest* nextTrack(ChordRest* cr);
      ChordRest* prevTrack(ChordRest* cr);

      void padToggle(Pad n);
      void addTempo();
      void addMetronome();

      void cmdResetBeamMode();

      void cmdInsertClef(ClefType);
      void cmdAddGrace(NoteType, int);

      void removeChordRest(ChordRest* cr, bool clearSegment);
      void cmdMoveRest(Rest*, MScore::Direction);
      void cmdMoveLyrics(Lyrics*, MScore::Direction);

      void cmdIncDecDuration(int nSteps, bool stepDotted = false);

      void cmdAddBracket();

      void resetUserStretch();

      Page* addPage();
      bool layoutSystem(qreal& minWidth, qreal w, bool, bool);
      void createMMRests();
      bool layoutSystem1(qreal& minWidth, bool, bool);
      QList<System*> layoutSystemRow(qreal w, bool, bool);
      void addSystemHeader(Measure* m, bool);
      System* getNextSystem(bool, bool);
      bool doReLayout();

      void layoutStage2();
      void layoutStage3();
      void beamGraceNotes(Chord*, bool);

      void hideEmptyStaves(System* system, bool isFirstSystem);

      void checkSlurs();
      void checkScore();

      bool rewriteMeasures(Measure* fm, Measure* lm, const Fraction&, int staffIdx);
      bool rewriteMeasures(Measure* fm, const Fraction& ns, int staffIdx);
      void updateVelo();
      void swingAdjustParams(Chord*, int&, int&, int, int);
      bool isSubdivided(ChordRest*, int);
      void addAudioTrack();
      void parseVersion(const QString&);
      QList<Fraction> splitGapToMeasureBoundaries(ChordRest*, Fraction);
      void pasteChordRest(ChordRest* cr, int tick, const Interval&);
      void init();
      void removeGeneratedElements(Measure* mb, Measure* end);
      qreal cautionaryWidth(Measure* m, bool& hasCourtesy);

      void selectSingle(Element* e, int staffIdx);
      void selectAdd(Element* e);
      void selectRange(Element* e, int staffIdx);

      QQmlListProperty<Ms::Part> qmlParts() { return QmlListAccess<Ms::Part>(this, _parts); }
      QQmlListProperty<Ms::Excerpt> qmlExcerpts() { return QmlListAccess<Ms::Excerpt>(this, excerpts()); }
      FileError loadCompressedMsc(QIODevice*, bool ignoreVersionError);
      FileError read114(XmlReader&);
      FileError read1(XmlReader&, bool ignoreVersionError);

      void renderStaff(EventMap* events, Staff*);
      void renderSpanners(EventMap* events);
      void renderMetronome(EventMap* events, Measure* m, int tickOffset);

   protected:
      void createPlayEvents(Chord*);
      void createGraceNotesPlayEvents(QList<Chord*> gnb, int tick, Chord* chord, int& ontime);

      SynthesizerState _synthesizerState;

   signals:
      void posChanged(POS, unsigned);
      void playlistChanged();

   public:
      Score();
      Score(const MStyle*);
      Score(Score*);                // used for excerpts
      Score(Score*, const MStyle*);
      ~Score();

      Score* clone();
//      void setDirty(bool val);

      void rebuildBspTree();
      bool noStaves() const         { return _staves.empty(); }
      void insertPart(Part*, int);
      void removePart(Part*);
      void insertStaff(Staff*, int);
      void cmdRemoveStaff(int staffIdx);
      void removeStaff(Staff*);
      void addMeasure(MeasureBase*, MeasureBase*);
      void readStaff(XmlReader&);

      void cmdRemovePart(Part*);
      void cmdAddTie();
      void cmdAddHairpin(bool);
      void cmdAddOttava(Ottava::Type);
      void cmdAddStretch(qreal);
      void cmdResetNoteAndRestGroupings();
      void cmdDoubleDuration()      { cmdIncDecDuration(-1, 0); }
      void cmdHalfDuration()        { cmdIncDecDuration( 1, 0); }
      void cmdIncDurationDotted()   { cmdIncDecDuration(-1, 1); }
      void cmdDecDurationDotted()   { cmdIncDecDuration( 1, 1); }
      void cmdToggleLayoutBreak(LayoutBreak::Type);

      void addRemoveBreaks(int interval, bool lock);

      bool transpose(Note* n, Interval, bool useSharpsFlats);
      void transposeKeys(int staffStart, int staffEnd, int tickStart, int tickEnd, const Interval&, bool useInstrument = false, bool flip = false);
      bool transpose(TransposeMode mode, TransposeDirection, Key transposeKey, int transposeInterval,
         bool trKeys, bool transposeChordNames, bool useDoubleSharpsFlats);

      bool appendScore(Score*, bool addPageBreak = false, bool addSectionBreak = true);

      int pageIdx(Page* page) const { return _pages.indexOf(page); }

      bool write(Xml&, bool onlySelection);
      bool read(XmlReader&);

      QList<Staff*>& staves()                { return _staves; }
      const QList<Staff*>& staves() const    { return _staves; }
      int nstaves() const                    { return _staves.size(); }
      int ntracks() const                    { return _staves.size() * VOICES; }
      int npages() const                     { return _pages.size(); }

      int staffIdx(const Part*) const;
      int staffIdx(const Staff* staff) const { return _staves.indexOf((Staff*)staff, 0); }
      Staff* staff(int n) const              { return ((n >= 0) && (n < _staves.size())) ? _staves.at(n) : nullptr; }


      MeasureBase* pos2measure(const QPointF&, int* staffIdx, int* pitch,
         Segment**, QPointF* offset) const;

      void undoAddElement(Element* element);
      void undoAddCR(ChordRest* element, Measure*, int tick);
      void undoRemoveElement(Element* element);
      void undoChangeElement(Element* oldElement, Element* newElement);
      void undoChangeChordRestSize(ChordRest* cr, bool small);
      void undoChangeChordNoStem(Chord* cr, bool noStem);
      void undoChangePitch(Note* note, int pitch, int tpc1, int tpc2);
      void undoChangeFretting(Note* note, int pitch, int string, int fret, int tpc1, int tpc2);
      void spellNotelist(QList<Note*>& notes);
      void undoChangeTpc(Note* note, int tpc);
      void undoChangeChordRestLen(ChordRest* cr, const TDuration&);
      void undoChangeEndBarLineType(Measure*, BarLineType);
      void undoChangeBarLineSpan(Staff*, int span, int spanFrom, int spanTo);
      void undoChangeSingleBarLineSpan(BarLine* barLine, int span, int spanFrom, int spanTo);
      void undoTransposeHarmony(Harmony*, int, int);
      void undoExchangeVoice(Measure* measure, int val1, int val2, int staff1, int staff2);
      void undoRemovePart(Part* part, int idx);
      void undoInsertPart(Part* part, int idx);
      void undoRemoveStaff(Staff* staff);
      void undoInsertStaff(Staff* staff, int idx, bool createRests=true);
      void undoChangeInvisible(Element*, bool);
      void undoChangeBracketSpan(Staff* staff, int column, int span);
      void undoChangeBracketType(Bracket* bracket, BracketType type);
      void undoChangeTuning(Note*, qreal);
      void undoChangePageFormat(PageFormat*);
      void undoChangePageFormat(PageFormat*, qreal spatium, int);
      void undoChangeUserMirror(Note*, MScore::DirectionH);
      void undoChangeKeySig(Staff* ostaff, int tick, KeySigEvent);
      void undoChangeClef(Staff* ostaff, Segment*, ClefType st);
      void undoChangeBarLine(Measure* m, BarLineType);
      void undoChangeProperty(ScoreElement*, P_ID, const QVariant&, PropertyStyle ps = PropertyStyle::NOSTYLE);
      void undoPropertyChanged(Element*, P_ID, const QVariant& v);
      void undoPropertyChanged(ScoreElement*, P_ID, const QVariant& v);
      UndoStack* undo() const;
      void undo(UndoCommand* cmd) const;
      void undoRemoveMeasures(Measure*, Measure*);
      void undoAddBracket(Staff* staff, int level, BracketType type, int span);
      void undoRemoveBracket(Bracket*);
      void undoInsertTime(int tick, int len);

      void setGraceNote(Chord*,  int pitch, NoteType type, int len);

      Segment* setNoteRest(Segment*, int track, NoteVal nval, Fraction, MScore::Direction stemDirection = MScore::Direction::AUTO, bool rhythmic = false);
      void changeCRlen(ChordRest* cr, const TDuration&);

      Fraction makeGap(Segment*, int track, const Fraction&, Tuplet*, bool keepChord = false);
      bool makeGap1(int baseTick, int staffIdx, Fraction len, int voiceOffset[VOICES]);
      bool makeGapVoice(Segment* seg, int track, Fraction len, int tick);

      Rest* addRest(int tick, int track, TDuration, Tuplet*);
      Rest* addRest(Segment* seg, int track, TDuration d, Tuplet*);
      Chord* addChord(int tick, TDuration d, Chord* oc, bool genTie, Tuplet* tuplet);

      ChordRest* addClone(ChordRest* cr, int tick, const TDuration& d);
      Rest* setRest(int tick,  int track, Fraction, bool useDots, Tuplet* tuplet, bool useFullMeasureRest = true);

      void upDown(bool up, UpDownMode);
      ChordRest* searchNote(int tick, int track) const;

      // undo/redo ops
      void addArticulation(ArticulationType);
      void changeAccidental(AccidentalType);
      void changeAccidental(Note* oNote, Ms::AccidentalType);

      void addElement(Element*);
      void removeElement(Element*);

      Note* addPitch(NoteVal&, bool addFlag);
      void addPitch(int pitch, bool addFlag);
      Note* addTiedMidiPitch(int pitch, bool addFlag, Chord* prevChord);
      Note* addMidiPitch(int pitch, bool addFlag);
      Note* addNote(Chord*, NoteVal& noteVal);

      NoteVal noteValForPosition(Position pos, bool &error);

      void deleteItem(Element*);
      void cmdDeleteSelectedMeasures();
      void cmdDeleteSelection();
      void cmdFullMeasureRest();

      void putNote(const QPointF& pos, bool replace);
      void putNote(const Position& pos, bool replace);
      void repitchNote(const Position& pos, bool replace);
      void regroupNotesAndRests(int startTick, int endTick, int track);
      void cmdAddPitch(int pitch, bool addFlag);

      //@ to be used at least once by plugins of type "dialog" before score modifications to make them undoable
      Q_INVOKABLE void startCmd();        // start undoable command
      //@ to be used at least once by plugins of type "dialog" after score modifications to make them undoable
      Q_INVOKABLE void endCmd(bool rollback = false);          // end undoable command
      void end();             // layout & update canvas
      void end1();
      void update();

      void cmdRemoveTimeSig(TimeSig*);
      void cmdAddTimeSig(Measure*, int staffIdx, TimeSig*, bool local);

      void setUpdateAll(bool v = true) { _updateAll = v;   }
      void setLayoutAll(bool val);
      bool layoutAll() const           { return _layoutAll; }
      void addRefresh(const QRectF& r) { refresh |= r;     }
      const QRectF& getRefresh() const { return refresh;     }

      void changeVoice(int);

      void colorItem(Element*);
      QList<Part*>& parts()                { return _parts; }
      const QList<Part*>& parts() const    { return _parts; }

      void appendPart(Part* p);
      void updateStaffIndex();
      void sortStaves(QList<int>& dst);

      bool showInvisible() const       { return _showInvisible; }
      bool showUnprintable() const     { return _showUnprintable; }
      bool showFrames() const          { return _showFrames; }
      bool showPageborders() const     { return _showPageborders; }
      bool showInstrumentNames() const { return _showInstrumentNames; }
      bool showVBox() const            { return _showVBox; }
      void setShowInvisible(bool v);
      void setShowUnprintable(bool v);
      void setShowFrames(bool v);
      void setShowPageborders(bool v);
      void setShowInstrumentNames(bool v) { _showInstrumentNames = v; }
      void setShowVBox(bool v)            { _showVBox = v;            }

      FileError loadMsc(QString name, bool ignoreVersionError);
      FileError loadMsc(QString name, QIODevice*, bool ignoreVersionError);

      bool saveFile(QFileInfo& info);
      bool saveFile(QIODevice* f, bool msczFormat, bool onlySelection = false);
      bool saveCompressedFile(QFileInfo&, bool onlySelection);
      bool saveCompressedFile(QIODevice*, QFileInfo&, bool onlySelection);
      bool exportFile();

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

      int pos();
      Measure* tick2measure(int tick) const;
      Measure* tick2measureMM(int tick) const;
      MeasureBase* tick2measureBase(int tick) const;
      Segment* tick2segment(int tick, bool first = false, Segment::Type st = Segment::Type::All,bool useMMrest = false ) const;
      Segment* tick2segmentMM(int tick, bool first = false, Segment::Type st = Segment::Type::All) const;
      Segment* tick2segmentEnd(int track, int tick) const;
      Segment* tick2leftSegment(int tick) const;
      Segment* tick2rightSegment(int tick) const;
      void fixTicks();
      bool addArticulation(Element*, Articulation* atr);

      void cmd(const QAction*);
      int fileDivision(int t) const { return ((qint64)t * MScore::division + _fileDivision/2) / _fileDivision; }
      bool saveFile();

      QFileInfo* fileInfo()          { return &info; }
      QString name() const           { return info.completeBaseName(); }
      void setName(QString s);

      QString importedFilePath() const           { return _importedFilePath; }
      void setImportedFilePath(const QString& filePath);

      bool isSavable() const;
      bool dirty() const;
      void setCreated(bool val)      { _created = val;        }
      bool created() const           { return _created;       }
      bool savedCapture() const      { return _savedCapture;  }
      bool saved() const             { return _saved;         }
      void setSaved(bool v)          { _saved = v;            }
      void setSavedCapture(bool v)   { _savedCapture = v;     }
      bool printing() const          { return _printing;      }
      void setPrinting(bool val)     { _printing = val;      }
      void setAutosaveDirty(bool v)  { _autosaveDirty = v;    }
      bool autosaveDirty() const     { return _autosaveDirty; }
      bool playlistDirty()           { return _playlistDirty; }
      void setPlaylistDirty()        { _playlistDirty = true; }

      void spell();
      void spell(int startStaff, int endStaff, Segment* startSegment, Segment* endSegment);
      void spell(Note*);
      int nextSeg(int tick, int track);

      MStyle* style()                          { return &_style;                  }
      const MStyle* style() const              { return &_style;                  }
      void setStyle(const MStyle& s)           { _style = s;                      }
      bool loadStyle(const QString&);
      bool saveStyle(const QString&);

      QVariant style(StyleIdx idx) const   { return _style.value(idx);   }
      Spatium  styleS(StyleIdx idx) const  { return Spatium(_style.value(idx).toDouble());  }
      qreal    styleP(StyleIdx idx) const  { return _style.value(idx).toDouble() * spatium();  }
      QString  styleSt(StyleIdx idx) const { return _style.value(idx).toString(); }
      bool     styleB(StyleIdx idx) const  { return _style.value(idx).toBool();  }
      qreal    styleD(StyleIdx idx) const  { return _style.value(idx).toDouble();  }
      int      styleI(StyleIdx idx) const  { return _style.value(idx).toInt();  }

      const TextStyle& textStyle(TextStyleType idx) const { return _style.textStyle(idx); }
      const TextStyle& textStyle(const QString& s) const  { return _style.textStyle(s); }

      // These position are in ticks and not uticks
      int playPos() const                      { return pos(POS::CURRENT);   }
      void setPlayPos(int tick)                { setPos(POS::CURRENT, tick); }
      int loopInTick() const                   { return pos(POS::LEFT);      }
      int loopOutTick() const                  { return pos(POS::RIGHT);     }
      void setLoopInTick(int tick)             { setPos(POS::LEFT, tick);    }
      void setLoopOutTick(int tick)            { setPos(POS::RIGHT, tick);   }

      int pos(POS pos) const                   {  return _pos[int(pos)]; }
      void setPos(POS pos, int tick);

      bool noteEntryMode() const               { return inputState().noteEntryMode(); }
      void setNoteEntryMode(bool val)          { inputState().setNoteEntryMode(val); }
      NoteEntryMethod noteEntryMethod() const         { return inputState().noteEntryMethod();        }
      void setNoteEntryMethod(NoteEntryMethod m)      { inputState().setNoteEntryMethod(m);           }
      bool usingNoteEntryMethod(NoteEntryMethod m)    { return inputState().usingNoteEntryMethod(m);  }
      int inputPos() const;
      int inputTrack() const                   { return inputState().track(); }
      const InputState& inputState() const     { return _is;                  }
      InputState& inputState()                 { return _is;                  }
      void setInputState(const InputState& st) { _is = st;                    }
      void setInputTrack(int t)                { inputState().setTrack(t);    }

      void spatiumChanged(qreal oldValue, qreal newValue);

      PasteStatus cmdPaste(const QMimeData* ms, MuseScoreView* view);
      PasteStatus pasteStaff(XmlReader&, Segment* dst, int staffIdx);
      void pasteSymbols(XmlReader& e, ChordRest* dst);
      void renderMidi(EventMap* events);
      void renderMidi(EventMap* events, bool metronome, bool expandRepeats);

      BeatType tick2beatType(int tick);

      int mscVersion() const     { return _mscVersion;     }
      int mscRealVersion() const { return _mscRealVersion; }
      void setMscVersion(int v)  { _mscVersion = v;        }

      void addLyrics(int tick, int staffIdx, const QString&);

      QList<Excerpt*>& excerpts()             { return _excerpts; }
      const QList<Excerpt*>& excerpts() const { return _excerpts; }

      int midiPort(int idx) const;
      int midiChannel(int idx) const;
      inline QQueue<MidiInputEvent>* midiInputQueue()       { return &_midiInputQueue;    }
      inline std::list<MidiInputEvent>* activeMidiPitches() { return &_activeMidiPitches; }
      QList<MidiMapping>* midiMapping()       { return &_midiMapping;          }
      MidiMapping* midiMapping(int channel)   { return &_midiMapping[channel]; }
      void rebuildMidiMapping();
      void updateChannel();
      void updateSwing();
      void createPlayEvents();

      int midiPortCount() const;
      void setMidiPortCount(int);

      void cmdConcertPitchChanged(bool, bool /*useSharpsFlats*/);

      void setTempomap(TempoMap* tm);
      TempoMap* tempomap() const;
      TimeSigMap* sigmap() const;

      void setTempo(Segment*, qreal);
      void setTempo(int tick, qreal bps);
      void removeTempo(int tick);
      void setPause(int tick, qreal seconds);
      qreal tempo(int tick) const;

      bool defaultsRead() const                      { return _defaultsRead;    }
      void setDefaultsRead(bool b)                   { _defaultsRead = b;       }
      Text* getText(TextStyleType subtype);

      void lassoSelect(const QRectF&);
      void lassoSelectEnd();

      Page* searchPage(const QPointF&) const;
      QList<System*> searchSystem(const QPointF& p) const;
      Measure* searchMeasure(const QPointF& p) const;

      bool getPosition(Position* pos, const QPointF&, int voice) const;

      void cmdDeleteTuplet(Tuplet*, bool replaceWithRest);

      void moveBracket(int staffIdx, int srcCol, int dstCol);
      Measure* getCreateMeasure(int tick);

      void adjustBracketsDel(int sidx, int eidx);
      void adjustBracketsIns(int sidx, int eidx);
      void adjustKeySigs(int sidx, int eidx, KeyList km);

      void endUndoRedo();
      Measure* searchLabel(const QString& s, Measure* startMeasure = nullptr, Measure* endMeasure = nullptr);
      Measure* searchLabelWithinSectionFirst(const QString& s, Measure* sectionStartMeasure, Measure* sectionEndMeasure);
      RepeatList* repeatList() const;
      qreal utick2utime(int tick) const;
      int utime2utick(qreal utime) const;
      //@ ??
      Q_INVOKABLE void updateRepeatList(bool expandRepeats);

      void nextInputPos(ChordRest* cr, bool);
      void cmdMirrorNoteHead();

      qreal spatium() const                    { return style()->spatium();    }
      void setSpatium(qreal v);
      PageFormat* pageFormat()                 { return style()->pageFormat(); }
      const PageFormat* pageFormat() const     { return style()->pageFormat(); }
      void setPageFormat(const PageFormat& pf) { style()->setPageFormat(pf);   }
      qreal loWidth() const;
      qreal loHeight() const;

      const QList<Page*>& pages() const        { return _pages;                }
      QList<System*>* systems()                { return &_systems;             }

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

      Ms::Segment* firstSegment(Segment::Type s) const;
      //@ returns the first segment of the score of the given type (use Segment.Clef, ... enum)
      Q_INVOKABLE Ms::Segment* firstSegment(int segType = static_cast<int>(Segment::Type::All)) const;
      Ms::Segment* firstSegmentMM(Segment::Type s = Segment::Type::All) const;
      Ms::Segment* lastSegment() const;

      void connectTies(bool silent=false);

      qreal point(const Spatium sp) const { return sp.val() * spatium(); }

      void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      void scanElementsInRange(void* data, void (*func)(void*, Element*), bool all = true);
      QByteArray buildCanonical(int track);
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
      void cmdAddInterval(int, const QList<Note*>&);
      void cmdCreateTuplet(ChordRest*, Tuplet*);
      Omr* omr() const                         { return _omr;     }
      void setOmr(Omr* o)                      { _omr = o;        }
      void removeOmr();
      bool showOmr() const                     { return _showOmr; }
      void setShowOmr(bool v)                  { _showOmr = v;    }
      void removeAudio();
      void enqueueMidiEvent(MidiInputEvent ev) { midiInputQueue()->enqueue(ev); }

      //@ ??
      Q_INVOKABLE void doLayout();
      void layoutSystems();
      void layoutSystems2();
      void layoutLinear();
      void layoutPages();
      void layoutSystemsUndoRedo();
      void layoutPagesUndoRedo();
      Page* getEmptyPage();

      void layoutChords1(Segment* segment, int staffIdx);
      qreal layoutChords2(QList<Note*>& notes, bool up);
      void layoutChords3(QList<Note*>& notes, Staff* staff, Segment* segment);

      SynthesizerState& synthesizerState()     { return _synthesizerState; }
      void setSynthesizerState(const SynthesizerState& s);

      void addLayoutFlags(LayoutFlags val)               { layoutFlags |= val; }
      void updateHairpin(Hairpin*);       // add/modify hairpin to pitchOffset list
      void removeHairpin(Hairpin*);       // remove hairpin from pitchOffset list
      Score* parentScore() const    { return _parentScore; }
      void setParentScore(Score* s) { _parentScore = s;    }
      const Score* rootScore() const;
      Score* rootScore();
      void addExcerpt(Score*);
      void removeExcerpt(Score*);

      void createRevision();
      QByteArray readCompressedToBuffer();
      QByteArray readToBuffer();
      void writeSegments(Xml& xml, int strack, int etrack, Segment* first, Segment* last, bool, bool, bool, bool);

      const QMap<QString, QString>& metaTags() const   { return _metaTags; }
      QMap<QString, QString>& metaTags()               { return _metaTags; }
      void setMetaTags(const QMap<QString,QString>& t) { _metaTags = t; }

      //@ returns as a string the metatag named 'tag'
      Q_INVOKABLE QString metaTag(const QString& tag) const;
      //@ sets the metatag named 'tag' to 'val'
      Q_INVOKABLE void setMetaTag(const QString& tag, const QString& val);

      QMap<int, LinkedElements*>& links();
      void layoutFingering(Fingering*);
      void cmdSplitMeasure(ChordRest*);
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
      bool playNote() const                 { return _playNote; }
      void setPlayNote(bool v)              { _playNote = v;    }
      bool playChord() const                { return _playChord; }
      void setPlayChord(bool v)             { _playChord = v;    }
      bool excerptsChanged() const          { return _excerptsChanged; }
      void setExcerptsChanged(bool val)     { _excerptsChanged = val; }
      bool instrumentsChanged() const       { return _instrumentsChanged; }
      void setInstrumentsChanged(bool val)  { _instrumentsChanged = val; }
      bool selectionChanged() const         { return _selectionChanged; }
      void setSelectionChanged(bool val)    { _selectionChanged = val;  }
      void setSoloMute();

      LayoutMode layoutMode() const         { return _layoutMode; }
      void setLayoutMode(LayoutMode lm)     { _layoutMode = lm;   }

      void doLayoutSystems();
      void doLayoutPages();
      Tuplet* searchTuplet(XmlReader& e, int id);
      void cmdSelectAll();
      void cmdSelectSection();
      void setUndoRedo(bool val)            { _undoRedo = val; }
      bool undoRedo() const                 { return _undoRedo; }
      void respace(QList<ChordRest*>* elements);
      void transposeSemitone(int semitone);
      MeasureBase* insertMeasure(Element::Type type, MeasureBase*,
         bool createEmptyMeasures = false, bool moveHeader = true);
      Audio* audio() const         { return _audio;    }
      void setAudio(Audio* a)      { _audio = a;       }
      PlayMode playMode() const    { return _playMode; }
      void setPlayMode(PlayMode v) { _playMode = v;    }
      int linkId();
      void linkId(int);
      int getLinkId() const { return _linkId; }
      QList<Score*> scoreList();
      bool switchLayer(const QString& s);
      void layoutPage(const PageContext&,  qreal);
      //@ appends to the score a named part as last part
      Q_INVOKABLE void appendPart(const QString&);
      //@ appends to the score a number of measures
      Q_INVOKABLE void appendMeasures(int);
#ifdef SCRIPT_INTERFACE
      //@ ??
      Q_INVOKABLE void addText(const QString&, const QString&);
      //@ creates and returns a cursor to be used to navigate the score
      Q_INVOKABLE Ms::Cursor* newCursor();
#endif
      qreal computeMinWidth(Segment* fs, bool firstMeasureInSystem);
      void updateBarLineSpans(int idx, int linesOld, int linesNew);

      const std::multimap<int, Spanner*>& spanner() const { return _spanner.map(); }
      SpannerMap& spannerMap() { return _spanner; }
      bool isSpannerStartEnd(int tick, int track) const;
      void removeSpanner(Spanner*);
      void addSpanner(Spanner*);
      void cmdAddSpanner(Spanner* spanner, const QPointF& pos);
      void cmdAddSpanner(Spanner* spanner, int staffIdx, Segment* startSegment, Segment* endSegment);
      void checkSpanner(int startTick, int lastTick);
      const std::set<Spanner*>unmanagedSpanners() { return _unmanagedSpanner; }
      void addUnmanagedSpanner(Spanner*);
      void removeUnmanagedSpanner(Spanner*);

      Hairpin* addHairpin(bool crescendo, int tickStart, int tickEnd, int track);

      ChordRest* findCR(int tick, int track) const;
      ChordRest* findCRinStaff(int tick, int staffIdx) const;
      void layoutSpanner();
      void insertTime(int tickPos, int tickLen);

      ScoreFont* scoreFont() const            { return _scoreFont;     }
      void setScoreFont(ScoreFont* f)         { _scoreFont = f;        }

      qreal noteHeadWidth() const     { return _noteHeadWidth; }
      void setNoteHeadWidth( qreal n) { _noteHeadWidth = n; }

      QList<int> uniqueStaves() const;
      void transpositionChanged(Part*, Interval, int tickStart = 0, int tickEnd = -1);

      void moveUp(ChordRest*);
      void moveDown(ChordRest*);
      Element* upAlt(Element*);
      Note* upAltCtrl(Note*) const;
      Element* downAlt(Element*);
      Note* downAltCtrl(Note*) const;

      Element* firstElement();
      Element* lastElement();

      QString title();
      QString subtitle();
      QString composer();
      QString poet();
      int nmeasures();
      bool hasLyrics();
      bool hasHarmonies();
      int  lyricCount();
      int  harmonyCount();
      Q_INVOKABLE QString extractLyrics();
      int keysig();
      int duration();

      void cmdInsertClef(Clef* clef, ChordRest* cr);

      void cmdExplode();
      void cmdImplode();
      void cmdSlashFill();
      void cmdSlashRhythm();
      void cmdResequenceRehearsalMarks();
      void cmdExchangeVoice(int, int);

      void setAccessibleInfo(QString s) { accInfo = s.remove(":").remove(";"); }
      QString accessibleInfo()          { return accInfo;          }

      QImage createThumbnail();
      QString createRehearsalMarkText(RehearsalMark* current) const;
      QString nextRehearsalMarkText(RehearsalMark* previous, RehearsalMark* current) const;

      //@ ??
      Q_INVOKABLE void cropPage(qreal margins);
      bool sanityCheck(const QString& name = nullptr);

      bool checkKeys();
      bool checkClefs();

      void switchToPageMode();

      virtual QVariant getProperty(P_ID) const override;
      virtual bool setProperty(P_ID, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      friend class ChangeSynthesizerState;
      friend class Chord;
      };

extern Score* gscore;
extern void fixTicks();

Q_DECLARE_OPERATORS_FOR_FLAGS(LayoutFlags);

}     // namespace Ms


#endif

