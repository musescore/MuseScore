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
#include "tie.h"
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

namespace Ms {

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
                        for (Note* n : chord->notes())
                              n->updateLine();
                        for (Chord* gc : chord->graceNotes()) {
                              for(Note* gn : gc->notes())
                                    gn->updateLine();
                              }
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
      else
            qDebug("UndoStack:push1(): no active command, UndoStack %p", this);
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
      score->setSelection(undoSelection);
      }

void SaveState::redo()
      {
      undoInputState = score->inputState();
      undoSelection  = score->selection();
      score->setInputState(redoInputState);
      score->setSelection(redoSelection);
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Score::undoChangeProperty(Element* e, P_ID t, const QVariant& st, PropertyStyle ps)
      {
      if (propertyLink(t) && e->links()) {
            foreach(Element* e, *e->links()) {
                  if (e->getProperty(t) != st)
                        undo(new ChangeProperty(e, t, st, ps));
                  }
            }
      else {
            if (e->getProperty(t) != st)
                  undo(new ChangeProperty(e, t, st, ps));
            }
      }

//---------------------------------------------------------
//   undoPropertyChanged
//---------------------------------------------------------

void Score::undoPropertyChanged(Element* e, P_ID t, const QVariant& st)
      {
      if (propertyLink(t) && e->links()) {
            foreach (Element* ee, *e->links()) {
                  if (ee == e) {
                        if (ee->getProperty(t) != st)
                              undo()->push1(new ChangeProperty(ee, t, st));
                        }
                  else {
                        // property in linked element has not changed yet
                        // push() calls redo() to change it
                        if (ee->getProperty(t) != e->getProperty(t))
                              undo()->push(new ChangeProperty(ee, t, e->getProperty(t)));
                        }
                  }
            }
      else {
            if (e->getProperty(t) != st) {
                  undo()->push1(new ChangeProperty(e, t, st));
                  }
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

            int diff = -staff->part()->instr()->transpose().chromatic;
            if (diff != 0 && !score->styleB(ST_concertPitch))
                  st.setAccidentalType(transposeKey(st.accidentalType(), diff));

            nks->changeKeySigEvent(st);
            nks->setParent(s);
            if (links == 0)
                  links = new LinkedElements(score);
            links->append(nks);
            nks->setLinks(links);

            if (ks)
                  undo(new ChangeElement(ks, nks));
            else
                  undo(new AddElement(nks));
            updateNoteLines(s, track);
            //
            // change all following generated keysigs
            //
            for (Measure* m = measure->nextMeasure(); m; m = m->nextMeasure()) {
                  Segment* s = m->undoGetSegment(Segment::SegKeySig, m->tick());
                  if (!s)
                        continue;
                  KeySig* ks = static_cast<KeySig*>(s->element(track));
                  if (!ks)
                        continue;
                  if (ks && !ks->generated())
                        break;
                  if (ks->keySigEvent() != st)
                        undo(new ChangeKeySig(ks, st, ks->showCourtesy(), ks->showNaturals()));
                  }
            }
      }

//---------------------------------------------------------
//   undoChangeClef
//    change clef if seg contains a clef
//    else
//    create a clef before segment seg
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
            if (staff->staffType()->group() != ClefInfo::staffGroup(st))
                  continue;
            int tick = seg->tick();
            Measure* measure = score->tick2measure(tick);
            if (!measure) {
                  qDebug("measure for tick %d not found!", tick);
                  continue;
                  }

            // move clef to last segment of prev measure?
            //    TODO: section break?
            Segment* segment = measure->findSegment(seg->segmentType(), tick);

            if (firstSeg
               && measure->prevMeasure()
               && !(measure->prevMeasure()->repeatFlags() & RepeatEnd)
               ) {
                  measure = measure->prevMeasure();
                  segment = measure->findSegment(seg->segmentType(), tick);
                  if (!segment && (seg->segmentType() != Segment::SegClef))
                        segment = measure->findSegment(Segment::SegClef, tick);
                  }

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
      Staff* ostaff = element->staff();

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
         || (et == Element::SLUR && element->parent() && element->parent()->type() == Element::CHORD)
         || (et == Element::CHORD && static_cast<Chord*>(element)->isGrace())
            ) {
            Element* parent       = element->parent();
            LinkedElements* links = parent->links();
            if (links == 0) {
                  undo(new AddElement(element));
                  if (element->type() == Element::FINGERING)
                        element->score()->layoutFingering(static_cast<Fingering*>(element));
                  else if (element->type() == Element::CHORD) {
                        for (Note* n : static_cast<Chord*>(element)->notes()) {
                              if(n->tpc() == INVALID_TPC)
                                    n->setTpcFromPitch();
                              }
                        element->score()->updateNotes();
                        }
                  return;
                  }
            foreach (Element* e, *links) {
                  Element* ne = (e == parent) ? element : element->linkedClone();
                  ne->setScore(e->score());
                  ne->setSelected(false);
                  ne->setParent(e);
                  undo(new AddElement(ne));
                  if (ne->type() == Element::FINGERING)
                        e->score()->layoutFingering(static_cast<Fingering*>(ne));
                  else if (ne->type() == Element::CHORD) {
                        for (Note* n : static_cast<Chord*>(ne)->notes())
                              n->setTpcFromPitch();
                        ne->score()->updateNotes();
                        }
                  else if (ne->type() == Element::SLUR) {
                        Slur* slur = static_cast<Slur*>(ne);
                        slur->setStartElement(e);
                        slur->setEndElement(e->parent());
                        }
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
         && et != Element::PEDAL
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
            else if (element->type() == Element::SLUR
               || element->type() == Element::HAIRPIN
               || element->type() == Element::OTTAVA
               || element->type() == Element::TRILL
               || element->type() == Element::TEXTLINE
               || element->type() == Element::PEDAL
               || element->type() == Element::VOLTA) {
                  // ne->setParent(0);  ???
                  Spanner* nsp = static_cast<Spanner*>(ne);
                  Spanner* sp = static_cast<Spanner*>(element);
                  nsp->setTrack2(staffIdx * VOICES + (sp->track2() % VOICES));
                  undo(new AddElement(nsp));
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
                  Segment* ns1;
                  Segment* ns2;
                  ns1 = nm1->findSegment(s1->segmentType(), s1->tick());
                  ns2 = nm2 ? nm2->findSegment(s2->segmentType(), s2->tick()) : 0;
                  Chord* c1      = static_cast<Chord*>(ns1->element(staffIdx * VOICES + cr1->voice()));
                  int sm = 0;
                  if (cr1->staffIdx() != cr2->staffIdx())
                        sm = cr1->staffMove() + cr2->staffMove();
                  Chord* c2      = ns2 ? static_cast<Chord*>(ns2->element((staffIdx + sm) * VOICES + cr2->voice())) : 0;
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
      Segment::SegmentType segmentType = Segment::SegChordRest;

      Tuplet* t = cr->tuplet();
      foreach(Staff* staff, staffList) {
            Score* score = staff->score();
            Measure* m   = (score == this) ? measure : score->tick2measure(tick);
            Segment* seg = m->undoGetSegment(segmentType, tick);

            ChordRest* newcr = (staff == ostaff) ? cr : static_cast<ChordRest*>(cr->linkedClone());
            newcr->setScore(score);
            int staffIdx = score->staffIdx(staff);
            int ntrack   = staffIdx * VOICES + cr->voice();
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
            if (t) {
                  Tuplet* nt = 0;
                  if (staff == ostaff)
                        nt = t;
                  else {
                        if (t->elements().empty() || t->elements().front() == cr) {
                              nt = static_cast<Tuplet*>(t->linkedClone());
                              nt->setScore(score);
                              }
                        else {
                              LinkedElements* le = t->links();
                              // search the linked tuplet
                              foreach(Element* e, *le) {
                                    if (e->score() == score && e->track() == ntrack) {
                                          nt = static_cast<Tuplet*>(e);
                                          break;
                                          }
                                    }
                              if (nt == 0)
                                    qDebug("linked tuplet not found");
                              }
                        newcr->setTuplet(nt);
                        }
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
            //if (e->type() == Element::KEYSIG)                  // TODO: should be done in undo()/redo()
            //      e->score()->cmdUpdateNotes();
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
//      qDebug("AddElement::undo: %s %p parent %s %p", element->name(), element,
//         element->parent() ? element->parent()->name() : "nil", element->parent());

      element->score()->removeElement(element);
      if (element->type() == Element::TIE) {
            Tie* tie = static_cast<Tie*>(element);
            Measure* m1 = tie->startNote()->chord()->measure();
            Measure* m2 = 0;
            if(tie->endNote())
                  m2 = tie->endNote()->chord()->measure();

            if (m1 != m2) {
                  tie->score()->updateAccidentals(m1, tie->staffIdx());
                  if (m2)
                        tie->score()->updateAccidentals(m2, tie->staffIdx());
                  // tie->score()->cmdUpdateNotes();
                  }
            else
                  tie->score()->updateAccidentals(m1, tie->staffIdx());
            }
      else if (element->isChordRest()) {
            undoRemoveTuplet(static_cast<ChordRest*>(element));
            }
      else if (element->type() == Element::KEYSIG) {
            KeySig* ks = static_cast<KeySig*>(element);
            if (!ks->generated())
                  ks->score()->cmdUpdateAccidentals(ks->measure(), ks->staffIdx());
            }
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void AddElement::redo()
      {
//      qDebug("AddElement::redo: %s %p parent %s %p", element->name(), element,
//         element->parent() ? element->parent()->name() : "nil", element->parent());

      element->score()->addElement(element);
      if (element->type() == Element::TIE) {
            Tie* tie = static_cast<Tie*>(element);
            Measure* m1 = tie->startNote()->chord()->measure();
            Measure* m2 = tie->endNote() ? tie->endNote()->chord()->measure() : 0;

            if (m2 && (m1 != m2)) {
                  tie->score()->updateAccidentals(m1, tie->staffIdx());
                  if (m2)
                        tie->score()->updateAccidentals(m2, tie->staffIdx());
                  // tie->score()->cmdUpdateNotes();
                  }
            else
                  tie->score()->updateAccidentals(m1, tie->staffIdx());
            }
      else if (element->isChordRest()) {
            undoAddTuplet(static_cast<ChordRest*>(element));
            }
      else if (element->type() == Element::KEYSIG) {
            KeySig* ks = static_cast<KeySig*>(element);
            if (!ks->generated())
                  ks->score()->cmdUpdateAccidentals(ks->measure(), ks->staffIdx());
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
            if (cr->tuplet() && cr->tuplet()->elements().empty())
                  score->undoRemoveElement(cr->tuplet());
            if (e->type() == Element::CHORD) {
                  Chord* chord = static_cast<Chord*>(e);
                  // remove tremolo between 2 notes
                  if (chord->tremolo()) {
                        Tremolo* tremolo = chord->tremolo();
                        if (tremolo->twoNotes())
                              score->undoRemoveElement(tremolo);
                        }
                  foreach(Note* note, chord->notes()) {
                        if (note->tieFor() && note->tieFor()->endNote())
                              score->undoRemoveElement(note->tieFor());
                        if (note->tieBack())
                              score->undoRemoveElement(note->tieBack());
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
      if (element->type() == Element::KEYSIG) {
            KeySig* ks = static_cast<KeySig*>(element);
            if (!ks->generated())
                  ks->score()->cmdUpdateAccidentals(ks->measure(), ks->staffIdx());
            }
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void RemoveElement::redo()
      {
      element->score()->removeElement(element);
      if (element->isChordRest())
            undoRemoveTuplet(static_cast<ChordRest*>(element));
      else if (element->type() == Element::KEYSIG) {
            KeySig* ks = static_cast<KeySig*>(element);
            if (!ks->generated())
                  ks->score()->cmdUpdateAccidentals(ks->measure(), ks->staffIdx());
            }
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
      if (updateAccid) {
            Chord* chord = note->chord();
            Measure* measure = chord->segment()->measure();
            score->updateAccidentals(measure, chord->staffIdx());
            }
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

      if (newElement->type() == Element::KEYSIG) {
            KeySig* ks = static_cast<KeySig*>(newElement);
            if (!ks->generated()) {
                  ks->staff()->setKey(ks->tick(),ks->keySigEvent());
                  ks->insertIntoKeySigChain();
                  ks->score()->cmdUpdateAccidentals(ks->measure(), ks->staffIdx());
                  newElement->staff()->setUpdateKeymap(true);
                  }
            }
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
      qSwap(oldElement, newElement);
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
      //keysig->score()->cmdUpdateNotes();
      if (!keysig->generated())
            keysig->score()->cmdUpdateAccidentals(keysig->measure(), keysig->staffIdx());
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
      cr->score()->setLayoutAll(true);
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
      staff->score()->setLayoutAll(true);
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

      if (MScore::seq == 0) {
            qDebug("ChangePatch: no seq");
            return;
            }

      NPlayEvent event;
      event.setType(ME_CONTROLLER);
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

ChangeStaff::ChangeStaff(Staff* _staff, bool _small, bool _invisible, qreal _userDist, StaffType* st)
      {
      staff     = _staff;
      small     = _small;
      invisible = _invisible;
      userDist  = _userDist;
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
      qreal oldUserDist = staff->userDist();
      StaffType* st     = staff->staffType();

      staff->setSmall(small);
      staff->setInvisible(invisible);
      staff->setUserDist(userDist);
      staff->setStaffType(staffType);

      small     = oldSmall;
      invisible = oldInvisible;
      userDist  = oldUserDist;
      staffType = st;

      Score* score = staff->score();
      if (invisibleChanged || typeChanged) {
            int staffIdx = score->staffIdx(staff);
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  MStaff* mstaff = m->mstaff(staffIdx);
                  mstaff->lines->setVisible(!staff->invisible());
                  }
            }
      staff->score()->setLayoutAll(true);
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
                  text->textStyleChanged();
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

static void updateTimeSigs(void*, Element* e)
      {
      if (e->type() == Element::TIMESIG) {
            TimeSig* ts = static_cast<TimeSig*>(e);
            ts->setNeedLayout(true);
            }
      }

static void updateTextStyle2(void*, Element* e)
      {
      if (!e->isText()) {
            e->styleChanged();
            return;
            }

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

      if (score->style(ST_concertPitch) != style.value(ST_concertPitch))
            score->cmdConcertPitchChanged(style.value(ST_concertPitch).toBool(), true);
      if (score->style(ST_MusicalSymbolFont) != style.value(ST_MusicalSymbolFont))
            score->scanElements(0, updateTimeSigs);

      score->setStyle(style);
      score->scanElements(0, updateTextStyle2);
      score->setLayoutAll(true);

      style = tmp;
      }

//---------------------------------------------------------
//   ChangeStyleVal::flip
//---------------------------------------------------------

void ChangeStyleVal::flip()
      {
      QVariant v = score->style(idx);
      score->style()->set(idx, value);
      score->setLayoutAll(true);
      value = v;
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
//   undoRemoveMeasures
//---------------------------------------------------------

void Score::undoRemoveMeasures(Measure* m1, Measure* m2)
      {
      int tick1 = m1->tick();
      int tick2 = m2->endTick();

      for (auto i : _spanner.findContained(tick1, tick2))
            undoRemoveElement(i.value);

      //
      //  handle ties which start before m1 and end in (m1-m2)
      //
      for (Segment* s = m1->first(); s != m2->last(); s = s->next1()) {
            if (s->segmentType() != Segment::SegChordRest)
                  continue;
            for (int track = 0; track < ntracks(); ++track) {
                  Chord* c = static_cast<Chord*>(s->element(track));
                  if (c == 0 || c->type() != Element::CHORD)
                        continue;
                  for (Note* n : c->notes()) {
                        Tie* t = n->tieBack();
                        if (t) {
                              if (t->startNote()->chord()->tick() < m1->tick()) {
                                    t->setEndNote(0);
                                    n->setTieBack(0);
                                    }
                              }
                        }
                  }
            }
      undo(new RemoveMeasures(m1, m2));
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
      int ticks = 0;
      for (Measure* m = fm; m; m = m->nextMeasure()) {
            ticks += m->ticks();
            if (m == lm)
                  break;
            }
      fm->score()->insertTime(fm->tick(), ticks);
      fm->score()->fixTicks();
      fm->score()->connectTies();
      fm->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   redo
//    remove measures
//---------------------------------------------------------

void RemoveMeasures::redo()
      {
      fm->score()->measures()->remove(fm, lm);
      int ticks = 0;
      for (Measure* m = fm; m; m = m->nextMeasure()) {
            ticks += m->ticks();
            if (m == lm)
                  break;
            }
      fm->score()->insertTime(fm->tick(), -ticks);
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
      fm->score()->connectTies();
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
#ifdef DEBUG_UNDO
      qDebug()
            << "ChangeProperty::flip(): "
            << propertyName(id)
            << " "
            << element->getProperty(id)
            << " -> "
            << property
            ;
#endif
      QVariant v       = element->getProperty(id);
      PropertyStyle ps = element->propertyStyle(id);
      if (propertyStyle == PropertyStyle::STYLED)
            element->resetProperty(id);
      else
            element->setProperty(id, property);
      property = v;
      propertyStyle = ps;
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

//---------------------------------------------------------
//   ChangeSynthesizerState::flip
//---------------------------------------------------------

void ChangeSynthesizerState::flip()
      {
      std::swap(state, score->_synthesizerState);
      }

//---------------------------------------------------------
//   undoAddBracket
//---------------------------------------------------------

void Score::undoAddBracket(Staff* staff, int level, BracketType type, int span)
      {
      undo(new AddBracket(staff, level, type, span));
      }

//---------------------------------------------------------
//   undoRemoveBracket
//---------------------------------------------------------

void Score::undoRemoveBracket(Bracket* b)
      {
      undo(new RemoveBracket(b->staff(), b->level(), b->bracketType(), b->span()));
      }

void AddBracket::redo()
      {
      staff->setBracket(level, type);
      staff->setBracketSpan(level, span);
      staff->score()->setLayoutAll(true);
      }

void AddBracket::undo()
      {
      staff->setBracket(level, NO_BRACKET);
      staff->score()->setLayoutAll(true);
      }


void RemoveBracket::redo()
      {
      staff->setBracket(level, NO_BRACKET);
      staff->score()->setLayoutAll(true);
      }

void RemoveBracket::undo()
      {
      staff->setBracket(level, type);
      staff->setBracketSpan(level, span);
      staff->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   ChangeSpannerElements
//---------------------------------------------------------

void ChangeSpannerElements::flip()
      {
      Element* se = spanner->startElement();
      Element* ee = spanner->endElement();
      spanner->setStartElement(startElement);
      spanner->setEndElement(endElement);
      startElement = se;
      endElement   = ee;
      if (spanner->type() == Element::TIE) {
            Tie* tie = static_cast<Tie*>(spanner);
            static_cast<Note*>(endElement)->setTieBack(0);
            tie->endNote()->setTieBack(tie);
            static_cast<Note*>(startElement)->setTieFor(0);
            tie->startNote()->setTieFor(tie);
            }
      spanner->score()->setLayoutAll(true);
      }


}

