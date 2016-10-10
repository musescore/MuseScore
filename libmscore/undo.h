//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __UNDO_H__
#define __UNDO_H__

/**
 \file
 Definition of undo-releated classes and structs.
*/

#include "spatium.h"
#include "mscore.h"
#include "sig.h"
#include "tempo.h"
#include "input.h"
#include "style.h"
#include "key.h"
#include "select.h"
#include "instrument.h"
#include "synthesizer/midipatch.h"
#include "pitchvalue.h"
#include "timesig.h"
#include "noteevent.h"
#include "synthesizerstate.h"
#include "bracket.h"
#include "dynamic.h"
#include "staff.h"
#include "stafftype.h"
#include "cleflist.h"
#include "note.h"
#include "drumset.h"
#include "rest.h"

Q_DECLARE_LOGGING_CATEGORY(undoRedo)

namespace Ms {

class ElementList;
class Element;
class Instrument;
class System;
class Measure;
class Segment;
class Staff;
class Part;
class Volta;
class Score;
class Note;
class Chord;
class ChordRest;
class Harmony;
class SlurTie;
struct MStaff;
class MeasureBase;
class Dynamic;
class Selection;
class Text;
struct Channel;
class PageFormat;
class TextStyle;
class Tuplet;
class KeySig;
class TimeSig;
class Clef;
class Image;
class Bend;
class TremoloBar;
class NoteEvent;
class SlurSegment;
class InstrumentChange;
class Box;
class Spanner;
class BarLine;
enum class ClefType : signed char;
enum class PlayEventType : char;

#define UNDO_NAME(a)  virtual const char* name() const override { return a; }

enum class LayoutMode : char;

//---------------------------------------------------------
//   UndoCommand
//---------------------------------------------------------

class UndoCommand {
      QList<UndoCommand*> childList;

   protected:
      virtual void flip() {}

   public:
      virtual ~UndoCommand();
      virtual void undo();
      virtual void redo();
      void appendChild(UndoCommand* cmd) { childList.append(cmd);       }
      UndoCommand* removeChild()         { return childList.takeLast(); }
      int childCount() const             { return childList.size();     }
      void unwind();
      virtual void cleanup(bool undo);
// #ifndef QT_NO_DEBUG
      virtual const char* name() const { return "UndoCommand"; }
// #endif
      };

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

class UndoStack {
      UndoCommand* curCmd;
      QList<UndoCommand*> list;
      int curIdx;
      int cleanIdx;

   public:
      UndoStack();
      ~UndoStack();

      bool active() const           { return curCmd != 0; }
      void beginMacro();
      void endMacro(bool rollback);
      void push(UndoCommand*);      // push & execute
      void push1(UndoCommand*);
      void pop();
      void setClean();
      bool canUndo() const          { return curIdx > 0;           }
      bool canRedo() const          { return curIdx < list.size(); }
      bool isClean() const          { return cleanIdx == curIdx;   }
      bool empty() const          { return !canUndo() && !canRedo();  }
      UndoCommand* current() const  { return curCmd;               }
      void undo();
      void redo();
      };

//---------------------------------------------------------
//   SaveState
//---------------------------------------------------------

class SaveState : public UndoCommand {
      InputState undoInputState;
      InputState redoInputState;
      Selection  undoSelection;
      Selection  redoSelection;
      Score* score;

   public:
      SaveState(Score*);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("SaveState")
      };

//---------------------------------------------------------
//   InsertPart
//---------------------------------------------------------

class InsertPart : public UndoCommand {
      Part* part;
      int idx;

   public:
      InsertPart(Part* p, int i);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertPart")
      };

//---------------------------------------------------------
//   RemovePart
//---------------------------------------------------------

class RemovePart : public UndoCommand {
      Part* part;
      int idx;

   public:
      RemovePart(Part*, int idx);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemovePart")
      };

//---------------------------------------------------------
//   InsertStaff
//---------------------------------------------------------

class InsertStaff : public UndoCommand {
      Staff* staff;
      int ridx;

   public:
      InsertStaff(Staff*, int idx);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertStaff")
      };

//---------------------------------------------------------
//   RemoveStaff
//---------------------------------------------------------

class RemoveStaff : public UndoCommand {
      Staff* staff;
      int ridx;

   public:
      RemoveStaff(Staff*);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveStaff")
      };

//---------------------------------------------------------
//   InsertMStaff
//---------------------------------------------------------

class InsertMStaff : public UndoCommand {
      Measure* measure;
      MStaff* mstaff;
      int idx;

