//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
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

#include "input.h"
#include "mscore.h"
#include "style.h"
#include "durationtype.h"
#include "select.h"
#include "fraction.h"
#include "interval.h"
#include "sparm.h"
#include "mscoreview.h"
#include "segment.h"
#include "accidental.h"
#include "note.h"

class TempoMap;
struct TEvent;
class SigEvent;
class TimeSigMap;
class System;
class TextStyle;
class Page;
class PageFormat;
class ElementList;
class Selection;
class Segment;
class Rest;
class Xml;
class Articulation;
class Note;
class Chord;
class ChordRest;
class Slur;
class Hairpin;
class Undo;
class Part;
class BSymbol;
class KeySig;
class KeySigEvent;
class Volta;
class MidiEvent;
class Excerpt;
class EventMap;
class Harmony;
struct Channel;
class Tuplet;
class Dynamic;
class Measure;
class MeasureBase;
class Staff;
class Part;
class Instrument;
class UndoStack;
class RepeatList;
class MusicXmlCreator;
class TimeSig;
class Clef;
class Beam;
class Lyrics;
class Text;
class Omr;
class Audio;
class Parameter;
class StaffType;
class Revisions;
class Spanner;
class MuseScoreView;
class LinkedElements;
class Fingering;
class QPainter;
class FiguredBass;
class UndoCommand;
class Cursor;
struct PageContext;

extern bool showRubberBand;

enum {
      PAD_NOTE00,
      PAD_NOTE0,
      PAD_NOTE1,
      PAD_NOTE2,
      PAD_NOTE4,
      PAD_NOTE8,
      PAD_NOTE16,
      PAD_NOTE32,
      PAD_NOTE64,
      PAD_NOTE128,
      //--------------------
      PAD_REST,
      PAD_DOT,
      PAD_DOTDOT,
      };

//---------------------------------------------------------
//   LayoutMode
//---------------------------------------------------------

enum LayoutMode {
      LayoutPage, LayoutFloat, LayoutLine, LayoutSystem
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
      char port;
      char channel;
      Part* part;
      Channel* articulation;
      };

//---------------------------------------------------------
//   MidiInputEvent
//---------------------------------------------------------

struct MidiInputEvent {
      int pitch;
      bool chord;
      };

//---------------------------------------------------------
//   Position
//---------------------------------------------------------

struct Position {
      Segment* segment;
      int staffIdx;
      int line;
      int fret;
      QPointF pos;
      };

//---------------------------------------------------------
//   LayoutFlag bits
//---------------------------------------------------------

enum LayoutFlag {
      LAYOUT_FIX_TICKS = 1,
      LAYOUT_FIX_PITCH_VELO = 2
      };

typedef QFlags<LayoutFlag> LayoutFlags;
Q_DECLARE_OPERATORS_FOR_FLAGS(LayoutFlags)

//---------------------------------------------------------
//   PlayMode
//---------------------------------------------------------

enum PlayMode {
      PLAYMODE_SYNTHESIZER,
      PLAYMODE_AUDIO
      };

//---------------------------------------------------------
//   Layer
//---------------------------------------------------------

struct Layer {
      QString name;
      uint tags;
      };

//---------------------------------------------------------
//   @@ Score
//   @P name QString    name of the score
//   @P nstaves int     number of staves, read only
//   @P ntracks int     number of tracks (staves * 4), read only
//   @P npages  int     number of pages, read only
//---------------------------------------------------------

class Score : public QObject {
      Q_OBJECT
      Q_PROPERTY(QString name READ name WRITE setName)
      Q_PROPERTY(int nstaves  READ nstaves)
      Q_PROPERTY(int ntracks  READ ntracks)
      Q_PROPERTY(int npages   READ npages)

   public:
      enum FileError {
            FILE_NO_ERROR,
            FILE_ERROR,
            FILE_NOT_FOUND,
            FILE_OPEN_ERROR,
            FILE_BAD_FORMAT,
            FILE_UNKNOWN_TYPE,
            FILE_NO_ROOTFILE,
            FILE_TOO_OLD,
            FILE_TOO_NEW
            };

