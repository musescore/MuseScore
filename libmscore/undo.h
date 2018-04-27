//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
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
class MStaff;
class MeasureBase;
class Dynamic;
class Selection;
class Text;
struct Channel;
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
class Excerpt;
class EditData;

#define UNDO_NAME(a)  virtual const char* name() const override { return a; }

enum class LayoutMode : char;

//---------------------------------------------------------
//   UndoCommand
//---------------------------------------------------------

class UndoCommand {
      QList<UndoCommand*> childList;

   protected:
      virtual void flip(EditData*) {}

   public:
      virtual ~UndoCommand();
      virtual void undo(EditData*);
      virtual void redo(EditData*);
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
      void push(UndoCommand*, EditData*);      // push & execute
      void push1(UndoCommand*);
      void pop();
      void setClean();
      bool canUndo() const          { return curIdx > 0;           }
      bool canRedo() const          { return curIdx < list.size(); }
      bool isClean() const          { return cleanIdx == curIdx;   }
      bool empty() const            { return !canUndo() && !canRedo();  }
      UndoCommand* current() const  { return curCmd;               }
      void undo(EditData*);
      void redo(EditData*);
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      void flip(EditData*) override;

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
      void flip(EditData*) override;

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

      void flip(EditData*) override;

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
      void flip(EditData*) override;

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
      void flip(EditData*) override;

   public:
      ChangeElement(Element* oldElement, Element* newElement);
      UNDO_NAME("ChangeElement")
      };

//---------------------------------------------------------
//   TransposeHarmony
//---------------------------------------------------------

class TransposeHarmony : public UndoCommand {
      Harmony* harmony;
      int rootTpc, baseTpc;
      void flip(EditData*) override;

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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
      UNDO_NAME("CloneVoice")
      };

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

class ChangeInstrumentShort : public UndoCommand {
      Part* part;
      int tick;
      QList<StaffName> text;
      void flip(EditData*) override;

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
      void flip(EditData*) override;

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
      void flip(EditData*) override;

   public:
      MoveElement(Element*, const QPointF&);
      UNDO_NAME("MoveElement")
      };

//---------------------------------------------------------
//   ChangeBracketType
//---------------------------------------------------------

class ChangeBracketType : public UndoCommand {
      Bracket* bracket;
      BracketType type;
      void flip(EditData*) override;

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
      void undo(EditData*) override;
      void redo(EditData*) override;

   public:
      AddElement(Element*);
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
      UNDO_NAME("EditText")
      };

//---------------------------------------------------------
//   ChangePatch
//---------------------------------------------------------

class ChangePatch : public UndoCommand {
      Score* score;
      Channel* channel;
      MidiPatch patch;

      void flip(EditData*) override;

   public:
      ChangePatch(Score* s, Channel* c, const MidiPatch* pt)
         : score(s), channel(c), patch(*pt) {}
      UNDO_NAME("ChangePatch")
      };

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

class ChangeStaff : public UndoCommand {
      Staff*   staff;
      bool     invisible;
      ClefTypeList clefType;
      qreal    userDist;
      Staff::HideMode hideMode;
      bool     showIfEmpty;
      bool     cutaway;
      bool     hideSystemBarLine;

      void flip(EditData*) override;

   public:
      ChangeStaff(Staff*, bool invisible, ClefTypeList _clefType, qreal userDist, Staff::HideMode _hideMode,
         bool _showIfEmpty, bool _cutaway, bool hide);
      UNDO_NAME("ChangeStaff")
      };

//---------------------------------------------------------
//   ChangeStaffType
//---------------------------------------------------------

class ChangeStaffType : public UndoCommand {
      Staff*       staff;
      StaffType    staffType;

      void flip(EditData*) override;

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

      void flip(EditData*) override;

   public:
      ChangePart(Part*, Instrument*, const QString& name);
      UNDO_NAME("ChangePart")
      };

//---------------------------------------------------------
//   ChangeStyle
//---------------------------------------------------------

class ChangeStyle : public UndoCommand {
      Score* score;
      MStyle style;
      void flip(EditData*) override;

