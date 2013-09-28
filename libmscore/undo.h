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
class StaffType;
class TimeSig;
class Clef;
class Image;
class Hairpin;
class Bend;
class TremoloBar;
class NoteEvent;
class SlurSegment;
class InstrumentChange;
class Box;
class Accidental;
class Spanner;
class BarLine;
enum class ClefType : signed char;

// #define DEBUG_UNDO

#ifdef DEBUG_UNDO
#define UNDO_NAME(a)  virtual const char* name() const { return a; }
#else
#define UNDO_NAME(a)
#endif

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
#ifdef DEBUG_UNDO
      virtual const char* name() const  { return "UndoCommand"; }
#endif
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
      UNDO_NAME("SaveState");
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
      UNDO_NAME("InsertPart");
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
      UNDO_NAME("RemovePart");
      };

//---------------------------------------------------------
//   InsertStaff
//---------------------------------------------------------

class InsertStaff : public UndoCommand {
      Staff* staff;
      int idx;

   public:
      InsertStaff(Staff*, int idx);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertStaff");
      };

//---------------------------------------------------------
//   RemoveStaff
//---------------------------------------------------------

class RemoveStaff : public UndoCommand {
      Staff* staff;
      int idx;

   public:
      RemoveStaff(Staff*, int idx);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveStaff");
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
      UNDO_NAME("InsertMStaff");
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
      UNDO_NAME("RemoveMStaff");
      };

//---------------------------------------------------------
//   InsertMeasure
//---------------------------------------------------------

class InsertMeasure : public UndoCommand {
      MeasureBase* measure;
      MeasureBase* pos;

   public:
      InsertMeasure(MeasureBase* nm, MeasureBase* p) : measure(nm), pos(p) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertMeasure");
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
      UNDO_NAME("InsertStaves");
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
      UNDO_NAME("RemoveStaves");
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
      UNDO_NAME("SortStaves");
      };

//---------------------------------------------------------
//   ChangePitch
//---------------------------------------------------------

class ChangePitch : public UndoCommand {
      Note* note;
      int pitch;
      int tpc;
      int line;
      void flip();

   public:
      ChangePitch(Note* note, int pitch, int tpc, int l/*, int f, int string*/);
      UNDO_NAME("ChangePitch");
      };

//---------------------------------------------------------
//   ChangeKeySig
//---------------------------------------------------------

class ChangeKeySig : public UndoCommand {
      KeySig* keysig;
      KeySigEvent ks;
      bool showCourtesy;
      bool showNaturals;

      void flip();

   public:
      ChangeKeySig(KeySig*, KeySigEvent newKeySig, bool sc, bool sn);
      UNDO_NAME("ChangeKeySig");
      };

//---------------------------------------------------------
//   FlipNoteDotDirection
//---------------------------------------------------------

class FlipNoteDotDirection : public UndoCommand {
      Note* note;
      void flip();

   public:
      FlipNoteDotDirection(Note* n) : note(n) {}
      UNDO_NAME("FlipNoteDotDirection");
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
      UNDO_NAME("ChangeMeasureLen");
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
      UNDO_NAME("ChangeElement");
      };

//---------------------------------------------------------
//   ChangeVoltaEnding
//---------------------------------------------------------

class ChangeVoltaEnding : public UndoCommand {
      Volta* volta;
      QList<int> list;
      void flip();

   public:
      ChangeVoltaEnding(Volta*, const QList<int>&);
      UNDO_NAME("ChangeVoltaEnding");
      };

//---------------------------------------------------------
//   ChangeVoltaText
//---------------------------------------------------------

class ChangeVoltaText : public UndoCommand {
      Volta* volta;
      QString text;
      void flip();

   public:
      ChangeVoltaText(Volta*, const QString&);
      UNDO_NAME("ChangeVoltaText");
      };

//---------------------------------------------------------
//   ChangeChordRestSize
//---------------------------------------------------------

class ChangeChordRestSize : public UndoCommand {
      ChordRest* cr;
      bool small;
      void flip();

   public:
      ChangeChordRestSize(ChordRest*, bool small);
      UNDO_NAME("ChangeChordRestSize");
      };

//---------------------------------------------------------
//   ChangeChordNoStem
//---------------------------------------------------------

class ChangeChordNoStem : public UndoCommand {
      Chord* chord;
      bool noStem;
      void flip();

   public:
      ChangeChordNoStem(Chord*, bool noStem);
      UNDO_NAME("ChangeChordNoStem");
      };