   private:
      int _linkId;
      Score* _parentScore;          // set if score is an excerpt (part)
      QReadWriteLock _layoutLock;
      QList<MuseScoreView*> viewer;

      QDate _creationDate;
      QString _mscoreVersion;
      int _mscoreRevision;

      Revisions* _revisions;
      QList<Excerpt*> _excerpts;

      QString _layerTags[32];
      QString _layerTagComments[32];
      QList<Layer> _layer;
      int _currentLayer;

      int _symIdx;                  // used symbol set, derived from style
      int _pageNumberOffset;        ///< Offset for page numbers.

      MeasureBaseList _measures;          // here are the notes
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

      QQueue<MidiInputEvent> midiInputQueue;
      QList<MidiMapping> _midiMapping;

      RepeatList* _repeatList;
      TimeSigMap* _sigmap;
      TempoMap* _tempomap;

      InputState _is;
      MStyle _style;
      QList<StaffType*> _staffTypes;

      QFileInfo info;
      bool _created;          ///< file is never saved, has generated name
      QString _tmpName;       ///< auto saved with this name if not empty

      // the following variables are reset on startCmd()
      //   modified during cmd processing and used in endCmd() to
      //   determine what to layout and what to repaint:

      QRectF refresh;
      Measure* startLayout;   ///< start a relayout at this measure
      LayoutFlags layoutFlags;

      bool _testMode;               // prepare for regression tests

      bool _updateAll;
      bool _layoutAll;        ///< do a complete relayout

      bool _undoRedo;         ///< true if in processing a undo/redo
      bool _playNote;         ///< play selected note after command

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
      bool _dirty;      ///< Score data was modified.
      bool _saved;      ///< True if project was already saved; only on first
                        ///< save a backup file will be created, subsequent
                        ///< saves will not overwrite the backup file.

      LayoutMode _layoutMode;

      Qt::KeyboardModifiers keyState;

      QList<Part*> _parts;
      QList<Staff*> _staves;

      int _playPos;     ///< sequencer seek position

      bool _foundPlayPosAfterRepeats; ///< Temporary used during playback rendering
                                      ///< indicating if playPos after expanded repeats
                                      ///< has been calculated.

      int _fileDivision; ///< division of current loading *.msc file
      int _mscVersion;   ///< version of current loading *.msc file
      QHash<int, LinkedElements*> _elinks;

      QMap<QString, QString> _metaTags;

      QList<MusicXmlCreator*> _creators;
      bool _creditsRead;             ///< credits were read at MusicXML import
      bool _defaultsRead;            ///< defaults were read at MusicXML import, allow export of defaults in convertermode

      Selection _selection;
      QList<KeySig*> customKeysigs;
      Omr* _omr;
      Audio* _audio;
      bool _showOmr;
      PlayMode _playMode;

      SyntiState _syntiState;

      //------------------

      ChordRest* nextMeasure(ChordRest* element, bool selectBehavior = false);
      ChordRest* prevMeasure(ChordRest* element);
      void cmdSetBeamMode(int);
      void cmdFlip();
      Note* getSelectedNote();
      Note* upAlt(Element*);
      Note* upAltCtrl(Note*) const;
      Note* downAlt(Element*);
      Note* downAltCtrl(Note*) const;
      ChordRest* upStaff(ChordRest* cr);
      ChordRest* downStaff(ChordRest* cr);
      void moveUp(Chord*);
      void moveDown(Chord*);

      void moveInputPos(Segment* s);
      void moveToNextInputPos();

      void padToggle(int n);

      void cmdAddPitch(int note, bool addFlag);

      void addTempo();
      void addMetronome();

      void cmdResetBeamMode();

      void cmdInsertClef(ClefType);
      void cmdExchangeVoice(int, int);

      void removeChordRest(ChordRest* cr, bool clearSegment);
      void cmdMoveRest(Rest*, MScore::Direction);
      void cmdMoveLyrics(Lyrics*, MScore::Direction);

      void cmdHalfDuration();
      void cmdDoubleDuration();

      void resetUserStretch();

