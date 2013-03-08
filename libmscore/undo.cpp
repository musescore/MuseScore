//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: undo.cpp 5649 2012-05-19 15:14:43Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Implementation of undo functions.

 The undo system requires calling startUndo() when starting a GUI command
 and calling endUndo() when ending the command. All changes to a score
 in response to a GUI command must be undoable/redoable by executing
 a sequence of low-level undo operations. This sequence is built by the code
 handling the command, by calling one or more undoOp()'s
 between startUndo() and endUndo().
*/

#include "undo.h"
#include "element.h"
#include "note.h"
#include "score.h"
#include "segment.h"
#include "measure.h"
#include "system.h"
#include "select.h"
#include "input.h"
#include "slur.h"
#include "clef.h"
#include "staff.h"
#include "chord.h"
#include "sig.h"
#include "key.h"
#include "barline.h"
#include "volta.h"
#include "tuplet.h"
#include "harmony.h"
#include "pitchspelling.h"
#include "part.h"
#include "beam.h"
#include "dynamic.h"
#include "page.h"
#include "keysig.h"
#include "image.h"
#include "hairpin.h"
#include "rest.h"
#include "bend.h"
#include "tremolobar.h"
#include "articulation.h"
#include "noteevent.h"
#include "slur.h"
#include "tempotext.h"
#include "instrchange.h"
#include "box.h"
#include "stafftype.h"
#include "accidental.h"
#include "layoutbreak.h"
#include "spanner.h"
#include "sequencer.h"
#include "breath.h"
#include "fingering.h"
#include "rehearsalmark.h"
#include "excerpt.h"
#include "stafftext.h"
#include "chordline.h"
#include "tremolo.h"

extern Measure* tick2measure(int tick);

//---------------------------------------------------------
//   updateNoteLines
//    compute line position of note heads after
//    clef change
//---------------------------------------------------------