   public:
      InsertMStaff(Measure*, MStaff*, int);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertMStaff")
      };

//---------------------------------------------------------
//   RemoveMStaff
//---------------------------------------------------------

class RemoveMStaff : public UndoCommand {
      Measure* measure;
      MStaff* mstaff;
      int idx;

   public:
      RemoveMStaff(Measure*, MStaff*, int);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveMStaff")
      };

//---------------------------------------------------------
//   InsertStaves
//---------------------------------------------------------

class InsertStaves : public UndoCommand {
      Measure* measure;
      int a;
      int b;

   public:
      InsertStaves(Measure*, int, int);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertStaves")
      };

//---------------------------------------------------------
//   RemoveStaves
//---------------------------------------------------------

class RemoveStaves : public UndoCommand {
      Measure* measure;
      int a;
      int b;

   public:
      RemoveStaves(Measure*, int, int);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveStaves")
      };

//---------------------------------------------------------
//   SortStaves
//---------------------------------------------------------

class SortStaves : public UndoCommand {
      Score* score;
      QList<int> list;
      QList<int> rlist;

   public:
      SortStaves(Score*, QList<int>);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("SortStaves")
      };

//---------------------------------------------------------
//   ChangePitch
//---------------------------------------------------------

class ChangePitch : public UndoCommand {
      Note* note;
      int pitch;
      int tpc1;
      int tpc2;
      void flip();

   public:
      ChangePitch(Note* note, int pitch, int tpc1, int tpc2);
      UNDO_NAME("ChangePitch")
      };

//---------------------------------------------------------
//   ChangeFretting
//---------------------------------------------------------

class ChangeFretting : public UndoCommand {
      Note* note;
      int pitch;
      int string;
      int fret;
      int tpc1;
      int tpc2;
      void flip();

   public:
      ChangeFretting(Note* note, int pitch, int string, int fret, int tpc1, int tpc2);
      UNDO_NAME("ChangeFretting")
      };

//---------------------------------------------------------
//   ChangeKeySig
//---------------------------------------------------------

class ChangeKeySig : public UndoCommand {
      KeySig* keysig;
      KeySigEvent ks;
      bool showCourtesy;

      void flip();

   public:
      ChangeKeySig(KeySig* k, KeySigEvent newKeySig, bool sc) : keysig(k), ks(newKeySig), showCourtesy(sc) {}
      UNDO_NAME("ChangeKeySig")
      };

//---------------------------------------------------------
//   ChangeMeasureLen
//---------------------------------------------------------

class ChangeMeasureLen : public UndoCommand {
      Measure* measure;
      Fraction len;
      void flip();

   public:
      ChangeMeasureLen(Measure*, Fraction);
      UNDO_NAME("ChangeMeasureLen")
      };

//---------------------------------------------------------
//   ChangeElement
//---------------------------------------------------------

class ChangeElement : public UndoCommand {
      Element* oldElement;
      Element* newElement;
      void flip();

   public:
      ChangeElement(Element* oldElement, Element* newElement);
      UNDO_NAME("ChangeElement")
      };

//---------------------------------------------------------
//   ChangeBarLineSpan
//---------------------------------------------------------

class ChangeBarLineSpan : public UndoCommand {
      Staff* staff;
      int span;
      int spanFrom;
      int spanTo;
      void flip();

   public:
      ChangeBarLineSpan(Staff*, int span, int spanFrom, int spanTo);
      UNDO_NAME("ChangeBarLineSpan")
      };

//---------------------------------------------------------
//   ChangeSingleBarLineSpan
//---------------------------------------------------------

class ChangeSingleBarLineSpan : public UndoCommand {
      BarLine* barLine;
      int span;
      int spanFrom;
      int spanTo;
      void flip();

   public:
      ChangeSingleBarLineSpan(BarLine* barLine, int span, int spanFrom, int spanTo);
      UNDO_NAME("ChangeSingleBarLineSpan")
      };

//---------------------------------------------------------
//   TransposeHarmony
//---------------------------------------------------------

class TransposeHarmony : public UndoCommand {
      Harmony* harmony;
      int rootTpc, baseTpc;
      void flip();