      Page* addPage();
      bool layoutSystem(qreal& minWidth, qreal w, bool, bool);
      bool layoutSystem1(qreal& minWidth, bool, bool);
      QList<System*> layoutSystemRow(qreal w, bool, bool);
      void addSystemHeader(Measure* m, bool);
      System* getNextSystem(bool, bool);
      bool doReLayout();
      Measure* skipEmptyMeasures(Measure*, System*);

      void layoutStage1();
      void layoutStage2();
      void layoutStage3();
      void transposeKeys(int staffStart, int staffEnd, int tickStart, int tickEnd, const Interval&);
      void reLayout(Measure*);

      void checkSlurs();
      void checkScore();
      bool rewriteMeasures(Measure* fm, Measure* lm, const Fraction&);
      void rewriteMeasures(Measure* fm, const Fraction& ns);
      void updateVelo();
      void addAudioTrack();
      void parseVersion(const QString&);
      QList<Fraction> splitGapToMeasureBoundaries(ChordRest*, Fraction);
      void pasteChordRest(ChordRest* cr, int tick);
      void init();
      void removeGeneratedElements(Measure* mb, Measure* end);
      qreal cautionaryWidth(Measure* m);

   public:
      void setDirty(bool val);

      // read context
      int curTick;                  // for read optimizations
      int curTrack;
      QList<Spanner*> spanner;
      QList<Beam*> beams;

      void rebuildBspTree();
      bool noStaves() const         { return _staves.empty(); }
      void insertPart(Part*, int);
      void removePart(Part*);
      void insertStaff(Staff*, int);
      void cmdRemoveStaff(int staffIdx);
      void removeStaff(Staff*);
      void addMeasure(MeasureBase*, MeasureBase*);
      void readStaff(const QDomElement&);

      void cmdInsertPart(Part*, int);
      void cmdRemovePart(Part*);
      void cmdAddTie();
      void cmdAddHairpin(bool);
      void cmdAddStretch(qreal);
      void transpose(Note* n, Interval, bool useSharpsFlats);

      Score();
      Score(const MStyle*);
      Score(Score*);                // used for excerpts
      ~Score();

      Score* clone();
      bool appendScore(Score*);

      int pageIdx(Page* page) const { return _pages.indexOf(page); }

      void write(Xml&, bool onlySelection);
      bool read(const QDomElement&);
      FileError read114(const QDomElement&);
      FileError read1(const QDomElement&, bool ignoreVersionError);
      FileError loadCompressedMsc(QString name, bool ignoreVersionError);

      QList<Staff*>& staves()                { return _staves; }
      const QList<Staff*>& staves() const    { return _staves; }
      int nstaves() const                    { return _staves.size(); }
      int ntracks() const                    { return _staves.size() * VOICES; }
      int npages() const                     { return _pages.size(); }

      int staffIdx(const Part*) const;
      int staffIdx(const Staff* staff) const { return _staves.indexOf((Staff*)staff, 0); }
      Staff* staff(int n) const              { return _staves.value(n); }

      Part* part(int staffIdx);

      MeasureBase* pos2measure(const QPointF&, int* staffIdx, int* pitch,
         Segment**, QPointF* offset) const;

      void undoAddElement(Element* element);
      void undoAddGrace(Chord* c, Segment* s, bool behind);
      void undoAddCR(ChordRest* element, Measure*, int tick);
      void undoRemoveElement(Element* element);
      void undoChangeElement(Element* oldElement, Element* newElement);
      void undoChangeRepeatFlags(Measure*, int);
      void undoChangeVoltaEnding(Volta* volta, const QList<int>& l);
      void undoChangeVoltaText(Volta* volta, const QString& s);
      void undoChangeChordRestSize(ChordRest* cr, bool small);
      void undoChangeChordNoStem(Chord* cr, bool noStem);
      void undoChangePitch(Note* note, int pitch, int tpc, int line/*, int fret, int string*/);
      void spellNotelist(QList<Note*>& notes);
      void undoChangeTpc(Note* note, int tpc);
      void undoChangeChordRestLen(ChordRest* cr, const TDuration&);
      void undoChangeEndBarLineType(Measure*, BarLineType);
      void undoChangeBarLineSpan(Staff*, int);
      void undoChangeDynamic(Dynamic* e, int velocity, Element::DynamicType type);
      void undoTransposeHarmony(Harmony*, int, int);
      void undoExchangeVoice(Measure* measure, int val1, int val2, int staff1, int staff2);
      void undoRemovePart(Part* part, int idx);
      void undoInsertPart(Part* part, int idx);
      void undoRemoveStaff(Staff* staff, int idx);
      void undoInsertStaff(Staff* staff, int idx);
      void undoChangeInvisible(Element*, bool);
      void undoMove(Element* e, const QPointF& pt);
      void undoChangeBracketSpan(Staff* staff, int column, int span);
      void undoChangeTuning(Note*, qreal);
      void undoChangePageFormat(PageFormat*, qreal spatium, int);
      void undoChangeUserMirror(Note*, MScore::DirectionH);
      void undoChangeKeySig(Staff* ostaff, int tick, KeySigEvent st);
      void undoChangeClef(Staff* ostaff, Segment*, ClefType st);
      void undoChangeBarLine(Measure* m, BarLineType);
      void undoSwapCR(ChordRest* cr1, ChordRest* cr2);
      void undoChangeProperty(Element*, P_ID, const QVariant& v);