//---------------------------------------------------------
//   ChangeEndBarLineType
//---------------------------------------------------------

class ChangeEndBarLineType : public UndoCommand {
      Measure* measure;
      BarLineType subtype;
      void flip();

   public:
      ChangeEndBarLineType(Measure*, BarLineType subtype);
      UNDO_NAME("ChangeEndBarLineType");
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
//   ChangeSlurOffsets
//---------------------------------------------------------

class ChangeSlurOffsets : public UndoCommand {
      SlurSegment* slur;
      QPointF off[4];
      void flip();

   public:
      ChangeSlurOffsets(SlurSegment* s, const QPointF& o1, const QPointF& o2,
         const QPointF& o3, const QPointF& o4) : slur(s) {
            off[0] = o1;
            off[1] = o2;
            off[2] = o3;
            off[3] = o4;
            }
      UNDO_NAME("ChangeSlurOffsets");
      };

//---------------------------------------------------------
//   SigInsertTime
//---------------------------------------------------------

class SigInsertTime : public UndoCommand {
      Score* score;
      int tick;
      int len;
      void flip();

   public:
      SigInsertTime(Score*, int tick, int len);
      UNDO_NAME("SigInsertTime");
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
      UNDO_NAME("TransposeHarmony");
      };

//---------------------------------------------------------
//   ExchangeVoice
//---------------------------------------------------------

class ExchangeVoice : public UndoCommand {
      Measure* measure;
      int val1, val2;
      int staff1, staff2;

   public:
      ExchangeVoice(Measure*, int val1, int val2, int staff1, int staff2);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("ExchangeVoice");
      };

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

class ChangeInstrumentShort : public UndoCommand {
      Part* part;
      int tick;
      QList<StaffNameDoc> text;
      void flip();

   public:
      ChangeInstrumentShort(int, Part*, QList<StaffNameDoc>);
      UNDO_NAME("ChangeInstrumentShort");
      };

//---------------------------------------------------------
//   ChangeInstrumentLong
//---------------------------------------------------------

class ChangeInstrumentLong : public UndoCommand {
      Part* part;
      int tick;
      QList<StaffNameDoc> text;
      void flip();

   public:
      const QList<StaffNameDoc>& longNames() const;
      ChangeInstrumentLong(int, Part*, QList<StaffNameDoc>);
      UNDO_NAME("ChangeInstrumentLong");
      };

//---------------------------------------------------------
//   ChangeChordRestLen
//---------------------------------------------------------

class ChangeChordRestLen : public UndoCommand {
      ChordRest* cr;
      TDuration d;
      void flip();

   public:
      ChangeChordRestLen(ChordRest*, const TDuration& d);
      UNDO_NAME("ChangeChordRestLen");
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
      UNDO_NAME("MoveElement");
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
      UNDO_NAME("ChangeBracketSpan");
      };

//---------------------------------------------------------
//   AddElement
//---------------------------------------------------------

class AddElement : public UndoCommand {
      Element* element;

   public:
      AddElement(Element*);
      virtual void undo();
      virtual void redo();
#ifdef DEBUG_UNDO
      virtual const char* name() const;
#endif
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
#ifdef DEBUG_UNDO
      virtual const char* name() const;
#endif
      };

//---------------------------------------------------------
//   ChangeConcertPitch
//---------------------------------------------------------

class ChangeConcertPitch : public UndoCommand {
      Score* score;
      bool val;
      void flip();

   public:
      ChangeConcertPitch(Score* s, bool val);
      UNDO_NAME("ChangeConcertPitch");
      };

//---------------------------------------------------------
//   EditText
//---------------------------------------------------------

class EditText : public UndoCommand {
      Text* text;
      QString oldText;
      int undoLevel;

      void undoRedo();

   public:
      EditText(Text* t, const QString& ot, int l) : text(t), oldText(ot), undoLevel(l) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("EditText");
      };

//---------------------------------------------------------
//   ChangePatch
//---------------------------------------------------------

class ChangePatch : public UndoCommand {
      Channel* channel;
      MidiPatch patch;

      void flip();

   public:
      ChangePatch(Channel* c, const MidiPatch* pt)
         : channel(c), patch(*pt) {}
      UNDO_NAME("ChangePitch");
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
      UNDO_NAME("ChangePageFormat");
      };

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

class ChangeStaff : public UndoCommand {
      Staff*      staff;
      bool        small;
      bool        invisible;
      qreal       userDist;
      StaffType*  staffType;