   public:
      TransposeHarmony(Harmony*, int rootTpc, int baseTpc);
      UNDO_NAME("TransposeHarmony")
      };

//---------------------------------------------------------
//   ExchangeVoice
//---------------------------------------------------------

class ExchangeVoice : public UndoCommand {
      Measure* measure;
      int val1, val2;
      int staff;

   public:
      ExchangeVoice(Measure* ,int val1, int val2, int staff);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("ExchangeVoice")
      };

//---------------------------------------------------------
//   CloneVoice
//---------------------------------------------------------

class CloneVoice : public UndoCommand {
      Segment* sf;
      int lTick;
      Segment* d;             //Destination
      int strack, dtrack;
      int otrack;
      bool linked;
      bool first = true;      //first redo

   public:
      CloneVoice(Segment* sf, int lTick, Segment* d, int strack, int dtrack, int otrack, bool linked = true);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("CloneVoice")
      };

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

class ChangeInstrumentShort : public UndoCommand {
      Part* part;
      int tick;
      QList<StaffName> text;
      void flip();

   public:
      ChangeInstrumentShort(int, Part*, QList<StaffName>);
      UNDO_NAME("ChangeInstrumentShort")
      };

//---------------------------------------------------------
//   ChangeInstrumentLong
//---------------------------------------------------------

class ChangeInstrumentLong : public UndoCommand {
      Part* part;
      int tick;
      QList<StaffName> text;
      void flip();

   public:
      const QList<StaffName>& longNames() const;
      ChangeInstrumentLong(int, Part*, QList<StaffName>);
      UNDO_NAME("ChangeInstrumentLong")
      };

//---------------------------------------------------------
//   MoveElement
//---------------------------------------------------------

class MoveElement : public UndoCommand {
      Element* element;
      QPointF offset;
      void flip();

   public:
      MoveElement(Element*, const QPointF&);
      UNDO_NAME("MoveElement")
      };

//---------------------------------------------------------
//   ChangeBracketSpan
//---------------------------------------------------------

class ChangeBracketSpan : public UndoCommand {
      Staff* staff;
      int column;
      int span;
      void flip();

   public:
      ChangeBracketSpan(Staff*, int column, int span);
      UNDO_NAME("ChangeBracketSpan")
      };

//---------------------------------------------------------
//   ChangeBracketType
//---------------------------------------------------------

class ChangeBracketType : public UndoCommand {
      Bracket* bracket;
      BracketType type;
      void flip();

   public:
      ChangeBracketType(Bracket*, BracketType type);
      UNDO_NAME("ChangeBracketType")
      };

//---------------------------------------------------------
//   AddElement
//---------------------------------------------------------

class AddElement : public UndoCommand {
      Element* element;

      void endUndoRedo(bool) const;

   public:
      AddElement(Element*);
      virtual void undo();
      virtual void redo();
      virtual void cleanup(bool);
      virtual const char* name() const override;
      };

//---------------------------------------------------------
//   RemoveElement
//---------------------------------------------------------

class RemoveElement : public UndoCommand {
      Element* element;

   public:
      RemoveElement(Element*);
      virtual void undo();
      virtual void redo();
      virtual void cleanup(bool);
      virtual const char* name() const override;
      };

//---------------------------------------------------------
//   EditText
//---------------------------------------------------------

class EditText : public UndoCommand {
      Text* text;
      QString oldText;
      //int undoLevel;

      void undoRedo();

   public:
      EditText(Text* t, const QString& ot, int /*l*/) : text(t), oldText(ot)/*, undoLevel(l)*/ {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("EditText")
      };

//---------------------------------------------------------
//   ChangePatch
//---------------------------------------------------------

class ChangePatch : public UndoCommand {
      Score* score;
      Channel* channel;
      MidiPatch patch;

      void flip();

   public:
      ChangePatch(Score* s, Channel* c, const MidiPatch* pt)
         : score(s), channel(c), patch(*pt) {}
      UNDO_NAME("ChangePatch")
      };

//---------------------------------------------------------
//   ChangePageFormat
//---------------------------------------------------------

class ChangePageFormat : public UndoCommand {
      Score* score;
      PageFormat* pf;
      qreal spatium;
      int pageOffset;

      void flip();

   public:
      ChangePageFormat(Score*, PageFormat*, qreal sp, int po);
      ~ChangePageFormat();
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangePageFormat")
      };

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

class ChangeStaff : public UndoCommand {
      Staff*   staff;
      bool     invisible;
      qreal    userDist;
      Staff::HideMode hideMode;
      bool     showIfEmpty;
      bool     cutaway;
      bool     hideSystemBarLine;