      void setGraceNote(Chord*,  int pitch, NoteType type, bool behind, int len);

      Segment* setNoteRest(Segment*, int track, NoteVal nval, Fraction, MScore::Direction stemDirection = MScore::AUTO);
      void changeCRlen(ChordRest* cr, const TDuration&);

      Fraction makeGap(Segment*, int track, const Fraction&, Tuplet*);
      bool makeGap1(int tick, int staffIdx, Fraction len);

      Rest* addRest(int tick, int track, TDuration, Tuplet*);
      Rest* addRest(Segment* seg, int track, TDuration d, Tuplet*);
      Chord* addChord(int tick, TDuration d, Chord* oc, bool genTie, Tuplet* tuplet);

      ChordRest* addClone(ChordRest* cr, int tick, const TDuration& d);
      Rest* setRest(int tick,  int track, Fraction, bool useDots, Tuplet* tuplet);

      void searchSelectedElements();

      void upDown(bool up, UpDownMode);
      ChordRest* searchNote(int tick, int track) const;

      // undo/redo ops
      void addArticulation(ArticulationType);
      void changeAccidental(Accidental::AccidentalType);
      void changeAccidental(Note* oNote, Accidental::AccidentalType);

      void addElement(Element*);
      void removeElement(Element*);

      void cmdAddSpanner(Spanner* e, const QPointF& pos, const QPointF& dragOffset);
      void cmdAddBSymbol(BSymbol*, const QPointF&, const QPointF&);

      Note* addNote(Chord*, int pitch);
      Note* addNote(Chord*, NoteVal &noteVal);

      void deleteItem(Element*);
      void cmdDeleteSelectedMeasures();
      void cmdDeleteSelection();

      void putNote(const QPointF& pos, bool replace);
      void putNote(const Position& pos, bool replace);
      void setInputState(Element* obj);

      void startCmd();        // start undoable command
      void endCmd();          // end undoable command
      void end();             // layout & update canvas
      void end1();
      void end2();
      void update();

      void cmdRemoveTimeSig(TimeSig*);
      void cmdAddTimeSig(Measure*, int staffIdx, TimeSig*, bool local);

      void setUpdateAll(bool v = true) { _updateAll = v;   }
      void setLayoutAll(bool val);
      bool layoutAll() const           { return _layoutAll; }
      void addRefresh(const QRectF& r) { refresh |= r;     }

      void changeVoice(int);

      void colorItem(Element*);
      QList<Part*>& parts()                { return _parts; }
      const QList<Part*>& parts() const    { return _parts; }
      Part* part(int n) const              { return _parts[n]; }
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

      bool saveFile(QFileInfo& info);
      void saveFile(QIODevice* f, bool msczFormat, bool onlySelection = false);
      void saveCompressedFile(QFileInfo&, bool onlySelection);
      void saveCompressedFile(QIODevice*, QFileInfo&, bool onlySelection);
      bool exportFile();

      void print(QPainter* printer, int page);
      ChordRest* getSelectedChordRest() const;
      void getSelectedChordRest2(ChordRest** cr1, ChordRest** cr2) const;