      void flip();

   public:
      ChangeStaff(Staff*, bool small, bool invisible, qreal userDist, StaffType*);
      UNDO_NAME("ChangeStaff");
      };

//---------------------------------------------------------
//   ChangePart
//---------------------------------------------------------

class ChangePart : public UndoCommand {
      Part* part;
      Instrument instrument;
      QString partName;

      void flip();

   public:
      ChangePart(Part*, const Instrument&, const QString& name);

      UNDO_NAME("ChangePart");
      };

//---------------------------------------------------------
//   ChangePartProperty
//---------------------------------------------------------

class ChangePartProperty : public UndoCommand {
      Part* part;
      int id;
      QVariant property;

      void flip();

   public:
      ChangePartProperty(Part* e, int i, const QVariant& v)
         : part(e), id(i), property(v) {}
      UNDO_NAME("ChangePartProperty");
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
      UNDO_NAME("ChangeTextStyle");
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
      UNDO_NAME("AddTextStyle");
      };

//---------------------------------------------------------
//   ChangeStretch
//---------------------------------------------------------

class ChangeStretch : public UndoCommand {
      Measure* measure;
      qreal stretch;
      void flip();

   public:
      ChangeStretch(Measure*, qreal);
      UNDO_NAME("ChangeStretch");
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
      UNDO_NAME("ChangeStyle");
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
      UNDO_NAME("ChangeStyleVal");
      };

//---------------------------------------------------------
//   ChangeChordStaffMove
//---------------------------------------------------------

class ChangeChordStaffMove : public UndoCommand {
      Chord* chord;
      int staffMove;
      void flip();

   public:
      ChangeChordStaffMove(Chord*, int);
      UNDO_NAME("ChangeChordStaffMove");
      };

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

class ChangeVelocity : public UndoCommand {
      Note* note;
      MScore::ValueType veloType;
      int veloOffset;
      void flip();

   public:
      ChangeVelocity(Note*, MScore::ValueType, int);
      UNDO_NAME("ChangeVelocity");
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
      UNDO_NAME("ChangeMStaffProperties");
      };

//---------------------------------------------------------
//   ChangeMeasureProperties
//---------------------------------------------------------

class ChangeMeasureProperties : public UndoCommand {
      Measure* measure;
      bool breakMM;
      int repeatCount;
      qreal stretch;
      int noOffset;
      bool irregular;

      void flip();

   public:
      ChangeMeasureProperties(Measure*, bool breakMM,
         int repeatCount, qreal stretch, int noOffset, bool irregular);
      UNDO_NAME("ChangeMeasureProperties");
      };

//---------------------------------------------------------
//   ChangeTimesig
//---------------------------------------------------------

class ChangeTimesig : public UndoCommand {
      TimeSig* timesig;
      bool showCourtesy;
      Fraction sig;
      Fraction stretch;
      TimeSigType subtype;
      QString numeratorString;
      QString denominatorString;

      void flip();

   public:
      ChangeTimesig(TimeSig* _timesig, bool sc, const Fraction&,
         const Fraction&, QString, QString, TimeSigType subtype);
      UNDO_NAME("ChangeTimesig");
      };

//---------------------------------------------------------
//   RemoveMeasures
//---------------------------------------------------------

class RemoveMeasures : public UndoCommand {
      Measure* fm;
      Measure* lm;

   public:
      RemoveMeasures(Measure*, Measure*);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveMeasures");
      };

//---------------------------------------------------------
//   InsertMeasures
//---------------------------------------------------------

class InsertMeasures : public UndoCommand {
      Measure* fm;
      Measure* lm;

   public:
      InsertMeasures(Measure* m1, Measure* m2) : fm(m1), lm(m2) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertMeasures");
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
      UNDO_NAME("ChangeImage");
      };

//---------------------------------------------------------
//   ChangeHairpin
//---------------------------------------------------------

class ChangeHairpin : public UndoCommand {
      Hairpin* hairpin;
      int veloChange;
      Element::DynamicRange dynRange;
      bool diagonal;

      void flip();

   public:
      ChangeHairpin(Hairpin* h, int c, Element::DynamicRange t, bool dg)
         : hairpin(h), veloChange(c), dynRange(t), diagonal(dg) {}
      UNDO_NAME("ChangeHairpin");
      };

//---------------------------------------------------------
//   ChangeDuration
//---------------------------------------------------------

class ChangeDuration : public UndoCommand {
      ChordRest* cr;
      Fraction d;