      void flip();

   public:
      ChangeStaff(Staff*, bool invisible, qreal userDist, Staff::HideMode _hideMode,
         bool _showIfEmpty, bool _cutaway, bool hide);
      UNDO_NAME("ChangeStaff")
      };

//---------------------------------------------------------
//   ChangeStaffType
//---------------------------------------------------------

class ChangeStaffType : public UndoCommand {
      Staff*       staff;
      StaffType    staffType;

      void flip();

   public:
      ChangeStaffType(Staff* s, const StaffType& t) : staff(s), staffType(t) {}
      UNDO_NAME("ChangeStaffType")
      };

//---------------------------------------------------------
//   ChangePart
//---------------------------------------------------------

class ChangePart : public UndoCommand {
      Part* part;
      Instrument* instrument;
      QString partName;

      void flip();

   public:
      ChangePart(Part*, Instrument*, const QString& name);
      UNDO_NAME("ChangePart")
      };

//---------------------------------------------------------
//   ChangeTextStyle
//---------------------------------------------------------

class ChangeTextStyle : public UndoCommand {
      Score* score;
      TextStyle style;
      void flip();

   public:
      ChangeTextStyle(Score*, const TextStyle& style);
      UNDO_NAME("ChangeTextStyle")
      };

//---------------------------------------------------------
//   AddTextStyle
//---------------------------------------------------------

class AddTextStyle : public UndoCommand {
      Score* score;
      TextStyle style;

   public:
      AddTextStyle(Score* s, const TextStyle& st) : score(s), style(st) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("AddTextStyle")
      };

//---------------------------------------------------------
//   ChangeStyle
//---------------------------------------------------------

class ChangeStyle : public UndoCommand {
      Score* score;
      MStyle style;
      void flip();

   public:
      ChangeStyle(Score*, const MStyle&);
      UNDO_NAME("ChangeStyle")
      };

//---------------------------------------------------------
//   ChangeStyleVal
//---------------------------------------------------------

class ChangeStyleVal : public UndoCommand {
      Score* score;
      StyleIdx idx;
      QVariant value;

      void flip();

   public:
      ChangeStyleVal(Score* s, StyleIdx i, const QVariant& v) : score(s), idx(i), value(v) {}
      UNDO_NAME("ChangeStyleVal")
      };

//---------------------------------------------------------
//   ChangeChordStaffMove
//---------------------------------------------------------

class ChangeChordStaffMove : public UndoCommand {
      ChordRest* chordRest;
      int staffMove;
      void flip();

   public:
      ChangeChordStaffMove(ChordRest* cr, int);
      UNDO_NAME("ChangeChordStaffMove")
      };

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

class ChangeVelocity : public UndoCommand {
      Note* note;
      Note::ValueType veloType;
      int veloOffset;
      void flip();

   public:
      ChangeVelocity(Note*, Note::ValueType, int);
      UNDO_NAME("ChangeVelocity")
      };

//---------------------------------------------------------
//   ChangeMStaffProperties
//---------------------------------------------------------

class ChangeMStaffProperties : public UndoCommand {
      MStaff* mstaff;
      bool visible;
      bool slashStyle;
      void flip();

   public:
      ChangeMStaffProperties(MStaff*, bool visible, bool slashStyle);
      UNDO_NAME("ChangeMStaffProperties")
      };

//---------------------------------------------------------
//   InsertRemoveMeasures
//---------------------------------------------------------

class InsertRemoveMeasures : public UndoCommand {
      MeasureBase* fm;
      MeasureBase* lm;

   protected:
      void removeMeasures();
      void insertMeasures();

   public:
      InsertRemoveMeasures(MeasureBase* _fm, MeasureBase* _lm) : fm(_fm), lm(_lm) {}
      virtual void undo() = 0;
      virtual void redo() = 0;
      };

//---------------------------------------------------------
//   RemoveMeasures
//---------------------------------------------------------

class RemoveMeasures : public InsertRemoveMeasures {

   public:
      RemoveMeasures(MeasureBase* m1, MeasureBase* m2) : InsertRemoveMeasures(m1, m2) {}
      virtual void undo() { insertMeasures(); }
      virtual void redo() { removeMeasures(); }
      UNDO_NAME("RemoveMeasures")
      };

//---------------------------------------------------------
//   InsertMeasures
//---------------------------------------------------------

class InsertMeasures : public InsertRemoveMeasures {