      void select(Element* obj, SelectType = SELECT_SINGLE, int staff = 0);
      void deselect(Element* obj);
      void deselectAll()                    { _selection.deselectAll(); }
      void updateSelection()                { _selection.update(); }
      Element* getSelectedElement() const   { return _selection.element(); }
      const Selection& selection() const    { return _selection; }
      Selection& selection()                { return _selection; }
      void setSelection(const Selection& s);

      int pos();
      Measure* tick2measure(int tick) const;
      MeasureBase* tick2measureBase(int tick) const;
      Segment* tick2segment(int tick, bool first = false, Segment::SegmentTypes st = Segment::SegAll) const;
      Segment* tick2segmentEnd(int track, int tick) const;
      void fixTicks();
      void addArticulation(Element*, Articulation* atr);

      bool playlistDirty();
      void setPlaylistDirty(bool val) { _playlistDirty = val; }

      void cmd(const QAction*);
      int fileDivision(int t) const { return (t * MScore::division + _fileDivision/2) / _fileDivision; }
      bool saveFile();

      QString filePath() const       { return info.filePath(); }
      QString absoluteFilePath() const { return info.absoluteFilePath(); }
      QFileInfo* fileInfo()          { return &info; }

      QString name() const           { return info.completeBaseName(); }
      void setName(const QString& s) { info.setFile(s); }

      bool isSavable() const;
      bool dirty() const             { return _dirty;         }
      void setCreated(bool val)      { _created = val;        }
      bool created() const           { return _created;       }
      bool saved() const             { return _saved;         }
      void setSaved(bool v)          { _saved = v;            }
      bool printing() const          { return _printing;      }
      void setPrinting(bool val)     { _printing = val;      }
      void setAutosaveDirty(bool v)  { _autosaveDirty = v;    }
      bool autosaveDirty() const     { return _autosaveDirty; }

      void spell();
      void spell(int startStaff, int endStaff, Segment* startSegment, Segment* endSegment);
      void spell(Note*);
      int nextSeg(int tick, int track);

      MStyle* style()                          { return &_style;                  }
      const MStyle* style() const              { return &_style;                  }
      void setStyle(const MStyle& s)           { _style = s;                      }
      bool loadStyle(const QString&);
      bool saveStyle(const QString&);

      StyleVal style(StyleIdx idx) const       { return _style.value(idx);   }
      Spatium styleS(StyleIdx idx) const       { return _style.valueS(idx);  }
      qreal   styleP(StyleIdx idx) const       { return _style.valueS(idx).val() * spatium();  }
      QString styleSt(StyleIdx idx) const      { return _style.valueSt(idx); }
      bool    styleB(StyleIdx idx) const       { return _style.valueB(idx);  }
      qreal  styleD(StyleIdx idx) const        { return _style.valueD(idx);  }
      int     styleI(StyleIdx idx) const       { return _style.valueI(idx);  }
      const TextStyle& textStyle(int idx) const { return _style.textStyle(idx); }
      const TextStyle& textStyle(const QString& s) const  { return _style.textStyle(s); }

      int playPos() const                      { return _playPos;    }
      void setPlayPos(int val)                 { _playPos = val;     }

      bool noteEntryMode() const               { return _is.noteEntryMode; }
      int inputPos() const;
      int inputTrack() const                   { return _is.track(); }
      InputState& inputState()                 { return _is;         }
      void setInputState(const InputState& st);
      void setInputTrack(int);

      void spatiumChanged(qreal oldValue, qreal newValue);

      void pasteStaff(const QDomElement&, ChordRest* dst);
      void toEList(EventMap* events);
      void renderPart(EventMap* events, Part*);
      int mscVersion() const    { return _mscVersion; }
      void setMscVersion(int v) { _mscVersion = v; }

      void addLyrics(int tick, int staffIdx, const QString&);

      QList<Excerpt*>& excerpts()             { return _excerpts; }
      const QList<Excerpt*>& excerpts() const { return _excerpts; }

      void setLayout(Measure* m);