   public:
      ChangeStyle(Score*, const MStyle&);
      UNDO_NAME("ChangeStyle")
      };

//---------------------------------------------------------
//   ChangeStyleVal
//---------------------------------------------------------

class ChangeStyleVal : public UndoCommand {
      Score* score;
      Sid idx;
      QVariant value;

      void flip(EditData*) override;

   public:
      ChangeStyleVal(Score* s, Sid i, const QVariant& v) : score(s), idx(i), value(v) {}
      UNDO_NAME("ChangeStyleVal")
      };

//---------------------------------------------------------
//   ChangeChordStaffMove
//---------------------------------------------------------

class ChangeChordStaffMove : public UndoCommand {
      ChordRest* chordRest;
      int staffMove;
      void flip(EditData*) override;

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
      void flip(EditData*) override;

   public:
      ChangeVelocity(Note*, Note::ValueType, int);
      UNDO_NAME("ChangeVelocity")
      };

//---------------------------------------------------------
//   ChangeMStaffProperties
//---------------------------------------------------------

class ChangeMStaffProperties : public UndoCommand {
      Measure* measure;
      int staffIdx;
      bool visible;
      bool slashStyle;
      void flip(EditData*) override;

   public:
      ChangeMStaffProperties(Measure*, int staffIdx, bool visible, bool slashStyle);
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
      virtual void undo(EditData*) override = 0;
      virtual void redo(EditData*) override = 0;
      };

//---------------------------------------------------------
//   RemoveMeasures
//---------------------------------------------------------

class RemoveMeasures : public InsertRemoveMeasures {

   public:
      RemoveMeasures(MeasureBase* m1, MeasureBase* m2) : InsertRemoveMeasures(m1, m2) {}
      virtual void undo(EditData*) override { insertMeasures(); }
      virtual void redo(EditData*) override { removeMeasures(); }
      UNDO_NAME("RemoveMeasures")
      };

//---------------------------------------------------------
//   InsertMeasures
//---------------------------------------------------------

class InsertMeasures : public InsertRemoveMeasures {

   public:
      InsertMeasures(MeasureBase* m1, MeasureBase* m2) : InsertRemoveMeasures(m1, m2) {}
      virtual void redo(EditData*) override { insertMeasures(); }
      virtual void undo(EditData*) override { removeMeasures(); }
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

      void flip(EditData*) override;

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
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
      UNDO_NAME("AddExcerpt")
      };

//---------------------------------------------------------
//   RemoveExcerpt
//---------------------------------------------------------

class RemoveExcerpt : public UndoCommand {
      Excerpt* excerpt;

   public:
      RemoveExcerpt(Excerpt* ex) : excerpt(ex) {}
      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;
      UNDO_NAME("RemoveExcerpt")
      };

//---------------------------------------------------------
//   SwapExcerpt
//---------------------------------------------------------

class SwapExcerpt : public UndoCommand {
      MasterScore* score;
      int pos1;
      int pos2;

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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
      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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
      Pid id;
      QVariant property;
      PropertyFlags flags;

      void flip(EditData*) override;

   public:
      ChangeProperty(ScoreElement* e, Pid i, const QVariant& v, PropertyFlags ps = PropertyFlags::NOSTYLE)
         : element(e), id(i), property(v), flags(ps) {}
      Pid getId() const  { return id; }
      UNDO_NAME("ChangeProperty")
      };

//---------------------------------------------------------
//   ChangeMetaText
//---------------------------------------------------------

class ChangeMetaText : public UndoCommand {
      Score* score;
      QString id;
      QString text;

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;

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

      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;

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

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      void redo(EditData*) override;
      void undo(EditData*) override;

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

      void flip(EditData*) override;

   public:
      ChangeNoteEvent(Note* n, NoteEvent* oe, const NoteEvent& ne)
         : note(n), oldEvent(oe), newEvent(ne) {}
      UNDO_NAME("ChangeNoteEvent")
      };

//---------------------------------------------------------
//   LinkUnlink
//---------------------------------------------------------