   public:
      InsertMeasures(MeasureBase* m1, MeasureBase* m2) : InsertRemoveMeasures(m1, m2) {}
      virtual void redo() { insertMeasures(); }
      virtual void undo() { removeMeasures(); }
      UNDO_NAME("InsertMeasures")
      };

//---------------------------------------------------------
//   ChangeImage
//---------------------------------------------------------

class ChangeImage : public UndoCommand {
      Image* image;
      bool lockAspectRatio;
      bool autoScale;
      int z;

      void flip();

   public:
      ChangeImage(Image* i, bool l, bool a, int _z)
         : image(i), lockAspectRatio(l), autoScale(a), z(_z) {}
      UNDO_NAME("ChangeImage")
      };

//---------------------------------------------------------
//   AddExcerpt
//---------------------------------------------------------

class AddExcerpt : public UndoCommand {
      Excerpt* excerpt;

   public:
      AddExcerpt(Excerpt* ex) : excerpt(ex) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("AddExcerpt")
      };

//---------------------------------------------------------
//   RemoveExcerpt
//---------------------------------------------------------

class RemoveExcerpt : public UndoCommand {
      Excerpt* excerpt;

   public:
      RemoveExcerpt(Excerpt* ex) : excerpt(ex) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveExcerpt")
      };

//---------------------------------------------------------
//   SwapExcerpt
//---------------------------------------------------------

class SwapExcerpt : public UndoCommand {
      MasterScore* score;
      int pos1;
      int pos2;

      void flip();

   public:
      SwapExcerpt(MasterScore* s, int p1, int p2) : score(s), pos1(p1), pos2(p2) {}
      UNDO_NAME("SwapExcerpt")
      };

//---------------------------------------------------------
//   ChangeExcerptTitle
//---------------------------------------------------------

class ChangeExcerptTitle : public UndoCommand {
      Excerpt* excerpt;
      QString title;

      void flip();

   public:
      ChangeExcerptTitle(Excerpt* x, const QString& t) : excerpt(x), title(t) {}
      UNDO_NAME("ChangeExcerptTitle")
      };

//---------------------------------------------------------
//   ChangeBend
//---------------------------------------------------------

class ChangeBend : public UndoCommand {
      Bend* bend;
      QList<PitchValue> points;

      void flip();

   public:
      ChangeBend(Bend* b, QList<PitchValue> p) : bend(b), points(p) {}
      UNDO_NAME("ChangeBend")
      };

//---------------------------------------------------------
//   ChangeTremoloBar
//---------------------------------------------------------

class ChangeTremoloBar : public UndoCommand {
      TremoloBar* bend;
      QList<PitchValue> points;

      void flip();

   public:
      ChangeTremoloBar(TremoloBar* b, QList<PitchValue> p) : bend(b), points(p) {}
      UNDO_NAME("ChangeTremoloBar")
      };

//---------------------------------------------------------
//   ChangeNoteEvents
//---------------------------------------------------------

class ChangeNoteEvents : public UndoCommand {
      //Chord* chord;
      QList<NoteEvent*> events;

      void flip();

   public:
      ChangeNoteEvents(Chord* /*n*/, const QList<NoteEvent*>& l) : /*chord(n),*/ events(l) {}
      UNDO_NAME("ChangeNoteEvents")
      };

//---------------------------------------------------------
//   ChangeInstrument
//    change instrument in an InstrumentChange element
//---------------------------------------------------------

class ChangeInstrument : public UndoCommand {
      InstrumentChange* is;
      Instrument* instrument;

      void flip();

   public:
      ChangeInstrument(InstrumentChange* _is, Instrument* i) : is(_is), instrument(i) {}
      UNDO_NAME("ChangeInstrument")
      };

extern void updateNoteLines(Segment*, int track);

//---------------------------------------------------------
//   SwapCR
//---------------------------------------------------------

class SwapCR : public UndoCommand {
      ChordRest* cr1;
      ChordRest* cr2;

      void flip();

   public:
      SwapCR(ChordRest* a, ChordRest* b) : cr1(a), cr2(b) {}
      UNDO_NAME("SwapCR")
      };

//---------------------------------------------------------
//   ChangeClefType
//---------------------------------------------------------

class ChangeClefType : public UndoCommand {
      Clef* clef;
      ClefType concertClef;
      ClefType transposingClef;
      void flip();