      int midiPort(int idx) const;
      int midiChannel(int idx) const;
      QList<MidiMapping>* midiMapping()       { return &_midiMapping;          }
      MidiMapping* midiMapping(int channel)   { return &_midiMapping[channel]; }
      void rebuildMidiMapping();
      void updateChannel();

      void cmdTransposeStaff(int staffIdx, Interval, bool useDoubleSharpsFlats);
      void cmdConcertPitchChanged(bool, bool useSharpsFlats);

      TempoMap* tempomap() const;
      TimeSigMap* sigmap() const;

      void setTempo(Segment*, qreal);
      void setTempo(int tick, qreal bps);
      void removeTempo(int tick);
      void setPause(int tick, qreal seconds);
      qreal tempo(int tick) const;

      bool creditsRead() const                       { return _creditsRead;     }
      void setCreditsRead(bool val)                  { _creditsRead = val;      }
      bool defaultsRead() const                      { return _defaultsRead;    }
      void setDefaultsRead(bool b)                   { _defaultsRead = b;       }
      void addCreator(MusicXmlCreator* c)            { _creators.append(c);     }
      const MusicXmlCreator* getCreator(int i) const { return _creators.at(i);  }
      int numberOfCreators() const                   { return _creators.size(); }
      Text* getText(int subtype);

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
      void renumberMeasures();
      UndoStack* undo() const;
      void undo(UndoCommand* cmd) const;

      void endUndoRedo();
      Measure* searchLabel(const QString& s);
      RepeatList* repeatList() const;
      qreal utick2utime(int tick) const;
      int utime2utick(qreal utime) const;
      Q_INVOKABLE void updateRepeatList(bool expandRepeats);

      void nextInputPos(ChordRest* cr, bool);
      void cmdMirrorNoteHead();

      qreal spatium() const                    { return style()->spatium();    }
      void setSpatium(qreal v)                 { style()->setSpatium(v);       }
      const PageFormat* pageFormat() const     { return style()->pageFormat(); }
      void setPageFormat(const PageFormat& pf) { style()->setPageFormat(pf);   }
      qreal loWidth() const;
      qreal loHeight() const;

      const QList<Page*>& pages() const        { return _pages;                }
      QList<System*>* systems()                { return &_systems;             }

      MeasureBaseList* measures()             { return &_measures; }
      bool checkHasMeasures() const;
      MeasureBase* first() const;
      MeasureBase* last()  const;
      Q_INVOKABLE Measure* firstMeasure() const;
      Q_INVOKABLE Measure* lastMeasure() const;
      int measureIdx(MeasureBase*) const;
      MeasureBase* measure(int idx) const;

      Q_INVOKABLE Segment* firstSegment(Segment::SegmentTypes s = Segment::SegAll) const;
      Q_INVOKABLE Segment* lastSegment() const;

      void connectTies();

      void add(Element*);
      void remove(Element*);
      qreal point(const Spatium sp) const { return sp.val() * spatium(); }

      void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
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
      Note* addPitch(int pitch, bool addFlag);

      int customKeySigIdx(KeySig*) const;
      int addCustomKeySig(KeySig*);
      KeySig* customKeySig(int) const;
      KeySig* keySigFactory(const KeySigEvent&);
      void cmdPaste(MuseScoreView*);
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
      void enqueueMidiEvent(MidiInputEvent ev) { midiInputQueue.enqueue(ev); }

      Q_INVOKABLE void doLayout();
      void layoutSystems();
      void layoutLinear();
      void layoutPages();
      void layoutSystemsUndoRedo();
      void layoutPagesUndoRedo();
      Page* getEmptyPage();

      void layoutChords1(Segment* segment, int staffIdx);
      SyntiState& syntiState()                           { return _syntiState;         }
      void setSyntiState(const SyntiState& s);

      const QList<StaffType*>& staffTypes() const;
      QList<StaffType*>& staffTypes();
      void addStaffTypes(const QList<StaffType*>& tl);
      void replaceStaffTypes(const QList<StaffType*>&);