      void flip();

   public:
      ChangeDuration(ChordRest* _cr, Fraction _d) : cr(_cr), d(_d) {}
      UNDO_NAME("ChangeDuration");
      };

//---------------------------------------------------------
//   AddExcerpt
//---------------------------------------------------------

class AddExcerpt : public UndoCommand {
      Score* score;

   public:
      AddExcerpt(Score* s) : score(s) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("AddExcerpt");
      };

//---------------------------------------------------------
//   RemoveExcerpt
//---------------------------------------------------------

class RemoveExcerpt : public UndoCommand {
      Score* score;

   public:
      RemoveExcerpt(Score* s) : score(s) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveExcerpt");
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
      UNDO_NAME("ChangeBend");
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
      UNDO_NAME("ChangeTremoloBar");
      };

//---------------------------------------------------------
//   ChangeNoteEvents
//---------------------------------------------------------

class ChangeNoteEvents : public UndoCommand {
      Chord* chord;
      QList<NoteEvent*> events;

      void flip();

   public:
      ChangeNoteEvents(Chord* n, const QList<NoteEvent*>& l) : chord(n), events(l) {}
      UNDO_NAME("ChangeNoteEvents");
      };

//---------------------------------------------------------
//   ChangeInstrument
//---------------------------------------------------------

class ChangeInstrument : public UndoCommand {
      InstrumentChange* is;
      Instrument instrument;

      void flip();

   public:
      ChangeInstrument(InstrumentChange* _is, const Instrument& i) : is(_is), instrument(i) {}
      UNDO_NAME("ChangeInstrument");
      };

extern void updateNoteLines(Segment*, int track);

//---------------------------------------------------------
//   ChangeBoxProperties
//---------------------------------------------------------

class ChangeBoxProperties : public UndoCommand {
      Box* _box;

      qreal _marginLeft, _marginTop, _marginRight, _marginBottom;
      Spatium _height, _width;
      qreal _topGap, _bottomGap;

      void flip();

   public:
      ChangeBoxProperties(Box *, qreal, qreal, qreal, qreal,
         Spatium, Spatium,
         qreal, qreal);
      UNDO_NAME("ChangeBoxProperties");
      };

//---------------------------------------------------------
//   SwapCR
//---------------------------------------------------------

class SwapCR : public UndoCommand {
      ChordRest* cr1;
      ChordRest* cr2;

      void flip();

   public:
      SwapCR(ChordRest* a, ChordRest* b) : cr1(a), cr2(b) {}
      UNDO_NAME("SwapCR");
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
      UNDO_NAME("ChangeClef");
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
      UNDO_NAME("MoveStaff");
      };

//---------------------------------------------------------
//   ChangeDurationType
//---------------------------------------------------------

class ChangeDurationType : public UndoCommand {
      ChordRest* cr;
      TDuration t;

      void flip();

   public:
      ChangeDurationType(ChordRest* _cr, TDuration _t)
         : cr(_cr), t(_t) {}
      UNDO_NAME("ChangeDurationType");
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
      UNDO_NAME("ChangeStaffUserDist");
      };

//---------------------------------------------------------
//   ChangeProperty
//---------------------------------------------------------

class ChangeProperty : public UndoCommand {
      Element* element;
      P_ID id;
      QVariant property;
      PropertyStyle propertyStyle;

      void flip();

   public:
      ChangeProperty(Element* e, P_ID i, const QVariant& v, PropertyStyle ps = PropertyStyle::NOSTYLE)
         : element(e), id(i), property(v), propertyStyle(ps) {}
      P_ID getId() const  { return id; }
      UNDO_NAME("ChangeProperty");
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
      UNDO_NAME("ChangeMetaText");
      };

//---------------------------------------------------------
//   ChangeEventList
//---------------------------------------------------------

class ChangeEventList : public UndoCommand {
      Chord* chord;
      QList<NoteEventList> events;
      bool userModified;

      void flip();

   public:
      ChangeEventList(Chord* c, const QList<NoteEventList> l, bool u) : chord(c), events(l), userModified(u) {}
      ~ChangeEventList();
      UNDO_NAME("ChangeEventList");
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
      UNDO_NAME("ChangeSynthesizerState");
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
      UNDO_NAME("RemoveBracket");
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
      UNDO_NAME("AddBracket");
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
      UNDO_NAME("ChangeSpannerElements");
      };

}     // namespace Ms
#endif