   public:
      ChangeClefType(Clef*, ClefType cl, ClefType tc);
      UNDO_NAME("ChangeClef")
      };

//---------------------------------------------------------
//   MoveStaff
//---------------------------------------------------------

class MoveStaff : public UndoCommand {
      Staff* staff;
      Part* part;
      int rstaff;

      void flip();

   public:
      MoveStaff(Staff* s, Part* p, int idx) : staff(s), part(p), rstaff(idx) {}
      UNDO_NAME("MoveStaff")
      };

//---------------------------------------------------------
//   ChangeStaffUserDist
//---------------------------------------------------------

class ChangeStaffUserDist : public UndoCommand {
      Staff* staff;
      qreal dist;

      void flip();

   public:
      ChangeStaffUserDist(Staff* s, qreal d)
         : staff(s), dist(d) {}
      UNDO_NAME("ChangeStaffUserDist")
      };

//---------------------------------------------------------
//   ChangeProperty
//---------------------------------------------------------

class ChangeProperty : public UndoCommand {
      ScoreElement* element;
      P_ID id;
      QVariant property;
      PropertyStyle propertyStyle;

      void flip();

   public:
      ChangeProperty(ScoreElement* e, P_ID i, const QVariant& v, PropertyStyle ps = PropertyStyle::NOSTYLE)
         : element(e), id(i), property(v), propertyStyle(ps) {}
      P_ID getId() const  { return id; }
      UNDO_NAME("ChangeProperty")
      };

//---------------------------------------------------------
//   ChangeMetaText
//---------------------------------------------------------

class ChangeMetaText : public UndoCommand {
      Score* score;
      QString id;
      QString text;

      void flip();

   public:
      ChangeMetaText(Score* s, const QString& i, const QString& t) : score(s), id(i), text(t) {}
      UNDO_NAME("ChangeMetaText")
      };

//---------------------------------------------------------
//   ChangeEventList
//---------------------------------------------------------

class ChangeEventList : public UndoCommand {
      Chord* chord;
      QList<NoteEventList> events;
      PlayEventType eventListType;

      void flip();

   public:
      ChangeEventList(Chord* c, const QList<NoteEventList> l);
      UNDO_NAME("ChangeEventList")
      };

//---------------------------------------------------------
//   ChangeSynthesizerState
//---------------------------------------------------------

class ChangeSynthesizerState : public UndoCommand {
      Score* score;
      SynthesizerState state;

      void flip();

   public:
      ChangeSynthesizerState(Score* s, const SynthesizerState& st) : score(s), state(st) {}
      UNDO_NAME("ChangeSynthesizerState")
      };

//---------------------------------------------------------
//   RemoveBracket
//---------------------------------------------------------

class RemoveBracket : public UndoCommand {
      Staff* staff;
      int level;
      BracketType type;
      int span;

      virtual void undo();
      virtual void redo();

   public:
      RemoveBracket(Staff* s, int l, BracketType t, int sp) : staff(s), level(l), type(t), span(sp) {}
      UNDO_NAME("RemoveBracket")
      };

//---------------------------------------------------------
//   AddBracket
//---------------------------------------------------------

class AddBracket : public UndoCommand {
      Staff* staff;
      int level;
      BracketType type;
      int span;

      virtual void undo();
      virtual void redo();

   public:
      AddBracket(Staff* s, int l, BracketType t, int sp) : staff(s), level(l), type(t), span(sp) {}
      UNDO_NAME("AddBracket")
      };

//---------------------------------------------------------
//   ChangeSpannerElements
//---------------------------------------------------------

class ChangeSpannerElements : public UndoCommand {
      Spanner* spanner;
      Element* startElement;
      Element* endElement;

      void flip();

   public:
      ChangeSpannerElements(Spanner* s, Element* se, Element* ee)
         : spanner(s), startElement(se), endElement(ee) {}
      UNDO_NAME("ChangeSpannerElements")
      };

//---------------------------------------------------------
//   ChangeParent
//---------------------------------------------------------

class ChangeParent : public UndoCommand {
      Element* element;
      Element* parent;
      int staffIdx;

      void flip();

   public:
      ChangeParent(Element* e, Element* p, int si) : element(e), parent(p), staffIdx(si) {}
      UNDO_NAME("ChangeParent")
      };

//---------------------------------------------------------
//   ChangeMMRest
//---------------------------------------------------------

class ChangeMMRest : public UndoCommand {
      Measure* m;
      Measure* mmrest;