class LinkUnlink : public UndoCommand {
      bool mustDelete  { false };

   protected:
      LinkedElements* le;
      ScoreElement* e;

      void link();
      void unlink();

   public:
      LinkUnlink() {}
      ~LinkUnlink();
      };

//---------------------------------------------------------
//   Unlink
//---------------------------------------------------------

class Unlink : public LinkUnlink {
   public:
      Unlink(ScoreElement*);
      virtual void undo(EditData*) override { link(); }
      virtual void redo(EditData*) override { unlink(); }
      UNDO_NAME("Unlink")
      };

//---------------------------------------------------------
//   Link
//---------------------------------------------------------

class Link : public LinkUnlink {
   public:
      Link(ScoreElement*, ScoreElement*);
      virtual void undo(EditData*) override { unlink(); }
      virtual void redo(EditData*) override { link();   }
      UNDO_NAME("Link")
      };

#if 0
//---------------------------------------------------------
//   LinkStaff
//---------------------------------------------------------

class LinkStaff : public UndoCommand {
      Staff* s1;
      Staff* s2;

   public:
      LinkStaff(Staff* _s1, Staff* _s2) : s1(_s1), s2(_s2) {}
      virtual void undo(EditData*) override { s2->unlink(s1); } // s1 is removed
      virtual void redo(EditData*) override { s1->linkTo(s2); } // s1 is added
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
      virtual void undo(EditData*) override { s2->linkTo(s1); } // s2 is added
      virtual void redo(EditData*) override { s1->unlink(s2); } // s2 is removed
      UNDO_NAME("UnlinkStaff")
      };
#endif

//---------------------------------------------------------
//   ChangeStartEndSpanner
//---------------------------------------------------------

class ChangeStartEndSpanner : public UndoCommand {
      Spanner* spanner;
      Element* start;
      Element* end;

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      void flip(EditData*) override;

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

      void flip(EditData*) override;

   public:
      ChangeGap(Rest* r, bool v) : rest(r), v(v) {}
      UNDO_NAME("ChangeGap")
      };

//---------------------------------------------------------
//   ChangeText
//---------------------------------------------------------

class ChangeText : public UndoCommand {
      TextCursor c;
      QString s;

   protected:
      void insertText(EditData*);
      void removeText(EditData*);

   public:
      ChangeText(const TextCursor* tc, const QString& t) : c(*tc), s(t) {}
      virtual void undo(EditData*) override = 0;
      virtual void redo(EditData*) override = 0;
      const TextCursor& cursor() const { return c; }
      const QString& string() const    { return s; }
      };

//---------------------------------------------------------
//   InsertText
//---------------------------------------------------------

class InsertText : public ChangeText {

   public:
      InsertText(const TextCursor* tc, const QString& t) : ChangeText(tc, t) {}
      virtual void redo(EditData* ed) override { insertText(ed); }
      virtual void undo(EditData* ed) override { removeText(ed); }
      UNDO_NAME("InsertText")
      };

//---------------------------------------------------------
//   RemoveText
//---------------------------------------------------------

class RemoveText : public ChangeText {

   public:
      RemoveText(const TextCursor* tc, const QString& t) : ChangeText(tc, t) {}
      virtual void redo(EditData* ed) override { removeText(ed); }
      virtual void undo(EditData* ed) override { insertText(ed); }
      UNDO_NAME("InsertText")
      };

//---------------------------------------------------------
//   SplitText
//---------------------------------------------------------

class SplitText : public UndoCommand {
      TextCursor c;

      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;

   public:
      SplitText(const TextCursor* tc) : c(*tc) {}
      UNDO_NAME("SplitText");
      };

//---------------------------------------------------------
//   JoinText
//---------------------------------------------------------

class JoinText : public UndoCommand {
      TextCursor c;

      virtual void undo(EditData*) override;
      virtual void redo(EditData*) override;

   public:
      JoinText(const TextCursor* tc) : c(*tc) {}
      UNDO_NAME("JoinText");
      };


}     // namespace Ms
#endif