void updateNoteLines(Segment* segment, int track)
      {
      Staff* staff = segment->score()->staff(track / VOICES);
      if (staff->isDrumStaff() || staff->isTabStaff())
            return;
      for (Segment* s = segment->next1(); s; s = s->next1()) {
            if (s->segmentType() == Segment::SegClef && s->element(track) && !s->element(track)->generated())
                  break;
            if (s->segmentType() != Segment::SegChordRest)
                  continue;
            for (int t = track; t < track + VOICES; ++t) {
                  Chord* chord = static_cast<Chord*>(s->element(t));
                  if (chord && chord->type() == Element::CHORD) {
                        int n = chord->notes().size();
                        for (int i = 0; i < n; ++i)
                              chord->notes().at(i)->updateLine();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   UndoCommand
//---------------------------------------------------------

UndoCommand::~UndoCommand()
      {
      foreach(UndoCommand* c, childList)
            delete c;
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void UndoCommand::undo()
      {
      int n = childList.size();
      for (int i = n-1; i >= 0; --i) {
#ifdef DEBUG_UNDO
            qDebug("   undo<%s> %p", childList[i]->name(), childList[i]);
#endif
            childList[i]->undo();
            }
      flip();
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void UndoCommand::redo()
      {
      int n = childList.size();
      for (int i = 0; i < n; ++i) {
#ifdef DEBUG_UNDO
            qDebug("   redo<%s> %p", childList[i]->name(), childList[i]);
#endif
            childList[i]->redo();
            }
      flip();
      }

//---------------------------------------------------------
//   unwind
//---------------------------------------------------------

void UndoCommand::unwind()
      {
      while (!childList.isEmpty()) {
            UndoCommand* c = childList.takeLast();
            c->undo();
            delete c;
            }
      }

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

UndoStack::UndoStack()
      {
      curCmd   = 0;
      curIdx   = 0;
      cleanIdx = 0;
      }

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

UndoStack::~UndoStack()
      {
      }

//---------------------------------------------------------
//   beginMacro
//---------------------------------------------------------

void UndoStack::beginMacro()
      {
      if (curCmd) {
            qDebug("UndoStack:beginMacro(): alread active");
            return;
            }
      curCmd = new UndoCommand();
      if (MScore::debugMode)
            qDebug("UndoStack::beginMacro %p, UndoStack %p", curCmd, this);
      }

//---------------------------------------------------------
//   endMacro
//---------------------------------------------------------

void UndoStack::endMacro(bool rollback)
      {
      if (MScore::debugMode)
            qDebug("UndoStack::endMacro %d", rollback);
      if (curCmd == 0) {
            qDebug("UndoStack:endMacro(): not active");
            return;
            }
      if (rollback)
            delete curCmd;
      else {
            while (list.size() > curIdx) {
                  UndoCommand* cmd = list.takeLast();
                  delete cmd;
                  }
            list.append(curCmd);
            ++curIdx;
            }
      curCmd = 0;
      }

//---------------------------------------------------------
//   push
//---------------------------------------------------------

void UndoStack::push(UndoCommand* cmd)
      {
      if (!curCmd) {
            // this can happen for layout() outside of a command (load)
            // qDebug("UndoStack:push(): no active command, UndoStack %p", this);

            cmd->redo();
            delete cmd;
            return;
            }
#ifdef DEBUG_UNDO
      if (strcmp(cmd->name(), "ChangeProperty") == 0) {
            ChangeProperty* cp = static_cast<ChangeProperty*>(cmd);
            qDebug("UndoStack::push <%s> %p id %d", cmd->name(), cmd, int(cp->getId()));
            }
      else {
            qDebug("UndoStack::push <%s> %p", cmd->name(), cmd);
            }
#endif
      curCmd->appendChild(cmd);
      cmd->redo();
      }

//---------------------------------------------------------
//   push1
//---------------------------------------------------------

void UndoStack::push1(UndoCommand* cmd)
      {
      if (curCmd)
            curCmd->appendChild(cmd);
      }

//---------------------------------------------------------
//   pop
//---------------------------------------------------------

void UndoStack::pop()
      {
      if (!curCmd) {
            qDebug("UndoStack:pop(): no active command");
            return;
            }
      UndoCommand* cmd = curCmd->removeChild();
      cmd->undo();
      }

//---------------------------------------------------------
//   setClean
//---------------------------------------------------------

void UndoStack::setClean()
      {
      if (cleanIdx != curIdx) {
            cleanIdx = curIdx;
            }
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void UndoStack::undo()
      {
      if (curIdx) {
            --curIdx;
            Q_ASSERT(curIdx >= 0);
            if (MScore::debugMode)
                  qDebug("--undo index %d", curIdx);
            list[curIdx]->undo();
            }
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void UndoStack::redo()
      {
      if (canRedo()) {
            if (MScore::debugMode)
                  qDebug("--redo index %d", curIdx);
            list[curIdx++]->redo();
            }
      }

//---------------------------------------------------------
//   SaveState
//---------------------------------------------------------

SaveState::SaveState(Score* s)
      {
      score          = s;
      redoInputState = score->inputState();
      redoSelection  = score->selection();
      }

void SaveState::undo()
      {
      redoInputState = score->inputState();
      redoSelection  = score->selection();
      score->setInputState(undoInputState);
      undoSelection.reconstructElementList();
      score->setSelection(undoSelection);
      }

void SaveState::redo()
      {
      undoInputState = score->inputState();
      undoSelection  = score->selection();
      score->setInputState(redoInputState);
      score->setSelection(redoSelection);
      score->selection().reconstructElementList();
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Score::undoChangeProperty(Element* e, P_ID t, const QVariant& st)
      {
      if (propertyLink(t) && e->links()) {
            foreach(Element* e, *e->links()) {
                  if (e->getProperty(t) != st)
                        undo(new ChangeProperty(e, t, st));
                  }
            }
      else {
            if (e->getProperty(t) != st)
                  undo(new ChangeProperty(e, t, st));
            }
      }

//---------------------------------------------------------
//   undoChangeElement
//---------------------------------------------------------

void Score::undoChangeElement(Element* oldElement, Element* newElement)
      {
      undo(new ChangeElement(oldElement, newElement));
      }

//---------------------------------------------------------
//   undoChangePitch
//---------------------------------------------------------

void Score::undoChangePitch(Note* note, int pitch, int tpc, int line/*, int fret, int string*/)
      {
      Chord* chord  = note->chord();
      int noteIndex = chord->notes().indexOf(note);

      Q_ASSERT(noteIndex >= 0);

      LinkedElements* l = chord->links();
      if (l) {
            foreach(Element* e, *l) {
                  Chord* c = static_cast<Chord*>(e);
                  Q_ASSERT(c->type() == Element::CHORD);
                  Q_ASSERT(c->notes().size() > noteIndex);
                  Note* n  = c->notes().at(noteIndex);
                  undo(new ChangePitch(n, pitch, tpc, line));
                  }
            }
      else
            undo(new ChangePitch(note, pitch, tpc, line));
      }

//---------------------------------------------------------
//   undoChangeKeySig
//---------------------------------------------------------

void Score::undoChangeKeySig(Staff* ostaff, int tick, KeySigEvent st)
      {
      QList<Staff*> staffList;
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);

      LinkedElements* links = 0;
      foreach(Staff* staff, staffList) {
            Score* score = staff->score();

            Measure* measure = score->tick2measure(tick);
            if (!measure) {
                  qDebug("measure for tick %d not found!", tick);
                  continue;
                  }
            Segment* s   = measure->undoGetSegment(Segment::SegKeySig, tick);
            int staffIdx = score->staffIdx(staff);
            int track    = staffIdx * VOICES;
            KeySig* ks   = static_cast<KeySig*>(s->element(track));

            KeySig* nks  = new KeySig(score);
            nks->setTrack(track);
            nks->changeKeySigEvent(st);
            nks->setParent(s);
            if (links == 0)
                  links = new LinkedElements(score);
            links->append(nks);
            nks->setLinks(links);

            if (ks) {
                  qDebug("  changeElement");
                  undo(new ChangeElement(ks, nks));
                  }
            else {
                  qDebug("  addElement");
                  undo(new AddElement(nks));
                  }
            updateNoteLines(s, track);
            }
      }

//---------------------------------------------------------
//   undoChangeClef
//---------------------------------------------------------

void Score::undoChangeClef(Staff* ostaff, Segment* seg, ClefType st)
      {
      QList<Staff*> staffList;
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);

      bool firstSeg = seg->measure()->first() == seg;

      foreach(Staff* staff, staffList) {
            Score* score = staff->score();
            if (staff->staffType()->group() != clefTable[st].staffGroup) {
                  qDebug("Staff::changeClef(%d): invalid staff group, src %d, dst %d",
                     st, clefTable[st].staffGroup, staff->staffType()->group());
                  continue;
                  }
            Measure* measure = score->tick2measure(seg->tick());
            if (!measure) {
                  qDebug("measure for tick %d not found!", seg->tick());
                  continue;
                  }

            // move clef to last segment of prev measure?
            //    TODO: section break?
            if (firstSeg
               && measure->prevMeasure()
               && !(measure->prevMeasure()->repeatFlags() & RepeatEnd)
               ) {
                  measure = measure->prevMeasure();
                  }

            int tick = seg->tick();
            Segment* segment = measure->findSegment(seg->segmentType(), seg->tick());
            if (segment) {
                  if (segment->segmentType() != Segment::SegClef) {
                        if (segment->prev() && segment->prev()->segmentType() == Segment::SegClef) {
                              segment = segment->prev();
                             }
                        else {
                              Segment* s = new Segment(measure, Segment::SegClef, seg->tick());
                              s->setNext(segment);
                              s->setPrev(segment->prev());
                              score->undoAddElement(s);
                              segment = s;
                              }
                        }
                  }
            else {
                  segment = new Segment(measure, Segment::SegClef, seg->tick());
                  score->undoAddElement(segment);
                  }
            int staffIdx = staff->idx();
            int track    = staffIdx * VOICES;
            Clef* clef   = static_cast<Clef*>(segment->element(track));

            if (clef) {
                  //
                  // for transposing instruments, differentiate
                  // clef type for concertPitch
                  //
                  Instrument* i = staff->part()->instr(tick);
                  ClefType cp, tp;
                  if (i->transpose().isZero()) {
                        cp = st;
                        tp = st;
                        }
                  else {
                        bool concertPitch = score->concertPitch();
                        if (concertPitch) {
                              cp = st;
                              tp = clef->transposingClef();
                              }
                        else {
                              cp = clef->concertClef();
                              tp = st;
                              }
                        }
                  clef->setGenerated(false);
                  score->undo(new ChangeClefType(clef, cp, tp));
                  }
            else {
                  clef = new Clef(score);
                  clef->setTrack(track);
                  clef->setClefType(st);
                  clef->setParent(segment);
                  score->undo(new AddElement(clef));
                  }
            }
      }

//---------------------------------------------------------
//   undoChangeTpc
//---------------------------------------------------------

void Score::undoChangeTpc(Note* note, int tpc)
      {
      undoChangeProperty(note, P_TPC, tpc);
      }

//---------------------------------------------------------
//   findLinkedVoiceElement
//---------------------------------------------------------

static Element* findLinkedVoiceElement(Element* e, Staff* nstaff)
      {
      Score* score     = nstaff->score();
      Segment* segment = static_cast<Segment*>(e->parent());
      Measure* measure = segment->measure();
      Measure* m       = score->tick2measure(measure->tick());
      Segment* s       = m->findSegment(segment->segmentType(), segment->tick());
      int staffIdx     = score->staffIdx(nstaff);
      return s->element(staffIdx * VOICES + e->voice());
      }

//---------------------------------------------------------
//   undoChangeChordRestLen
//---------------------------------------------------------

void Score::undoChangeChordRestLen(ChordRest* cr, const TDuration& d)
      {
      Staff* ostaff = cr->staff();
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves) {
            foreach(Staff* staff, linkedStaves->staves()) {
                  if (staff == cr->staff())
                        continue;
                  ChordRest* ncr = static_cast<ChordRest*>(findLinkedVoiceElement(cr, staff));
                  undo(new ChangeChordRestLen(ncr, d));
                  }
            }
      undo(new ChangeChordRestLen(cr, d));
      }

//---------------------------------------------------------
//   undoChangeEndBarLineType
//---------------------------------------------------------

void Score::undoChangeEndBarLineType(Measure* m, BarLineType subtype)
      {
      undo(new ChangeEndBarLineType(m, subtype));
      }

//---------------------------------------------------------
//   undoChangeBarLineSpan
//---------------------------------------------------------

void Score::undoChangeBarLineSpan(Staff* staff, int span, int spanFrom, int spanTo)
      {
      undo(new ChangeBarLineSpan(staff, span, spanFrom, spanTo));
      }

//---------------------------------------------------------
//   undoChangeSingleBarLineSpan
//---------------------------------------------------------

void Score::undoChangeSingleBarLineSpan(BarLine* barLine, int span, int spanFrom, int spanTo)
      {
      undo(new ChangeSingleBarLineSpan(barLine, span, spanFrom, spanTo));
      }

//---------------------------------------------------------
//   undoTransposeHarmony
//---------------------------------------------------------

void Score::undoTransposeHarmony(Harmony* h, int rootTpc, int baseTpc)
      {
      undo(new TransposeHarmony(h, rootTpc, baseTpc));
      }

//---------------------------------------------------------
//   undoExchangeVoice
//---------------------------------------------------------

void Score::undoExchangeVoice(Measure* measure, int v1, int v2, int staff1, int staff2)
      {
      undo(new ExchangeVoice(measure, v1, v2, staff1, staff2));
      if (v1 == 0 || v2 == 0) {
            for (int staffIdx = staff1; staffIdx < staff2; ++staffIdx) {
                  // check for complete timeline of voice 0
                  int ctick  = measure->tick();
                  int track = staffIdx * VOICES;
                  for (Segment* s = measure->first(Segment::SegChordRest); s; s = s->next(Segment::SegChordRest)) {
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (cr == 0)
                              continue;
                        if (ctick < s->tick()) {
                              // fill gap
                              int ticks = s->tick() - ctick;
                              setRest(ctick, track, Fraction::fromTicks(ticks), false, 0);
                              }
                        ctick = s->tick() + cr->actualTicks();
                        }
                  int etick = measure->tick() + measure->ticks();
                  if (ctick < etick) {
                        // fill gap
                        int ticks = etick - ctick;
                        setRest(ctick, track, Fraction::fromTicks(ticks), false, 0);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   undoRemovePart
//---------------------------------------------------------

void Score::undoRemovePart(Part* part, int idx)
      {
      undo(new RemovePart(part, idx));
      }

//---------------------------------------------------------
//   undoInsertPart
//---------------------------------------------------------

void Score::undoInsertPart(Part* part, int idx)
      {
      undo(new InsertPart(part, idx));
      }

//---------------------------------------------------------
//   undoRemoveStaff
//---------------------------------------------------------

void Score::undoRemoveStaff(Staff* staff, int idx)
      {
      undo(new RemoveStaff(staff, idx));
      }

//---------------------------------------------------------
//   undoInsertStaff
//---------------------------------------------------------

void Score::undoInsertStaff(Staff* staff, int idx)
      {
      undo(new InsertStaff(staff, idx));
      }

//---------------------------------------------------------
//   undoMove
//---------------------------------------------------------

void Score::undoMove(Element* e, const QPointF& pt)
      {
      undo(new MoveElement(e, pt));
      }

//---------------------------------------------------------
//   undoChangeVoltaEnding
//---------------------------------------------------------

void Score::undoChangeVoltaEnding(Volta* volta, const QList<int>& l)
      {
      undo(new ChangeVoltaEnding(volta, l));
      }

//---------------------------------------------------------
//   undoChangeVoltaText
//---------------------------------------------------------

void Score::undoChangeVoltaText(Volta* volta, const QString& s)
      {
      undo(new ChangeVoltaText(volta, s));
      }

//---------------------------------------------------------
//   undoChangeChordRestSize
//---------------------------------------------------------

void Score::undoChangeChordRestSize(ChordRest* cr, bool small)
      {
      undo(new ChangeChordRestSize(cr, small));
      }

//---------------------------------------------------------
//   undoChangeChordNoStem
//---------------------------------------------------------

void Score::undoChangeChordNoStem(Chord* cr, bool noStem)
      {
      undo(new ChangeChordNoStem(cr, noStem));
      }

//---------------------------------------------------------
//   undoChangeBracketSpan
//---------------------------------------------------------

void Score::undoChangeBracketSpan(Staff* staff, int column, int span)
      {
      undo(new ChangeBracketSpan(staff, column, span));
      }

//---------------------------------------------------------
//   undoChangeInvisible
//---------------------------------------------------------

void Score::undoChangeInvisible(Element* e, bool v)
      {
      undoChangeProperty(e, P_VISIBLE, v);
      e->setGenerated(false);
      }

//---------------------------------------------------------
//   undoAddElement
//---------------------------------------------------------

void Score::undoAddElement(Element* element)
      {
      if (element->isText()) {
            Text* text = static_cast<Text*>(element);
            if (text->textStyleType() == TEXT_STYLE_UNKNOWN) {
                  style()->addTextStyle(text->textStyle());
                  text->setTextStyleType(style()->textStyleType(text->textStyle().name()));
                  }
            }
      QList<Staff*> staffList;
      Staff* ostaff;
      if (element->type() == Element::SLUR)
            ostaff = static_cast<Slur*>(element)->startElement()->staff();
      else
            ostaff = element->staff();

      Element::ElementType et = element->type();


      //
      // some elements are replicated for all parts regardless of
      // linking:
      //

      if ((et == Element::REHEARSAL_MARK)
         || ((et == Element::STAFF_TEXT) && static_cast<StaffText*>(element)->systemFlag())
         || (et == Element::JUMP)
         || (et == Element::MARKER)
         ) {
            foreach(Score* s, scoreList())
                  staffList.append(s->staff(0));

            foreach(Staff* staff, staffList) {
                  Score* score = staff->score();
                  int staffIdx = score->staffIdx(staff);
                  Element* ne;
                  if (staff == ostaff)
                        ne = element;
                  else {
                        ne = element->linkedClone();
                        ne->setScore(score);
                        ne->setSelected(false);
                        ne->setTrack(staffIdx * VOICES + element->voice());
                        }
                  if (et == Element::REHEARSAL_MARK
                     || et == Element::STAFF_TEXT
                     || (et == Element::JUMP)
                     || (et == Element::MARKER)
                     ) {
                        Segment* segment  = static_cast<Segment*>(element->parent());
                        int tick          = segment->tick();
                        Measure* m        = score->tick2measure(tick);
                        Segment* seg      = m->findSegment(Segment::SegChordRest, tick);
                        int ntrack        = staffIdx * VOICES + element->voice();
                        ne->setTrack(ntrack);
                        ne->setParent(seg);
                        undo(new AddElement(ne));
                        }
                  }
            return;
            }

      if (et == Element::FINGERING
         || (et == Element::IMAGE  && element->parent()->type() != Element::SEGMENT)
         || (et == Element::SYMBOL && element->parent()->type() != Element::SEGMENT)
         || et == Element::NOTE
         || et == Element::GLISSANDO
            ) {
            Element* parent       = element->parent();
            LinkedElements* links = parent->links();
            if (links == 0) {
                  undo(new AddElement(element));
                  if (element->type() == Element::FINGERING)
                        element->score()->layoutFingering(static_cast<Fingering*>(element));
                  return;
                  }
            foreach(Element* e, *links) {
                  Element* ne = (e == parent) ? element : element->linkedClone();
                  ne->setScore(e->score());
                  ne->setSelected(false);
                  ne->setParent(e);
                  undo(new AddElement(ne));
                  if (ne->type() == Element::FINGERING)
                        e->score()->layoutFingering(static_cast<Fingering*>(ne));
                  }
            return;
            }

      if (ostaff == 0 || (
         et    != Element::ARTICULATION
         && et != Element::CHORDLINE
         && et != Element::SLUR
         && et != Element::TIE
         && et != Element::NOTE
         && et != Element::INSTRUMENT_CHANGE
         && et != Element::HAIRPIN
         && et != Element::OTTAVA
         && et != Element::TRILL
         && et != Element::TEXTLINE
         && et != Element::VOLTA
         && et != Element::BREATH
         && et != Element::DYNAMIC
         && et != Element::STAFF_TEXT
         && et != Element::TREMOLO
         && et != Element::ARPEGGIO
         && et != Element::SYMBOL)
            ) {
            undo(new AddElement(element));
            return;
            }
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);

      foreach(Staff* staff, staffList) {
            Score* score = staff->score();
            int staffIdx = score->staffIdx(staff);
            Element* ne;
            if (staff == ostaff)
                  ne = element;
            else {
                  ne = element->linkedClone();
                  ne->setScore(score);
                  ne->setSelected(false);
                  ne->setTrack(staffIdx * VOICES + element->voice());
                  }
            if (element->type() == Element::ARTICULATION) {
                  Articulation* a  = static_cast<Articulation*>(element);
                  Segment* segment;
                  Segment::SegmentType st;
                  Measure* m;
                  int tick;
                  if (a->parent()->isChordRest()) {
                        ChordRest* cr = a->chordRest();
                        segment       = cr->segment();
                        st            = Segment::SegChordRest;
                        tick          = segment->tick();
                        m             = score->tick2measure(tick);
                        }
                  else {
                        segment  = static_cast<Segment*>(a->parent()->parent());
                        st       = Segment::SegEndBarLine;
                        tick     = segment->tick();
                        m        = score->tick2measure(tick);
                        if (m->tick() == tick)
                              m = m->prevMeasure();
                        }
                  Segment* seg = m->findSegment(st, tick);
                  if (seg == 0) {
                        qDebug("undoAddSegment: segment not found");
                        break;
                        }
                  Articulation* na = static_cast<Articulation*>(ne);
                  int ntrack       = staffIdx * VOICES + a->voice();
                  na->setTrack(ntrack);
                  if (a->parent()->isChordRest()) {
                        ChordRest* ncr = static_cast<ChordRest*>(seg->element(ntrack));
                        na->setParent(ncr);
                        }
                  else {
                        BarLine* bl = static_cast<BarLine*>(seg->element(ntrack));
                        na->setParent(bl);
                        }
                  undo(new AddElement(na));
                  }
            else if (element->type() == Element::CHORDLINE) {
                  ChordLine* a     = static_cast<ChordLine*>(element);
                  Segment* segment = a->chord()->segment();
                  int tick         = segment->tick();
                  Measure* m       = score->tick2measure(tick);
                  Segment* seg     = m->findSegment(Segment::SegChordRest, tick);
                  if (seg == 0) {
                        qDebug("undoAddSegment: segment not found");
                        break;
                        }
                  int ntrack    = staffIdx * VOICES + a->voice();
                  ne->setTrack(ntrack);
                  ChordRest* ncr = static_cast<ChordRest*>(seg->element(ntrack));
                  ne->setParent(ncr);
                  undo(new AddElement(ne));
                  }
            //
            // elements with Segment as parent
            //
            else if (element->type() == Element::SYMBOL
               || element->type() == Element::IMAGE
               || element->type() == Element::DYNAMIC
               || element->type() == Element::STAFF_TEXT) {
                  Segment* segment = static_cast<Segment*>(element->parent());
                  int tick         = segment->tick();
                  Measure* m       = score->tick2measure(tick);
                  Segment* seg     = m->findSegment(Segment::SegChordRest, tick);
                  int ntrack       = staffIdx * VOICES + element->voice();
                  ne->setTrack(ntrack);
                  ne->setParent(seg);
                  undo(new AddElement(ne));
                  }
            else if (element->type() == Element::SLUR) {
                  Slur* slur     = static_cast<Slur*>(element);
                  ChordRest* cr1 = static_cast<ChordRest*>(slur->startElement());
                  ChordRest* cr2 = static_cast<ChordRest*>(slur->endElement());
                  Segment* s1    = cr1->segment();
                  Segment* s2    = cr2->segment();
                  Measure* m1    = s1->measure();
                  Measure* m2    = s2->measure();
                  Measure* nm1   = score->tick2measure(m1->tick());
                  Measure* nm2   = score->tick2measure(m2->tick());
                  Segment* ns1   = nm1->findSegment(s1->segmentType(), s1->tick());
                  Segment* ns2   = nm2->findSegment(s2->segmentType(), s2->tick());
                  Chord* c1      = static_cast<Chord*>(ns1->element(staffIdx * VOICES + cr1->voice()));
                  Chord* c2      = static_cast<Chord*>(ns2->element(staffIdx * VOICES + cr2->voice()));
                  Slur* nslur    = static_cast<Slur*>(ne);
                  nslur->setStartElement(c1);
                  nslur->setEndElement(c2);
                  nslur->setParent(0);
                  undo(new AddElement(nslur));
                  }
            else if (element->type() == Element::TREMOLO && static_cast<Tremolo*>(element)->twoNotes()) {
                  Tremolo* tremolo = static_cast<Tremolo*>(element);
                  ChordRest* cr1 = static_cast<ChordRest*>(tremolo->chord1());
                  ChordRest* cr2 = static_cast<ChordRest*>(tremolo->chord2());
                  Segment* s1    = cr1->segment();
                  Segment* s2    = cr2->segment();
                  Measure* m1    = s1->measure();
                  Measure* m2    = s2->measure();
                  Measure* nm1   = score->tick2measure(m1->tick());
                  Measure* nm2   = score->tick2measure(m2->tick());
                  Segment* ns1   = nm1->findSegment(s1->segmentType(), s1->tick());
                  Segment* ns2   = nm2->findSegment(s2->segmentType(), s2->tick());
                  Chord* c1      = static_cast<Chord*>(ns1->element(staffIdx * VOICES + cr1->voice()));
                  Chord* c2      = static_cast<Chord*>(ns2->element(staffIdx * VOICES + cr2->voice()));
                  Tremolo* ntremolo = static_cast<Tremolo*>(ne);
                  ntremolo->setChords(c1, c2);
                  ntremolo->setParent(c1);
                  undo(new AddElement(ntremolo));
                  }
            else if (
               (element->type() == Element::TREMOLO && !static_cast<Tremolo*>(element)->twoNotes())
               || (element->type() == Element::ARPEGGIO))
                  {
                  ChordRest* cr = static_cast<ChordRest*>(element->parent());
                  Segment* s    = cr->segment();
                  Measure* m    = s->measure();
                  Measure* nm   = score->tick2measure(m->tick());
                  Segment* ns   = nm->findSegment(s->segmentType(), s->tick());
                  Chord* c1     = static_cast<Chord*>(ns->element(staffIdx * VOICES + cr->voice()));
                  ne->setParent(c1);
                  undo(new AddElement(ne));
                  }
            else if (element->type() == Element::TIE) {
                  Tie* tie       = static_cast<Tie*>(element);
                  Note* n1       = tie->startNote();
                  Note* n2       = tie->endNote();
                  Chord* cr1     = n1->chord();
                  Chord* cr2     = n2 ? n2->chord() : 0;
                  Segment* s1    = cr1->segment();
                  Segment* s2    = cr2 ? cr2->segment() : 0;
                  Measure* nm1   = score->tick2measure(s1->tick());
                  Measure* nm2   = s2 ? score->tick2measure(s2->tick()) : 0;
                  Segment* ns1   = nm1->findSegment(s1->segmentType(), s1->tick());
                  Segment* ns2   = nm2 ? nm2->findSegment(s2->segmentType(), s2->tick()) : 0;
                  Chord* c1      = static_cast<Chord*>(ns1->element(staffIdx * VOICES + cr1->voice()));
                  Chord* c2      = ns2 ? static_cast<Chord*>(ns2->element(staffIdx * VOICES + cr2->voice())) : 0;
                  Note* nn1      = c1->findNote(n1->pitch());
                  Note* nn2      = c2 ? c2->findNote(n2->pitch()) : 0;
                  Tie* ntie      = static_cast<Tie*>(ne);
                  QList<SpannerSegment*>& segments = ntie->spannerSegments();
                  foreach(SpannerSegment* segment, segments)
                        delete segment;
                  segments.clear();
                  ntie->setTrack(c1->track());
                  ntie->setStartNote(nn1);
                  ntie->setEndNote(nn2);
                  undo(new AddElement(ntie));
                  }
            else if (element->type() == Element::INSTRUMENT_CHANGE) {
                  InstrumentChange* is = static_cast<InstrumentChange*>(element);
                  Segment* s1    = is->segment();
                  Measure* m1    = s1->measure();
                  Measure* nm1   = score->tick2measure(m1->tick());
                  Segment* ns1   = nm1->findSegment(s1->segmentType(), s1->tick());
                  InstrumentChange* nis = static_cast<InstrumentChange*>(ne);
                  nis->setParent(ns1);
                  undo(new AddElement(nis));
                  }
            else if (element->type() == Element::HAIRPIN
               || element->type() == Element::OTTAVA
               || element->type() == Element::TRILL
               || element->type() == Element::TEXTLINE
               || element->type() == Element::VOLTA
               ) {
                  SLine* hp  = static_cast<SLine*>(element);
                  SLine* nhp = static_cast<SLine*>(ne);
                  Element* e1;
                  Element* e2;
                  switch(hp->anchor()) {
                        case Spanner::ANCHOR_SEGMENT:
                              {
                              Segment* s1  = static_cast<Segment*>(hp->startElement());
                              Segment* s2  = static_cast<Segment*>(hp->endElement());
                              Measure* m1  = s1->measure();
                              Measure* m2  = s2->measure();
                              Measure* nm1 = score->tick2measure(m1->tick());
                              Measure* nm2 = score->tick2measure(m2->tick());
                              e1           = nm1->findSegment(s1->segmentType(), s1->tick());
                              e2           = nm2->findSegment(s2->segmentType(), s2->tick());
                              }
                              break;
                        case Spanner::ANCHOR_MEASURE:
                              {
                              Measure* m1 = static_cast<Measure*>(hp->startElement());
                              Measure* m2 = static_cast<Measure*>(hp->endElement());
                              e1          = score->tick2measure(m1->tick());
                              e2          = score->tick2measure(m2->tick());
                              }
                              break;
                        case Spanner::ANCHOR_NOTE:
                              {
                              Note* n1 = static_cast<Note*>(hp->startElement());
                              Note* n2 = static_cast<Note*>(hp->endElement());
                              e1       = n1;          // TODO: parts
                              e2       = n2;
                              }
                              break;
                        default:
                              abort();
                        }
                  nhp->setStartElement(e1);
                  nhp->setEndElement(e2);
                  nhp->setParent(e1);
                  undo(new AddElement(nhp));
                  }
            else if (element->type() == Element::BREATH) {
                  Breath* breath   = static_cast<Breath*>(element);
                  int tick         = breath->segment()->tick();
                  Measure* m       = score->tick2measure(tick);
                  Segment* seg     = m->undoGetSegment(Segment::SegBreath, tick);
                  Breath* nbreath  = static_cast<Breath*>(ne);
                  int ntrack       = staffIdx * VOICES + nbreath->voice();
                  nbreath->setScore(score);
                  nbreath->setTrack(ntrack);
                  nbreath->setParent(seg);
                  undo(new AddElement(nbreath));
                  }
            else
                  qDebug("undoAddElement: unhandled: <%s>", element->name());
            }
      }

//---------------------------------------------------------
//   undoAddGrace
//---------------------------------------------------------

void Score::undoAddGrace(Chord* chord, Segment* s, bool behind)
      {
      QList<Staff*> staffList;
      Staff* ostaff = chord->staff();
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);
      int tick = s->tick();
      if (behind)
            tick += chord->duration().ticks();

      foreach(Staff* staff, staffList) {
            //
            // search for segment s
            //
            Score* score = staff->score();
            Measure* m;
            Segment* ss;
            if (score == this) {
                  m   = s->measure();
                  ss  = s;
                  }
            else {
                  m   = score->tick2measure(tick);
                  ss  = m->findSegment(Segment::SegChordRest, tick);
                  }
            // always create new segment for grace note:
            Segment* seg = new Segment(m, Segment::SegGrace, tick);
            if (behind) {
                  seg->setNext(ss->next());
                  seg->setPrev(ss);
                  }
            else {
                  seg->setNext(ss);
                  seg->setPrev(ss->prev());
                  }
            score->undoAddElement(seg);

            Chord* nc = (staff == ostaff) ? chord : static_cast<Chord*>(chord->linkedClone());
            nc->setScore(score);
            int staffIdx = score->staffIdx(staff);
            int ntrack = staffIdx * VOICES + chord->voice();
            nc->setTrack(ntrack);
            nc->setParent(seg);
            // setTpcFromPitch needs to know the note tick position
            foreach(Note* note, nc->notes()) {
                  if (note->tpc() == INVALID_TPC)
                        note->setTpcFromPitch();
                  }
            undo(new AddElement(nc));
            score->updateAccidentals(m, staffIdx);
            }
      }

//---------------------------------------------------------
//   undoAddCR
//---------------------------------------------------------

void Score::undoAddCR(ChordRest* cr, Measure* measure, int tick)
      {
      Q_ASSERT(cr->type() != Element::CHORD || !(static_cast<Chord*>(cr)->notes()).isEmpty());

      QList<Staff*> staffList;
      Staff* ostaff = cr->staff();
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);
      Segment::SegmentType segmentType;
      if ((cr->type() == Element::CHORD) && (((Chord*)cr)->noteType() != NOTE_NORMAL))
            segmentType = Segment::SegGrace;
      else
            segmentType = Segment::SegChordRest;
      foreach(Staff* staff, staffList) {
            Score* score = staff->score();
            Measure* m   = (score == this) ? measure : score->tick2measure(tick);
            // always create new segment for grace note:
            Segment* seg = 0;
            if (segmentType != Segment::SegGrace)
                  seg = m->findSegment(segmentType, tick);
            if (seg == 0) {
                  seg = new Segment(m, segmentType, tick);
                  score->undoAddElement(seg);
                  }
            ChordRest* newcr = (staff == ostaff) ? cr : static_cast<ChordRest*>(cr->linkedClone());
            newcr->setScore(score);
            int staffIdx = score->staffIdx(staff);
            int ntrack = staffIdx * VOICES + cr->voice();
            newcr->setTrack(ntrack);
            newcr->setParent(seg);

            if (newcr->type() == Element::CHORD) {
                  Chord* chord = static_cast<Chord*>(newcr);
                  // setTpcFromPitch needs to know the note tick position
                  foreach(Note* note, chord->notes()) {
                        if (note->tpc() == INVALID_TPC)
                              note->setTpcFromPitch();
                        }
                  }
            if (cr->tuplet()) {
                  Tuplet* nt = 0;
                  Tuplet* t = cr->tuplet();
                  if (staff == ostaff)
                        nt = t;
                  else {
                        //if (t->elements().isEmpty() || t->elements().front() == cr) {
                        if (t->elements().front() == cr) {
                              nt = static_cast<Tuplet*>(t->linkedClone());
                              nt->setScore(score);
                              }
                        else {
                              LinkedElements* le = cr->tuplet()->links();
                              foreach(Element* e, *le) {
                                    if (e->score() == score)
                                          nt = static_cast<Tuplet*>(e);
                                    }
                              }
                        }
                  newcr->setTuplet(nt);
                  }
            undo(new AddElement(newcr));
            score->updateAccidentals(m, staffIdx);
            }
      }

//---------------------------------------------------------
//   undoRemoveElement
//---------------------------------------------------------

void Score::undoRemoveElement(Element* element)
      {
      QList<Segment*> segments;
      foreach (Element* e, element->linkList()) {
            undo(new RemoveElement(e));
            if (e->type() == Element::KEYSIG)                  // TODO: should be done in undo()/redo()
                  e->score()->cmdUpdateNotes();
            if (!e->isChordRest() && e->parent() && (e->parent()->type() == Element::SEGMENT)) {
                  Segment* s = static_cast<Segment*>(e->parent());
                  if (!segments.contains(s))
                        segments.append(s);
                  }
            }
      foreach(Segment* s, segments) {
            if (s->isEmpty())
                  undo(new RemoveElement(s));
            }
      }

//---------------------------------------------------------
//   undoChangeTuning
//---------------------------------------------------------

void Score::undoChangeTuning(Note* n, qreal v)
      {
      undoChangeProperty(n, P_TUNING, v);
      }

void Score::undoChangeUserMirror(Note* n, MScore::DirectionH d)
      {
      undoChangeProperty(n, P_MIRROR_HEAD, d);
      }

//---------------------------------------------------------
//   undoChangePageFormat
//---------------------------------------------------------

void Score::undoChangePageFormat(PageFormat* p, qreal v, int pageOffset)
      {
      undo(new ChangePageFormat(this, p, v, pageOffset));
      }

//---------------------------------------------------------
//   AddElement
//---------------------------------------------------------

AddElement::AddElement(Element* e)
      {
      element = e;
      }

//---------------------------------------------------------
//   undoRemoveTuplet
//---------------------------------------------------------

static void undoRemoveTuplet(DurationElement* cr)
      {
      if (cr->tuplet()) {
            cr->tuplet()->remove(cr);
            if (cr->tuplet()->elements().isEmpty())
                  undoRemoveTuplet(cr->tuplet());
            }
      }

//---------------------------------------------------------
//   undoAddTuplet
//---------------------------------------------------------

static void undoAddTuplet(DurationElement* cr)
      {
      if (cr->tuplet()) {
            cr->tuplet()->add(cr);
            if (cr->tuplet()->elements().size() == 1)
                  undoAddTuplet(cr->tuplet());
            }
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void AddElement::undo()
      {
      element->score()->removeElement(element);
      if (element->type() == Element::TIE) {
            Tie* tie = static_cast<Tie*>(element);
            Measure* m1 = tie->startNote()->chord()->measure();
            Measure* m2 = 0;
            if(tie->endNote())
                  m2 = tie->endNote()->chord()->measure();

            if (m1 != m2)
                  tie->score()->cmdUpdateNotes();
            else
                  tie->score()->updateAccidentals(m1, tie->staffIdx());
            }
      else if (element->isChordRest()) {
            undoRemoveTuplet(static_cast<ChordRest*>(element));
            }
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void AddElement::redo()
      {
      element->score()->addElement(element);
      if (element->type() == Element::TIE) {
            Tie* tie = static_cast<Tie*>(element);
            Measure* m1 = tie->startNote()->chord()->measure();
            Measure* m2 = tie->endNote() ? tie->endNote()->chord()->measure() : 0;

            if (m2 && (m1 != m2))
                  tie->score()->cmdUpdateNotes();
            else
                  tie->score()->updateAccidentals(m1, tie->staffIdx());
            }
      else if (element->isChordRest()) {
            undoAddTuplet(static_cast<ChordRest*>(element));
            }
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

#ifdef DEBUG_UNDO
const char* AddElement::name() const
      {
      static char buffer[64];
      sprintf(buffer, "Add: %s", element->name());
      return buffer;
      }
#endif


//---------------------------------------------------------
//   RemoveElement
//---------------------------------------------------------

RemoveElement::RemoveElement(Element* e)
      {
      element = e;

      Score* score = element->score();
      if (element->isChordRest()) {
            // remove any slurs pointing to this chor/rest
            ChordRest* cr = static_cast<ChordRest*>(element);
            for (Spanner* s = cr->spannerFor(); s; s = s->next())
                  score->undoRemoveElement(s);
            for (Spanner* s = cr->spannerBack(); s; s = s->next())
                  score->undoRemoveElement(s);
            if (cr->tuplet() && cr->tuplet()->elements().empty())
                  score->undoRemoveElement(cr->tuplet());
            if (e->type() == Element::CHORD) {
                  Chord* chord = static_cast<Chord*>(e);
                  foreach(Note* note, chord->notes()) {
                        if (note->tieFor() && note->tieFor()->endNote())
                              note->tieFor()->endNote()->setTieBack(0);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void RemoveElement::undo()
      {
      element->score()->addElement(element);
      if (element->isChordRest()) {
            if (element->type() == Element::CHORD) {
                  Chord* chord = static_cast<Chord*>(element);
                  foreach(Note* note, chord->notes()) {
                        if (note->tieBack())
                              note->tieBack()->setEndNote(note);
                        }
                  }
            undoAddTuplet(static_cast<ChordRest*>(element));
            }
      if (element->type() == Element::MEASURE)
            element->score()->setLayoutAll(true);    //DEBUG
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void RemoveElement::redo()
      {
      element->score()->removeElement(element);
      if (element->isChordRest())
            undoRemoveTuplet(static_cast<ChordRest*>(element));
      if (element->type() == Element::MEASURE)
            element->score()->setLayoutAll(true);    //DEBUG
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

#ifdef DEBUG_UNDO
const char* RemoveElement::name() const
      {
      static char buffer[64];
      sprintf(buffer, "Remove: %s", element->name());
      return buffer;
      }
#endif


//---------------------------------------------------------
//   ChangeConcertPitch
//---------------------------------------------------------

ChangeConcertPitch::ChangeConcertPitch(Score* s, bool v)
      {
      score = s;
      val   = v;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeConcertPitch::flip()
      {
      int oval = int(score->styleB(ST_concertPitch));
      score->style()->set(ST_concertPitch, val);
      val = oval;
      }

//---------------------------------------------------------
//   InsertPart
//---------------------------------------------------------

InsertPart::InsertPart(Part* p, int i)
      {
      part = p;
      idx  = i;
      }

void InsertPart::undo()
      {
      part->score()->removePart(part);
      }

void InsertPart::redo()
      {
      part->score()->insertPart(part, idx);
      }

//---------------------------------------------------------
//   RemovePart
//---------------------------------------------------------

RemovePart::RemovePart(Part* p, int i)
      {
      part = p;
      idx  = i;
      }

void RemovePart::undo()
      {
      part->score()->insertPart(part, idx);
      }

void RemovePart::redo()
      {
      part->score()->removePart(part);
      }

//---------------------------------------------------------
//   InsertStaff
//---------------------------------------------------------

InsertStaff::InsertStaff(Staff* p, int i)
      {
      staff = p;
      idx  = i;
      }

void InsertStaff::undo()
      {
      staff->score()->removeStaff(staff);
      }

void InsertStaff::redo()
      {
      staff->score()->insertStaff(staff, idx);
      }

//---------------------------------------------------------
//   RemoveStaff
//---------------------------------------------------------

RemoveStaff::RemoveStaff(Staff* p, int i)
      {
      staff = p;
      idx  = i;
      }

void RemoveStaff::undo()
      {
      staff->score()->insertStaff(staff, idx);
      }

void RemoveStaff::redo()
      {
      staff->score()->removeStaff(staff);
      }

//---------------------------------------------------------
//   InsertMStaff
//---------------------------------------------------------

InsertMStaff::InsertMStaff(Measure* m, MStaff* ms, int i)
      {
      measure = m;
      mstaff  = ms;
      idx     = i;
      }

void InsertMStaff::undo()
      {
      measure->removeMStaff(mstaff, idx);
      }

void InsertMStaff::redo()
      {
      measure->insertMStaff(mstaff, idx);
      }

//---------------------------------------------------------
//   RemoveMStaff
//---------------------------------------------------------

RemoveMStaff::RemoveMStaff(Measure* m, MStaff* ms, int i)
      {
      measure = m;
      mstaff  = ms;
      idx     = i;
      }

void RemoveMStaff::undo()
      {
      measure->insertMStaff(mstaff, idx);
      }

void RemoveMStaff::redo()
      {
      measure->removeMStaff(mstaff, idx);
      }

//---------------------------------------------------------
//   InsertMeasure
//---------------------------------------------------------

void InsertMeasure::undo()
      {
      Score* score = measure->score();
      score->remove(measure);
      score->addLayoutFlags(LAYOUT_FIX_TICKS);
      score->setLayoutAll(true);
      }

void InsertMeasure::redo()
      {
      Score* score = measure->score();
      score->addMeasure(measure, pos);
      score->addLayoutFlags(LAYOUT_FIX_TICKS);
      score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   SortStaves
//---------------------------------------------------------

SortStaves::SortStaves(Score* s, QList<int> l)
      {
      score = s;

      for(int i=0 ; i < l.size(); i++) {
            rlist.append(l.indexOf(i));
            }
      list  = l;
      }

void SortStaves::redo()
      {
      score->sortStaves(list);
      }

void SortStaves::undo()
      {
      score->sortStaves(rlist);
      }

//---------------------------------------------------------
//   ChangePitch
//---------------------------------------------------------

ChangePitch::ChangePitch(Note* _note, int _pitch, int _tpc, int l/*, int f, int s*/)
      {
      note  = _note;
      if (_note == 0)
            abort();
      pitch  = _pitch;
      tpc    = _tpc;
      line   = l;
//      fret   = f;
//      string = s;
      }

void ChangePitch::flip()
      {
      int f_pitch                 = note->pitch();
      int f_tpc                   = note->tpc();
      int f_line                  = note->line();

      // do not change unless necessary: setting note pitch triggers chord re-fretting on TABs
      // which triggers ChangePitch(), leading to recursion with negative side effects
      bool updateAccid = false;
      if (f_pitch != pitch || f_tpc != tpc) {
            updateAccid = true;
            note->setPitch(pitch, tpc);
            }
      if (f_line != line)
            note->setLine(line);

      pitch          = f_pitch;
      tpc            = f_tpc;
      line           = f_line;

      Score* score = note->score();
      if(updateAccid) {
            Chord* chord = note->chord();
            Measure* measure = chord->segment()->measure();
            score->updateAccidentals(measure, chord->staffIdx());
            }
      // score->setLayout(measure);
      score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   FlipNoteDotDirection
//---------------------------------------------------------

void FlipNoteDotDirection::flip()
      {
      note->setDotPosition(note->dotIsUp() ? MScore::DOWN : MScore::UP);
      }

//---------------------------------------------------------
//   ChangeElement
//---------------------------------------------------------

ChangeElement::ChangeElement(Element* oe, Element* ne)
      {
      oldElement = oe;
      newElement = ne;
      }

void ChangeElement::flip()
      {
//      qDebug("ChangeElement::flip() %s(%p) -> %s(%p) links %d",
//         oldElement->name(), oldElement, newElement->name(), newElement,
//         oldElement->links() ? oldElement->links()->size() : -1);

      LinkedElements* links = oldElement->links();
      if (links) {
            links->removeOne(oldElement);
            links->append(newElement);
            }

      Score* score = oldElement->score();
      if (oldElement->selected())
            score->deselect(oldElement);
      if (newElement->selected())
            score->select(newElement);
      if (oldElement->parent() == 0) {
            score->removeElement(oldElement);
            score->addElement(newElement);
            }
      else {
            oldElement->parent()->change(oldElement, newElement);
            }

      qSwap(oldElement, newElement);

      if (newElement->type() == Element::KEYSIG)
            newElement->staff()->setUpdateKeymap(true);
      else if (newElement->type() == Element::DYNAMIC)
            newElement->score()->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
      else if (newElement->type() == Element::TEMPO_TEXT) {
            TempoText* t = static_cast<TempoText*>(oldElement);
            score->setTempo(t->segment(), t->tempo());
            }
      if (newElement->isSegment()) {
            SpannerSegment* os = static_cast<SpannerSegment*>(oldElement);
            SpannerSegment* ns = static_cast<SpannerSegment*>(newElement);
            if (os->system())
                  os->system()->remove(os);
            if (ns->system())
                  ns->system()->add(ns);
            }
      score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   InsertStaves
//---------------------------------------------------------

InsertStaves::InsertStaves(Measure* m, int _a, int _b)
      {
      measure = m;
      a       = _a;
      b       = _b;
      }

void InsertStaves::undo()
      {
      measure->removeStaves(a, b);
      }

void InsertStaves::redo()
      {
      measure->insertStaves(a, b);
      }

//---------------------------------------------------------
//   RemoveStaves
//---------------------------------------------------------

RemoveStaves::RemoveStaves(Measure* m, int _a, int _b)
      {
      measure = m;
      a       = _a;
      b       = _b;
      }

void RemoveStaves::undo()
      {
      measure->insertStaves(a, b);
      }

void RemoveStaves::redo()
      {
      measure->removeStaves(a, b);
      }

//---------------------------------------------------------
//   ChangeKeySig
//---------------------------------------------------------

ChangeKeySig::ChangeKeySig(KeySig* _keysig, KeySigEvent _ks, bool sc, bool sn)
      {
      keysig = _keysig;
      ks     = _ks;
      showCourtesy = sc;
      showNaturals = sn;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeKeySig::flip()
      {
      KeySigEvent oe = keysig->keySigEvent();
      bool sc        = keysig->showCourtesy();
      bool sn        = keysig->showNaturals();

      keysig->setKeySigEvent(ks);
      keysig->setShowCourtesy(showCourtesy);
      keysig->setShowNaturals(showNaturals);
//      keysig->staff()->setKey(keysig->segment()->tick(), ks);

      showCourtesy = sc;
      showNaturals = sn;
      ks           = oe;

      keysig->score()->setLayoutAll(true);
      keysig->score()->cmdUpdateNotes();
      }

//---------------------------------------------------------
//   ChangeMeasureLen
//---------------------------------------------------------

ChangeMeasureLen::ChangeMeasureLen(Measure* m, Fraction l)
      {
      measure     = m;
      len         = l;
      }

void ChangeMeasureLen::flip()
      {
      Fraction oLen = measure->len();

      //
      // move EndBarLine and TimeSigAnnounce
      // to end of measure:
      //
      int endTick = measure->tick() + len.ticks();
      for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            if (segment->segmentType() != Segment::SegEndBarLine
               && segment->segmentType() != Segment::SegTimeSigAnnounce)
                  continue;
            segment->setTick(endTick);
            }
      measure->setLen(len);
//      measure->score()->addLayoutFlags(LAYOUT_FIX_TICKS); // we need to fix tick immediately!
      measure->score()->fixTicks();
      len = oLen;
      }

//---------------------------------------------------------
//   ChangeVoltaEnding
//---------------------------------------------------------

ChangeVoltaEnding::ChangeVoltaEnding(Volta* v, const QList<int>& l)
      {
      volta = v;
      list  = l;
      }

void ChangeVoltaEnding::flip()
      {
      QList<int> l = volta->endings();
      volta->setEndings(list);
      list = l;
      }

//---------------------------------------------------------
//   ChangeVoltaText
//---------------------------------------------------------

ChangeVoltaText::ChangeVoltaText(Volta* v, const QString& t)
      {
      volta = v;
      text  = t;
      }

void ChangeVoltaText::flip()
      {
      QString s = volta->text();
      volta->setText(text);
      text = s;
      }

//---------------------------------------------------------
//   ChangeChordRestSize
//---------------------------------------------------------

ChangeChordRestSize::ChangeChordRestSize(ChordRest* _cr, bool _small)
      {
      cr = _cr;
      small = _small;
      }

void ChangeChordRestSize::flip()
      {
      bool s = cr->small();
      cr->setSmall(small);
      small = s;
      }

//---------------------------------------------------------
//   ChangeChordNoStem
//---------------------------------------------------------

ChangeChordNoStem::ChangeChordNoStem(Chord* c, bool f)
      {
      chord = c;
      noStem = f;
      }

void ChangeChordNoStem::flip()
      {
      bool ns = chord->noStem();
      chord->setNoStem(noStem);
      noStem = ns;
      }

//---------------------------------------------------------
//   ChangeEndBarLineType
//---------------------------------------------------------

ChangeEndBarLineType::ChangeEndBarLineType(Measure* m, BarLineType st)
      {
      measure = m;
      subtype = st;
      }

void ChangeEndBarLineType::flip()
      {
      BarLineType typ = measure->endBarLineType();
      measure->setEndBarLineType(subtype, false);
      subtype = typ;
      }

//---------------------------------------------------------
//   ChangeBarLineSpan
//---------------------------------------------------------

ChangeBarLineSpan::ChangeBarLineSpan(Staff* _staff, int _span, int _spanFrom, int _spanTo)
      {
      staff       = _staff;
      span        = _span;
      spanFrom    = _spanFrom;
      spanTo      = _spanTo;
      }

void ChangeBarLineSpan::flip()
      {
      int nspan         = staff->barLineSpan();
      int nspanFrom     = staff->barLineFrom();
      int nspanTo       = staff->barLineTo();
      staff->setBarLineSpan(span);
      staff->setBarLineFrom(spanFrom);
      staff->setBarLineTo(spanTo);
      span        = nspan;
      spanFrom    = nspanFrom;
      spanTo      = nspanTo;
      // all bar lines of this staff across the whole score needs to be re-laid out and re-drawn
      staff->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   ChangeSingleBarLineSpan
//---------------------------------------------------------

ChangeSingleBarLineSpan::ChangeSingleBarLineSpan(BarLine* _barLine, int _span, int _spanFrom, int _spanTo)
      {
      barLine     = _barLine;
      span        = _span;
      spanFrom    = _spanFrom;
      spanTo      = _spanTo;
      }

void ChangeSingleBarLineSpan::flip()
      {
      barLine->score()->addRefresh(barLine->abbox()); // area of this bar line needs redraw
      int nspan         = barLine->span();
      int nspanFrom     = barLine->spanFrom();
      int nspanTo       = barLine->spanTo();
      barLine->setSpan(span);
      barLine->setSpanFrom(spanFrom);
      barLine->setSpanTo(spanTo);
      barLine->setCustomSpan(true);
      span        = nspan;
      spanFrom    = nspanFrom;
      spanTo      = nspanTo;
      barLine->layout();                              // update bbox
      // re-create bar lines for other staves, if span of this bar line decreased
      if (barLine->parent() && barLine->parent()->type() == Element::SEGMENT)
            (static_cast<Segment*>(barLine->parent()))->measure()->createEndBarLines();
      barLine->score()->addRefresh(barLine->abbox()); // new area of this bar line needs redraw
      }

//---------------------------------------------------------
//   ChangeSlurOffsets
//---------------------------------------------------------

void ChangeSlurOffsets::flip()
      {
      for (int i = 0; i < 4; ++i) {
            QPointF f = slur->slurOffset(i);
            slur->setSlurOffset(i, off[i]);
            off[i] = f;
            }
      }

//---------------------------------------------------------
//   TransposeHarmony
//---------------------------------------------------------

TransposeHarmony::TransposeHarmony(Harmony* h, int rtpc, int btpc)
      {
      harmony = h;
      rootTpc = rtpc;
      baseTpc = btpc;
      }

void TransposeHarmony::flip()
      {
      int baseTpc1 = harmony->baseTpc();
      int rootTpc1 = harmony->rootTpc();
      harmony->setBaseTpc(baseTpc);
      harmony->setRootTpc(rootTpc);
      harmony->render();
      rootTpc = rootTpc1;
      baseTpc = baseTpc1;
      }

//---------------------------------------------------------
//   ExchangeVoice
//---------------------------------------------------------

ExchangeVoice::ExchangeVoice(Measure* m, int _val1, int _val2, int _staff1, int _staff2)
      {
      measure = m;
      val1    = _val1;
      val2    = _val2;
      staff1  = _staff1;
      staff2  = _staff2;
      }

void ExchangeVoice::undo()
      {
      measure->exchangeVoice(val2, val1, staff1, staff2);
      }

void ExchangeVoice::redo()
      {
      measure->exchangeVoice(val1, val2, staff1, staff2);
      }

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

ChangeInstrumentShort::ChangeInstrumentShort(int _tick, Part* p, QList<StaffNameDoc> t)
      {
      tick = _tick;
      part = p;
      text = t;
      }

void ChangeInstrumentShort::flip()
      {
      QList<StaffNameDoc> s = part->shortNames(tick);
      part->setShortNames(text, tick);
      text = s;
      part->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   ChangeInstrumentLong
//---------------------------------------------------------

ChangeInstrumentLong::ChangeInstrumentLong(int _tick, Part* p, QList<StaffNameDoc> t)
      {
      tick = _tick;
      part = p;
      text = t;
      }

void ChangeInstrumentLong::flip()
      {
      QList<StaffNameDoc> s = part->longNames(tick);
      part->setLongNames(text, tick);
      text = s;
      part->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   ChangeChordRestLen
//---------------------------------------------------------

ChangeChordRestLen::ChangeChordRestLen(ChordRest* c, const TDuration& _d)
   : cr(c), d(_d)
      {
      }

void ChangeChordRestLen::flip()
      {
      TDuration od = cr->durationType();
      cr->setDurationType(d);
      if (d == TDuration::V_MEASURE) {
            cr->setDuration(cr->measure()->len());
            }
      else {
            cr->setDuration(d.fraction());
            }
      d   = od;
      cr->score()->setLayout(cr->measure());
      }

//---------------------------------------------------------
//   MoveElement
//---------------------------------------------------------

MoveElement::MoveElement(Element* e, const QPointF& o)
      {
      element = e;
      offset = o;
      }

void MoveElement::flip()
      {
      QPointF po = element->userOff();
      element->score()->addRefresh(element->canvasBoundingRect());
      element->setUserOff(offset);
      if (element->type() == Element::REST)
            element->layout();            // ledgerline could change
      element->score()->addRefresh(element->canvasBoundingRect());
      offset = po;
      }

//---------------------------------------------------------
//   ChangeBracketSpan
//---------------------------------------------------------

ChangeBracketSpan::ChangeBracketSpan(Staff* s, int c, int sp)
      {
      staff  = s;
      column = c;
      span   = sp;
      }

void ChangeBracketSpan::flip()
      {
      int oSpan  = staff->bracketSpan(column);
      staff->setBracketSpan(column, span);
      span = oSpan;
      }

//---------------------------------------------------------
//   EditText::undo
//---------------------------------------------------------

void EditText::undo()
      {
      if (!text->styled()) {
            for (int i = 0; i < undoLevel; ++i)
                  text->undo();
            }
      undoRedo();
      }

//---------------------------------------------------------
//   EditText::redo
//---------------------------------------------------------

void EditText::redo()
      {
      if (!text->styled()) {
            for (int i = 0; i < undoLevel; ++i)
                  text->redo();
            }
      undoRedo();
      }

//---------------------------------------------------------
//   EditText::undoRedo
//---------------------------------------------------------

void EditText::undoRedo()
      {
      if (text->styled()) {
            QString s = text->text();
            text->setText(oldText);
            oldText = s;
            }
      else {
            if (text->type() == Element::TEMPO_TEXT) {
                  TempoText* tt = static_cast<TempoText*>(text);
                  tt->score()->setTempo(tt->segment(), tt->tempo());
                  }
            }
      text->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   ChangePatch
//---------------------------------------------------------

void ChangePatch::flip()
      {
      MidiPatch op;
      op.prog          = channel->program;
      op.bank          = channel->bank;
      op.synti         = channel->synti;

      channel->program = patch.prog;
      channel->bank    = patch.bank;
      channel->synti   = patch.synti;
      patch            = op;

      if (MScore::seq == 0)
            return;

      Event event(ME_CONTROLLER);
      event.setChannel(channel->channel);

      int hbank = (channel->bank >> 7) & 0x7f;
      int lbank = channel->bank & 0x7f;

      event.setController(CTRL_HBANK);
      event.setValue(hbank);
      MScore::seq->sendEvent(event);

      event.setController(CTRL_LBANK);
      event.setValue(lbank);
      MScore::seq->sendEvent(event);

      event.setController(CTRL_PROGRAM);
      event.setValue(channel->program);

      MScore::seq->sendEvent(event);
      }

//---------------------------------------------------------
//   ChangePageFormat
//---------------------------------------------------------

ChangePageFormat::ChangePageFormat(Score* cs, PageFormat* p, qreal s, int po)
      {
      score      = cs;
      pf         = new PageFormat;
      pf->copy(*p);
      spatium    = s;
      pageOffset = po;
      }

ChangePageFormat::~ChangePageFormat()
      {
      delete pf;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangePageFormat::flip()
      {
      PageFormat f;
      f.copy(*(score->pageFormat()));
      qreal os    = score->spatium();
      int po       = score->pageNumberOffset();

      score->setPageFormat(*pf);
      if (os != spatium) {
            score->setSpatium(spatium);
            score->spatiumChanged(os, spatium);
            }
      score->setPageNumberOffset(pageOffset);
      score->setLayoutAll(true);

      pf->copy(f);
      spatium = os;
      pageOffset = po;
      }

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

ChangeStaff::ChangeStaff(Staff* _staff, bool _small, bool _invisible, StaffType* st)
      {
      staff     = _staff;
      small     = _small;
      invisible = _invisible;
      staffType = st;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStaff::flip()
      {
      bool invisibleChanged = staff->invisible() != invisible;
      bool typeChanged      = staff->staffType() != staffType;

      int oldSmall      = staff->small();
      bool oldInvisible = staff->invisible();
      StaffType* st     = staff->staffType();

      staff->setSmall(small);
      staff->setInvisible(invisible);
      staff->setStaffType(staffType);

      small     = oldSmall;
      invisible = oldInvisible;
      staffType = st;

      if (invisibleChanged || typeChanged) {
            Score* score = staff->score();
            int staffIdx = score->staffIdx(staff);
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  MStaff* mstaff = m->mstaff(staffIdx);
                  mstaff->lines->setVisible(!staff->invisible());
                  }
            // if staff type changed, the whole staff across the entire score needs re-laying out
            if(typeChanged)
                  score->setLayoutAll(true);
            }
      staff->score()->rebuildMidiMapping();
      staff->score()->setPlaylistDirty(true);
      }

//---------------------------------------------------------
//   ChangePart
//---------------------------------------------------------

ChangePart::ChangePart(Part* _part, const Instrument& i, const QString& s)
      {
      instrument = i;
      part       = _part;
      partName   = s;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangePart::flip()
      {
      Instrument oi = *part->instr();
      QString s     = part->partName();
      part->setInstrument(instrument);
      part->setPartName(partName);
      partName   = s;
      instrument = oi;
      part->score()->rebuildMidiMapping();
      part->score()->setInstrumentsChanged(true);
      part->score()->setPlaylistDirty(true);
      }

//---------------------------------------------------------
//   ChangeTextStyle
//---------------------------------------------------------

ChangeTextStyle::ChangeTextStyle(Score* s, const TextStyle& st)
      {
      score = s;
      style = st;
      }

//---------------------------------------------------------
//   updateTextStyle
//---------------------------------------------------------

static void updateTextStyle(void* a, Element* e)
      {
      QString s = *(QString*)a;
      if (e->isText()) {
            Text* text = static_cast<Text*>(e);
            if (text->styled() && text->textStyle().name() == s) {
                  text->setTextStyle(text->score()->textStyle(s));
                  text->styleChanged();
                  }
            }
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeTextStyle::flip()
      {
      TextStyle os = score->style()->textStyle(style.name());
      score->style()->setTextStyle(style);
      QString s(style.name());
      score->scanElements(&s, updateTextStyle);
      style = os;
      score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   AddTextStyle::undo
//---------------------------------------------------------

void AddTextStyle::undo()
      {
      score->style()->removeTextStyle(style);
      }

//---------------------------------------------------------
//   AddTextStyle::redo
//---------------------------------------------------------

void AddTextStyle::redo()
      {
      score->style()->addTextStyle(style);
      }

//---------------------------------------------------------
//   ChangeStretch
//---------------------------------------------------------

ChangeStretch::ChangeStretch(Measure* m, qreal s)
   : measure(m), stretch(s)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStretch::flip()
      {
      qreal oStretch = measure->userStretch();
      measure->setUserStretch(stretch);
      measure->score()->setLayoutAll(true);
      stretch = oStretch;
      }

//---------------------------------------------------------
//   ChangeStyle
//---------------------------------------------------------

ChangeStyle::ChangeStyle(Score* s, const MStyle& st)
   : score(s), style(st)
      {
      }

static void updateTextStyle2(void*, Element* e)
      {
      if (!e->isText())
            return;

      if (e->type() == Element::HARMONY)
            static_cast<Harmony*>(e)->render();
      else {
#if 0 // TODO?
            Text* text = static_cast<Text*>(e);
            if (text->styled()) {
                  QString sn = text->styleName();
                  int st = text->score()->style()->textStyleType(sn);
                  if (st == TEXT_STYLE_INVALID) {
                        //
                        // this was probably a user defined text style
                        // which is not part of the new style file
                        //
                        text->setStyled(false);
                        }
                  else {
                        text->setText(text->getText());     // destroy formatting
                        }
                  }
#endif
            }
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStyle::flip()
      {
      MStyle tmp = *score->style();

      if (score->styleB(ST_concertPitch) != style.valueB(ST_concertPitch))
            score->cmdConcertPitchChanged(style.valueB(ST_concertPitch), true);

      score->setStyle(style);
      score->scanElements(0, updateTextStyle2);
      score->setLayoutAll(true);

      style = tmp;
      }

//---------------------------------------------------------
//   ChangeChordStaffMove
//---------------------------------------------------------

ChangeChordStaffMove::ChangeChordStaffMove(Chord* c, int v)
   : chord(c), staffMove(v)
      {
      }

void ChangeChordStaffMove::flip()
      {
      int v = chord->staffMove();
      chord->setStaffMove(staffMove);
      chord->score()->updateAccidentals(chord->measure(), chord->staffIdx());
      chord->score()->setLayoutAll(true);
      staffMove = v;
      }

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

ChangeVelocity::ChangeVelocity(Note* n, MScore::ValueType t, int o)
   : note(n), veloType(t), veloOffset(o)
      {
      }

void ChangeVelocity::flip()
      {
      MScore::ValueType t = note->veloType();
      int o       = note->veloOffset();
      note->setVeloType(veloType);
      note->setVeloOffset(veloOffset);
      veloType   = t;
      veloOffset = o;
      }

//---------------------------------------------------------
//   ChangeMStaffProperties
//---------------------------------------------------------

ChangeMStaffProperties::ChangeMStaffProperties(MStaff* ms, bool v, bool s)
   : mstaff(ms), visible(v), slashStyle(s)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeMStaffProperties::flip()
      {
      bool v = mstaff->visible();
      bool s = mstaff->slashStyle();
      mstaff->setVisible(visible);
      mstaff->setSlashStyle(slashStyle);
      visible    = v;
      slashStyle = s;
      }

//---------------------------------------------------------
//   ChangeMeasureProperties
//---------------------------------------------------------

ChangeMeasureProperties::ChangeMeasureProperties(
   Measure* m,
   bool _bmm,
   int rc,
   qreal s,
   int o,
   bool ir
   ) :
   measure(m),
   breakMM(_bmm),
   repeatCount(rc),
   stretch(s),
   noOffset(o),
   irregular(ir)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeMeasureProperties::flip()
      {
      bool a   = measure->breakMultiMeasureRest();
      int r    = measure->repeatCount();
      qreal s = measure->userStretch();
      int o    = measure->noOffset();
      bool ir  = measure->irregular();

      measure->setBreakMultiMeasureRest(breakMM);
      measure->setRepeatCount(repeatCount);
      measure->setUserStretch(stretch);
      Score* score = measure->score();
      if (o != noOffset || ir != irregular) {
            measure->setNoOffset(noOffset);
            measure->setIrregular(irregular);
            score->renumberMeasures();
            }
      breakMM     = a;
      repeatCount = r;
      stretch     = s;
      noOffset    = o;
      irregular   = ir;

      score->addLayoutFlags(LAYOUT_FIX_TICKS);
      score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   ChangeTimesig
//---------------------------------------------------------

ChangeTimesig::ChangeTimesig(TimeSig * _timesig, bool sc, const Fraction& f1,
   const Fraction& f2, QString numStr, QString denStr, TimeSigType st)
      {
      timesig           = _timesig;
      showCourtesy      = sc;
      sig               = f1;
      stretch           = f2;
      numeratorString   = numStr;
      denominatorString = denStr;
      subtype           = st;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeTimesig::flip()
      {
      timesig->score()->addRefresh(timesig->abbox());
      bool sc           = timesig->showCourtesySig();
      Fraction f1       = timesig->sig();
      Fraction f2       = timesig->stretch();
      QString numStr    = timesig->numeratorString();
      QString denStr    = timesig->denominatorString();
      TimeSigType st    = timesig->timeSigType();

      timesig->setShowCourtesySig(showCourtesy);
      timesig->setSig(sig, subtype);
      timesig->setStretch(stretch);
      timesig->setNumeratorString(numeratorString);
      timesig->setDenominatorString(denominatorString);

      showCourtesy      = sc;
      sig               = f1;
      stretch           = f2;
      numeratorString   = numStr;
      denominatorString = denStr;
      subtype           = st;

      timesig->layout();
      timesig->score()->addRefresh(timesig->abbox());
      }

//---------------------------------------------------------
//   RemoveMeasures
//---------------------------------------------------------

RemoveMeasures::RemoveMeasures(Measure* m1, Measure* m2)
   : fm(m1), lm(m2)
      {
      }

//---------------------------------------------------------
//   undo
//    insert back measures
//---------------------------------------------------------

void RemoveMeasures::undo()
      {
      fm->score()->measures()->insert(fm, lm);
      fm->score()->fixTicks();
      }

//---------------------------------------------------------
//   redo
//    remove measures
//---------------------------------------------------------

void RemoveMeasures::redo()
      {
      fm->score()->measures()->remove(fm, lm);
      fm->score()->fixTicks();
      }

//---------------------------------------------------------
//   undo
//    insert back measures
//---------------------------------------------------------

void InsertMeasures::undo()
      {
      fm->score()->measures()->remove(fm, lm);
      fm->score()->fixTicks();
      }

//---------------------------------------------------------
//   redo
//    remove measures
//---------------------------------------------------------

void InsertMeasures::redo()
      {
      fm->score()->measures()->insert(fm, lm);
      fm->score()->fixTicks();
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeImage::flip()
      {
      bool _lockAspectRatio = image->lockAspectRatio();
      bool _autoScale       = image->autoScale();
      int  _z               = image->z();
      image->setLockAspectRatio(lockAspectRatio);
      image->setAutoScale(autoScale);
      image->setZ(z);
      lockAspectRatio = _lockAspectRatio;
      autoScale       = _autoScale;
      z               = _z;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeHairpin::flip()
      {
      int vc        = hairpin->veloChange();
      Element::DynamicRange t = hairpin->dynRange();
      bool dg       = hairpin->diagonal();
      hairpin->setVeloChange(veloChange);
      hairpin->setDynRange(dynRange);
      hairpin->setDiagonal(diagonal);
      veloChange = vc;
      dynRange   = t;
      diagonal   = dg;
      hairpin->score()->updateHairpin(hairpin);
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeDuration::flip()
      {
      Fraction od = cr->duration();
      cr->setDuration(d);
      d = od;
      }

//---------------------------------------------------------
//   AddExcerpt::undo
//---------------------------------------------------------

void AddExcerpt::undo()
      {
      score->parentScore()->removeExcerpt(score);
      score->parentScore()->setExcerptsChanged(true);
      }

//---------------------------------------------------------
//   AddExcerpt::redo
//---------------------------------------------------------

void AddExcerpt::redo()
      {
      score->parentScore()->addExcerpt(score);
      score->parentScore()->setExcerptsChanged(true);
      }

//---------------------------------------------------------
//   RemoveExcerpt::undo()
//---------------------------------------------------------

void RemoveExcerpt::undo()
      {
      score->parentScore()->addExcerpt(score);
      score->parentScore()->setExcerptsChanged(true);
      }

//---------------------------------------------------------
//   RemoveExcerpt::redo()
//---------------------------------------------------------

void RemoveExcerpt::redo()
      {
      score->parentScore()->removeExcerpt(score);
      score->parentScore()->setExcerptsChanged(true);
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeBend::flip()
      {
      QList<PitchValue> pv = bend->points();
      bend->setPoints(points);
      points = pv;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeTremoloBar::flip()
      {
      QList<PitchValue> pv = bend->points();
      bend->setPoints(points);
      points = pv;
      }

//---------------------------------------------------------
//   ChangeNoteEvents::flip
//---------------------------------------------------------

void ChangeNoteEvents::flip()
      {
/*TODO:      QList<NoteEvent*> e = chord->playEvents();
      chord->setPlayEvents(events);
      events = e;
      */
      }

//---------------------------------------------------------
//   undoChangeBarLine
//---------------------------------------------------------

void Score::undoChangeBarLine(Measure* m, BarLineType barType)
      {
      foreach(Score* s, scoreList()) {
            Measure* measure = s->tick2measure(m->tick());
            Measure* nm      = m->nextMeasure();
            switch(barType) {
                  case END_BAR:
                  case NORMAL_BAR:
                  case DOUBLE_BAR:
                  case BROKEN_BAR:
                  case DOTTED_BAR:
                        {
                        s->undoChangeProperty(measure, P_REPEAT_FLAGS, measure->repeatFlags() & ~RepeatEnd);
                        if (nm)
                              s->undoChangeProperty(nm, P_REPEAT_FLAGS, nm->repeatFlags() & ~RepeatStart);
                        s->undoChangeEndBarLineType(measure, barType);
                        measure->setEndBarLineGenerated (false);
                        }
                        break;
                  case START_REPEAT:
                        s->undoChangeProperty(measure, P_REPEAT_FLAGS, measure->repeatFlags() | RepeatStart);
                        break;
                  case END_REPEAT:
                        s->undoChangeProperty(measure, P_REPEAT_FLAGS, measure->repeatFlags() | RepeatEnd);
                        if (nm)
                              s->undoChangeProperty(nm, P_REPEAT_FLAGS, nm->repeatFlags() & ~RepeatStart);
                        break;
                  case END_START_REPEAT:
                        s->undoChangeProperty(measure, P_REPEAT_FLAGS, measure->repeatFlags() | RepeatEnd);
                        if (nm)
                              s->undoChangeProperty(nm, P_REPEAT_FLAGS, nm->repeatFlags() | RepeatStart);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   ChangeInstrument::flip
//---------------------------------------------------------

void ChangeInstrument::flip()
      {
      Instrument oi = is->instrument();
      is->setInstrument(instrument);

      is->staff()->part()->setInstrument(instrument, is->segment()->tick());
      is->score()->rebuildMidiMapping();
      is->score()->setInstrumentsChanged(true);
      is->score()->setLayoutAll(true);
      instrument = oi;
      }

//---------------------------------------------------------
//   ChangeBoxProperties
//---------------------------------------------------------

ChangeBoxProperties::ChangeBoxProperties(Box* box,
   qreal marginLeft, qreal marginTop, qreal marginRight, qreal marginBottom,
   Spatium height, Spatium width, qreal tg, qreal bg)
      {
      _box              = box;
      _marginLeft       = marginLeft;
      _marginTop        = marginTop;
      _marginRight      = marginRight;
      _marginBottom     = marginBottom;
      _height           = height;
      _width            = width;
      _topGap           = tg;
      _bottomGap        = bg;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeBoxProperties::flip()
      {
      // flip margins
      qreal marginLeft       = _box->leftMargin();
      qreal marginTop        = _box->topMargin();
      qreal marginRight      = _box->rightMargin();
      qreal marginBottom     = _box->bottomMargin();
      qreal tg               = _box->topGap();
      qreal bg               = _box->bottomGap();

      _box->setLeftMargin  (_marginLeft);
      _box->setRightMargin (_marginRight);
      _box->setTopMargin   (_marginTop);
      _box->setBottomMargin(_marginBottom);
      _box->setTopGap      (_topGap);
      _box->setBottomGap   (_bottomGap);

      _marginLeft       = marginLeft;
      _marginTop        = marginTop;
      _marginRight      = marginRight;
      _marginBottom     = marginBottom;
      _topGap           = tg;
      _bottomGap        = bg;

      // according to box type, flip either height or width (or none)
      Spatium val;
      if (_box->type() == Element::VBOX) {
            val = _box->boxHeight();
            _box->setBoxHeight(_height);
            _height = val;
            }
      if (_box->type() == Element::HBOX) {
            val = _box->boxWidth();
            _box->setBoxWidth(_width);
            _width = val;
            }
      }

//---------------------------------------------------------
//   undoSwapCR
//---------------------------------------------------------

void Score::undoSwapCR(ChordRest* cr1, ChordRest* cr2)
      {
      undo(new SwapCR(cr1, cr2));
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void SwapCR::flip()
      {
      Segment* s1 = cr1->segment();
      Segment* s2 = cr2->segment();
      int track = cr1->track();

      Element* cr = s1->element(track);
      s1->setElement(track, s2->element(track));
      s2->setElement(track, cr);
      cr1->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   ChangeClefType
//---------------------------------------------------------

ChangeClefType::ChangeClefType(Clef* c, ClefType cl, ClefType tc)
      {
      clef            = c;
      concertClef     = cl;
      transposingClef = tc;
      }

//---------------------------------------------------------
//   ChangeClefType::flip
//---------------------------------------------------------

void ChangeClefType::flip()
      {
      ClefType ocl = clef->concertClef();
      ClefType otc = clef->transposingClef();

      clef->setConcertClef(concertClef);
      clef->setTransposingClef(transposingClef);
      clef->setClefType(clef->score()->concertPitch() ? concertClef : transposingClef);
      clef->staff()->addClef(clef);

      Segment* segment = clef->segment();
      updateNoteLines(segment, clef->track());
      clef->score()->setLayoutAll(true);

      concertClef     = ocl;
      transposingClef = otc;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void MoveStaff::flip()
      {
      Part* oldPart = staff->part();
      int idx = staff->rstaff();
      oldPart->removeStaff(staff);
      staff->setRstaff(rstaff);
      part->insertStaff(staff);
      part = oldPart;
      rstaff = idx;
      staff->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   ChangeDurationType::flip
//---------------------------------------------------------

void ChangeDurationType::flip()
      {
      TDuration type = cr->durationType();
      cr->setDurationType(t);
      t = type;
      }

//---------------------------------------------------------
//   ChangeSpannerAnchor::flip
//---------------------------------------------------------

void ChangeSpannerAnchor::flip()
      {
      Element* se = spanner->startElement();
      Element* ee = spanner->endElement();

//      qDebug("ChangeSpannerAnchor:flip() spanner(%p--%p) %s  end(%p) -> end(%p)",
//         spanner->score(), spanner, spanner->name(),
//         spanner->endElement(), endElement);

      switch(spanner->anchor()) {
            case Spanner::ANCHOR_CHORD:
                  {
                  Q_ASSERT(spanner->type() == Element::SLUR);
                  Slur* slur = static_cast<Slur*>(spanner);
                  slur->startChord()->removeSlurFor(slur);
                  spanner->setStartElement(startElement);
                  static_cast<ChordRest*>(startElement)->addSlurFor(slur);

                  slur->endChord()->removeSlurBack(slur);
                  spanner->setEndElement(endElement);
                  static_cast<ChordRest*>(endElement)->addSlurBack(slur);
                  }
                  break;

            case Spanner::ANCHOR_MEASURE:
                  Q_ASSERT(spanner->startElement()->type() == Element::MEASURE);
                  static_cast<Measure*>(spanner->startElement())->removeSpannerFor(spanner);
                  spanner->setStartElement(startElement);
                  static_cast<Measure*>(startElement)->addSpannerFor(spanner);

                  Q_ASSERT(spanner->endElement()->type() == Element::MEASURE);
                  static_cast<Measure*>(spanner->endElement())->removeSpannerBack(spanner);
                  spanner->setEndElement(endElement);
                  static_cast<Measure*>(endElement)->addSpannerBack(spanner);
                  break;

            case Spanner::ANCHOR_SEGMENT:
                  Q_ASSERT(spanner->startElement()->type() == Element::SEGMENT);
                  static_cast<Segment*>(spanner->startElement())->removeSpannerFor(spanner);
                  spanner->setStartElement(startElement);
                  static_cast<Segment*>(startElement)->addSpannerFor(spanner);

                  Q_ASSERT(spanner->endElement()->type() == Element::SEGMENT);
                  static_cast<Segment*>(spanner->endElement())->removeSpannerBack(spanner);
                  spanner->setEndElement(endElement);
                  static_cast<Segment*>(endElement)->addSpannerBack(spanner);
                  break;

            default:
                  qDebug("ChangeSpannerAnchor: not implemented for %s", spanner->name());
                  break;
            }
      startElement = se;
      endElement   = ee;
      spanner->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   ChangeStaffUserDist::flip
//---------------------------------------------------------

void ChangeStaffUserDist::flip()
      {
      qreal v = staff->userDist();
      staff->setUserDist(dist);
      dist = v;
      staff->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   ChangePartProperty::flip
//---------------------------------------------------------

void ChangePartProperty::flip()
      {
      QVariant v = part->getProperty(id);
      part->setProperty(id, property);
      property = v;
      }

//---------------------------------------------------------
//   ChangeProperty::flip
//---------------------------------------------------------

void ChangeProperty::flip()
      {
      QVariant v = element->getProperty(id);
      element->setProperty(id, property);
      property = v;
      }

//---------------------------------------------------------
//   ChangeMetaText::flip
//---------------------------------------------------------

void ChangeMetaText::flip()
      {
      QString s = score->metaTag(id);
      score->setMetaTag(id, text);
      text = s;
      }

//---------------------------------------------------------
//   ChangeEventList
//---------------------------------------------------------

ChangeEventList::~ChangeEventList()
      {
      }

//---------------------------------------------------------
//   ChangeEventList::flip
//---------------------------------------------------------

void ChangeEventList::flip()
      {
      int n = chord->notes().size();
      for (int i = 0; i < n; ++i) {
            Note* note = chord->notes()[i];
            note->playEvents().swap(events[i]);
            }
      bool um = chord->userPlayEvents();
      chord->setUserPlayEvents(userModified);
      userModified = um;
      }