      void flip();

   public:
      ChangeMMRest(Measure* _m, Measure* _mmr) : m(_m), mmrest(_mmr) {}
      UNDO_NAME("ChangeMMRest")
      };

//---------------------------------------------------------
//   InsertTime
//---------------------------------------------------------

class InsertTime : public UndoCommand {
      Score* score;
      int tick;
      int len;

      void redo();
      void undo();

   public:
      InsertTime(Score* _score, int _tick, int _len)
         : score(_score), tick(_tick), len(_len) {}
      UNDO_NAME("InsertTime")
      };

//---------------------------------------------------------
//   ChangeNoteEvent
//---------------------------------------------------------

class ChangeNoteEvent : public UndoCommand {
      Note* note;
      NoteEvent* oldEvent;
      NoteEvent newEvent;

      void flip();

   public:
      ChangeNoteEvent(Note* n, NoteEvent* oe, const NoteEvent& ne)
         : note(n), oldEvent(oe), newEvent(ne) {}
      UNDO_NAME("ChangeNoteEvent")
      };

//---------------------------------------------------------
//   LinkUnlink
//---------------------------------------------------------

class LinkUnlink : public UndoCommand {
      ScoreElement* e;
      ScoreElement* le;

   protected:
      void doLink();
      void doUnlink();

   public:
      LinkUnlink(ScoreElement* _e, ScoreElement* _le) : e(_e), le(_le) {}
      };

//---------------------------------------------------------
//   Unlink
//---------------------------------------------------------

class Unlink : public LinkUnlink {

   public:
      Unlink(ScoreElement* _e) : LinkUnlink(_e, 0) {}
      virtual void undo() override { doLink();   }
      virtual void redo() override { doUnlink(); }
      UNDO_NAME("Unlink")
      };

//---------------------------------------------------------
//   Link
//---------------------------------------------------------

class Link : public LinkUnlink {

   public:
      Link(ScoreElement* e, ScoreElement* le) : LinkUnlink(e, le) {}
      virtual void undo() override { doUnlink(); }
      virtual void redo() override { doLink();   }
      UNDO_NAME("Link")
      };

//---------------------------------------------------------
//   LinkStaff
//---------------------------------------------------------

class LinkStaff : public UndoCommand {
      Staff* s1;
      Staff* s2;

   public:
      LinkStaff(Staff* _s1, Staff* _s2) : s1(_s1), s2(_s2) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("LinkStaff")
      };

//---------------------------------------------------------
//   UnlinkStaff
//---------------------------------------------------------

class UnlinkStaff : public UndoCommand {
      Staff* s1;
      Staff* s2;

   public:
      UnlinkStaff(Staff* _s1, Staff* _s2) : s1(_s1), s2(_s2) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("UnlinkStaff")
      };

//---------------------------------------------------------
//   ChangeStartEndSpanner
//---------------------------------------------------------

class ChangeStartEndSpanner : public UndoCommand {
      Spanner* spanner;
      Element* start;
      Element* end;

      void flip();

   public:
      ChangeStartEndSpanner(Spanner* sp, Element*s, Element*e) : spanner(sp), start(s), end(e) {}
      UNDO_NAME("ChangeStartEndSpanner")
      };

//---------------------------------------------------------
//   ChangeMetaTags
//---------------------------------------------------------

class ChangeMetaTags : public UndoCommand {
      Score* score;
      QMap<QString,QString> metaTags;

      void flip();

   public:
      ChangeMetaTags(Score* s, const QMap<QString,QString>& m) : score(s), metaTags(m) {}
      UNDO_NAME("ChangeMetaTags")
      };

//---------------------------------------------------------
//   ChangeDrumset
//---------------------------------------------------------

class ChangeDrumset : public UndoCommand {
      Instrument* instrument;
      Drumset drumset;

      void flip();

   public:
      ChangeDrumset(Instrument* i, const Drumset* d) : instrument(i), drumset(*d) {}
      UNDO_NAME("ChangeDrumset")
      };

//---------------------------------------------------------
//   ChangeGap
//---------------------------------------------------------

class ChangeGap : public UndoCommand {
      Rest* rest;
      bool v;

      void flip();

   public:
      ChangeGap(Rest* r, bool v) : rest(r), v(v) {}
      UNDO_NAME("ChangeGap")
      };

}     // namespace Ms
#endif