      void addLayoutFlags(LayoutFlags val)               { layoutFlags |= val; }
      int symIdx() const                                 { return _symIdx; }
      void updateHairpin(Hairpin*);       // add/modify hairpin to pitchOffset list
      void removeHairpin(Hairpin*);       // remove hairpin from pitchOffset list
      Volta* searchVolta(int tick) const;
      Score* parentScore() const    { return _parentScore; }
      void setParentScore(Score* s) { _parentScore = s;    }
      const Score* rootScore() const;
      Score* rootScore();
      void addExcerpt(Score*);
      void removeExcerpt(Score*);
      void createRevision();
      QByteArray readCompressedToBuffer();
      QByteArray readToBuffer();
      void writeSegments(Xml& xml, const Measure*, int strack, int etrack, Segment* first, Segment* last, bool);
      Spanner* findSpanner(int id) const;

      const QMap<QString, QString> metaTags() const           { return _metaTags; }
      QMap<QString, QString>& metaTags()                      { return _metaTags; }
      Q_INVOKABLE QString metaTag(const QString& s) const                 { return _metaTags.value(s);}
      Q_INVOKABLE void setMetaTag(const QString& tag, const QString& val) { _metaTags.insert(tag, val); }

      void updateNotes();
      void cmdUpdateNotes();
      void updateAccidentals(Measure* m, int staffIdx);
      QHash<int, LinkedElements*>& links();
      bool concertPitch() const { return styleB(ST_concertPitch); }
      void layoutFingering(Fingering*);
      void cmdSplitMeasure(ChordRest*);
      void cmdJoinMeasure(Measure*, Measure*);
      void timesigStretchChanged(TimeSig* ts, Measure* fm, int staffIdx);
      int pageNumberOffset() const          { return _pageNumberOffset; }
      void setPageNumberOffset(int v)       { _pageNumberOffset = v; }

      const QDate& creationDate() const     { return _creationDate;  }
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

      void transpose(int mode, TransposeDirection, int transposeKey, int transposeInterval,
         bool trKeys, bool transposeChordNames, bool useDoubleSharpsFlats);
      void addViewer(MuseScoreView* v)      { viewer.append(v);    }
      void removeViewer(MuseScoreView* v)   { viewer.removeAll(v); }
      const QList<MuseScoreView*>& getViewer() const { return viewer;       }
      void moveCursor();
      bool playNote() const                 { return _playNote; }
      void setPlayNote(bool v)              { _playNote = v;    }
      bool excerptsChanged() const          { return _excerptsChanged; }
      void setExcerptsChanged(bool val)     { _excerptsChanged = val; }
      bool instrumentsChanged() const       { return _instrumentsChanged; }
      void setInstrumentsChanged(bool val)  { _instrumentsChanged = val; }
      bool selectionChanged() const         { return _selectionChanged; }
      void setSelectionChanged(bool val)    { _selectionChanged = val;  }

      LayoutMode layoutMode() const         { return _layoutMode; }
      void setLayoutMode(LayoutMode lm);

      QReadWriteLock* layoutLock() { return &_layoutLock; }
      void doLayoutSystems();
      void doLayoutPages();
      Tuplet* searchTuplet(const QDomElement& e, int id);
      void cmdSelectAll();
      void cmdSelectSection();
      void setUndoRedo(bool val)            { _undoRedo = val; }
      bool undoRedo() const                 { return _undoRedo; }
      void respace(QList<ChordRest*>* elements);
      void transposeSemitone(int semitone);
      MeasureBase* insertMeasure(Element::ElementType type, MeasureBase*,
         bool createEmptyMeasures = false);
      bool testMode() const        { return _testMode; }
      void setTestMode(bool val);
      Audio* audio() const         { return _audio;    }
      void setAudio(Audio* a)      { _audio = a;       }
      PlayMode playMode() const    { return _playMode; }
      void setPlayMode(PlayMode v) { _playMode = v;    }
      int linkId();
      void linkId(int);
      QList<Score*> scoreList();
      bool switchLayer(const QString& s);
      void layoutPage(const PageContext&,  qreal);
      Q_INVOKABLE void appendPart(const QString&);
      Q_INVOKABLE void appendMeasures(int);
      Q_INVOKABLE void addText(const QString&, const QString&);
      Q_INVOKABLE Cursor* newCursor();
      qreal computeMinWidth(Segment* fs) const;
      };

extern Score* gscore;
extern void fixTicks();

#endif

