//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
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
#include "sym.h"
#include "utils.h"
#include "glissando.h"
#include "stafflines.h"
#include "bracket.h"

namespace Ms {

extern Measure* tick2measure(int tick);

//---------------------------------------------------------
//   updateNoteLines
//    compute line position of noteheads after
//    clef change
//---------------------------------------------------------

void updateNoteLines(Segment* segment, int track)
      {
      Staff* staff = segment->score()->staff(track / VOICES);
      if (staff->isDrumStaff(segment->tick()) || staff->isTabStaff(segment->tick()))
            return;
      for (Segment* s = segment->next1(); s; s = s->next1()) {
            if ((s->segmentType() & (SegmentType::Clef | SegmentType::HeaderClef)) && s->element(track) && !s->element(track)->generated())
                  break;
            if (!s->isChordRestType())
                  continue;
            for (int t = track; t < track + VOICES; ++t) {
                  Element* e = s->element(t);
                  if (e && e->isChord()) {
                        Chord* chord = toChord(e);
                        for (Note* n : chord->notes())
                              n->updateLine();
                        chord->sortNotes();
                        for (Chord* gc : chord->graceNotes()) {
                              for (Note* gn : gc->notes())
                                    gn->updateLine();
                              gc->sortNotes();
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
      for (auto c : childList)
            delete c;
      }

//---------------------------------------------------------
//   UndoCommand::cleanup
//---------------------------------------------------------

void UndoCommand::cleanup(bool undo)
      {
      for (auto c : childList)
            c->cleanup(undo);
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void UndoCommand::undo(EditData* ed)
      {
      int n = childList.size();
      for (int i = n-1; i >= 0; --i) {
            qCDebug(undoRedo) << "<" << childList[i]->name() << ">";
            childList[i]->undo(ed);
            }
      flip(ed);
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void UndoCommand::redo(EditData* ed)
      {
      int n = childList.size();
      for (int i = 0; i < n; ++i) {
            qCDebug(undoRedo) << "<" << childList[i]->name() << ">";
            childList[i]->redo(ed);
            }
      flip(ed);
      }

//---------------------------------------------------------
//   unwind
//---------------------------------------------------------

void UndoCommand::unwind()
      {
      while (!childList.empty()) {
            UndoCommand* c = childList.takeLast();
            c->undo(0);
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
      int idx = 0;
      for (auto c : list)
            c->cleanup(idx++ < curIdx);
      qDeleteAll(list);
      }

//---------------------------------------------------------
//   beginMacro
//---------------------------------------------------------

void UndoStack::beginMacro()
      {
      if (curCmd) {
            qWarning("UndoStack:beginMacro(): already active");
            return;
            }
      curCmd = new UndoCommand();
      }

//---------------------------------------------------------
//   endMacro
//---------------------------------------------------------

void UndoStack::endMacro(bool rollback)
      {
      if (curCmd == 0) {
            qWarning("UndoStack:endMacro(): not active");
            return;
            }
      if (rollback)
            delete curCmd;
      else {
            // remove redo stack
            while (list.size() > curIdx) {
                  UndoCommand* cmd = list.takeLast();
                  cmd->cleanup(false);  // delete elements for which UndoCommand() holds ownership
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

void UndoStack::push(UndoCommand* cmd, EditData* ed)
      {
      if (!curCmd) {
            // this can happen for layout() outside of a command (load)
            // qWarning("UndoStack:push(): no active command, UndoStack %p", this);

            cmd->redo(ed);
            delete cmd;
            return;
            }
#ifndef QT_NO_DEBUG
      if (!strcmp(cmd->name(), "ChangeProperty")) {
            ChangeProperty* cp = static_cast<ChangeProperty*>(cmd);
            qCDebug(undoRedo, "<%s> id %d %s", cmd->name(), int(cp->getId()), propertyName(cp->getId()));
            }
      else {
            qCDebug(undoRedo, "<%s>", cmd->name());
            }
#endif
      curCmd->appendChild(cmd);
      cmd->redo(ed);
      }

//---------------------------------------------------------
//   push1
//---------------------------------------------------------

void UndoStack::push1(UndoCommand* cmd)
      {
      if (!curCmd) {
            qWarning("UndoStack:push1(): no active command, UndoStack %p", this);
            return;
            }
      curCmd->appendChild(cmd);
      }

//---------------------------------------------------------
//   pop
//---------------------------------------------------------

void UndoStack::pop()
      {
      if (!curCmd) {
            qWarning("UndoStack:pop(): no active command");
            return;
            }
      UndoCommand* cmd = curCmd->removeChild();
      cmd->undo(0);
      }

//---------------------------------------------------------
//   setClean
//---------------------------------------------------------

void UndoStack::setClean()
      {
      cleanIdx = curIdx;
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void UndoStack::undo(EditData* ed)
      {
      qCDebug(undoRedo) << "===";
      if (curIdx) {
            --curIdx;
            Q_ASSERT(curIdx >= 0);
            list[curIdx]->undo(ed);
            }
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void UndoStack::redo(EditData* ed)
      {
      qCDebug(undoRedo) << "===";
      if (canRedo())
            list[curIdx++]->redo(ed);
      }

//---------------------------------------------------------
//   SaveState
//---------------------------------------------------------

SaveState::SaveState(Score* s)
   : undoInputState(s), redoInputState(s->inputState())
      {
      score = s;
//      redoSelection  = score->selection();
      }

void SaveState::undo(EditData*)
      {
      redoInputState = score->inputState();
//      redoSelection  = score->selection();
      score->setInputState(undoInputState);
//      score->setSelection(undoSelection);
      }

void SaveState::redo(EditData*)
      {
      undoInputState = score->inputState();
//      undoSelection  = score->selection();
      score->setInputState(redoInputState);
//      score->setSelection(redoSelection);
      }

//---------------------------------------------------------
//   CloneVoice
//---------------------------------------------------------

CloneVoice::CloneVoice(Segment* _sf, int _lTick, Segment* _d, int _strack, int _dtrack, int _otrack, bool _linked)
      {
      sf      = _sf;          // first source segment
      lTick   = _lTick;       // last tick to clone
      d       = _d;           // first destination segment
      strack  = _strack;
      dtrack  = _dtrack;
      otrack  = _otrack;      // old source track if -1 delete voice in strack after copy
      linked  = _linked;      // if true  add elements in destination segment only
                              // if false add elements in every linked staff
      }

void CloneVoice::undo(EditData*)
      {
      Score* s = d->score();
      int ticks = d->tick() + lTick - sf->tick();
      int sTrack = otrack == -1 ? dtrack : otrack; // use the correct source / destination if deleting the source
      int dTrack = otrack == -1 ? strack : dtrack;

      // Clear destination voice (in case of not linked and otrack = -1 we would delete our source
      if (otrack != -1 && linked)
            for (Segment* seg = d; seg && seg->tick() < ticks; seg = seg->next1()) {
                  Element* el = seg->element(dTrack);
                  if (el && el->isChordRest()) {
                        el->unlink();
                        seg->setElement(dTrack, 0);
                        }
                  }

      if (otrack == -1 && !linked) {
            // On the first run get going the undo redo action for adding/deleting elements and slurs
            if (first) {
                  s->cloneVoice(sTrack, dTrack, sf, ticks, linked);
                  auto spanners = s->spannerMap().findOverlapping(sf->tick(), lTick);
                  for (auto i = spanners.begin(); i < spanners.end(); i++) {
                        Spanner* sp = i->value;
                        if (sp->isSlur() && (sp->track() == sTrack || sp->track2() == sTrack))
                              s->undoRemoveElement(sp);
                        }
                  for (Segment* seg = d; seg && seg->tick() < ticks; seg = seg->next1()) {
                        Element* el = seg->element(sTrack);
                        if (el && el->isChordRest()) {
                              s->undoRemoveElement(el);
                              }
                        }
                  }
            // Set rests if first voice in a staff
            if (!(sTrack % VOICES))
                  s->setRest(d->tick(), sTrack, Fraction::fromTicks(ticks), false, 0);
            }
      else {
            s->cloneVoice(sTrack, dTrack, sf, ticks, linked);
            if (!linked && !(dTrack % VOICES))
                  s->setRest(d->tick(), dTrack, Fraction::fromTicks(ticks), false, 0);
            }

      first = false;
      }

void CloneVoice::redo(EditData*)
      {
      Score* s = d->score();
      int ticks = d->tick() + lTick - sf->tick();

      // Clear destination voice (in case of not linked and otrack = -1 we would delete our source
      if (otrack != -1 && linked)
            for (Segment* seg = d; seg && seg->tick() < ticks; seg = seg->next1()) {
                  Element* el = seg->element(dtrack);
                  if (el && el->isChordRest()) {
                        el->unlink();
                        seg->setElement(dtrack, 0);
                        }
                  }

      if (otrack == -1 && !linked) {
            // On the first run get going the undo redo action for adding/deleting elements and slurs
            if (first) {
                  s->cloneVoice(strack, dtrack, sf, ticks, linked);
                  auto spanners = s->spannerMap().findOverlapping(sf->tick(), lTick);
                  for (auto i = spanners.begin(); i < spanners.end(); i++) {
                        Spanner* sp = i->value;
                        if (sp->isSlur() && (sp->track() == strack || sp->track2() == strack))
                              s->undoRemoveElement(sp);
                        }
                  for (Segment* seg = d; seg && seg->tick() < ticks; seg = seg->next1()) {
                        Element* el = seg->element(strack);
                        if (el && el->isChordRest()) {
                              s->undoRemoveElement(el);
                              }
                        }
                  }
            // Set rests if first voice in a staff
            if (!(strack % VOICES))
                  s->setRest(d->tick(), strack, Fraction::fromTicks(ticks), false, 0);
            }
      else
            s->cloneVoice(strack, dtrack, sf, ticks, linked, first);
      }

//---------------------------------------------------------
//   AddElement
//---------------------------------------------------------

AddElement::AddElement(Element* e)
      {
      element = e;
      }

//---------------------------------------------------------
//   AddElement::cleanup
//---------------------------------------------------------

void AddElement::cleanup(bool undo)
      {
      if (!undo) {
            delete element;
            element = 0;
            }
      }

//---------------------------------------------------------
//   undoRemoveTuplet
//---------------------------------------------------------

static void undoRemoveTuplet(DurationElement* cr)
      {
      if (cr->tuplet()) {
            cr->tuplet()->remove(cr);
            if (cr->tuplet()->elements().empty())
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
//   endUndoRedo
//---------------------------------------------------------

void AddElement::endUndoRedo(bool isUndo) const
      {
      if (element->isChordRest()) {
            if (isUndo)
                  undoRemoveTuplet(toChordRest(element));
            else
                  undoAddTuplet(toChordRest(element));
            }
      else if (element->isClef()) {
            element->score()->setLayout(element->tick());
            element->score()->setLayout(element->staff()->nextClefTick(element->tick()));
            }
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void AddElement::undo(EditData*)
      {
      if (!element->isTuplet())
            element->score()->removeElement(element);
      endUndoRedo(true);
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void AddElement::redo(EditData*)
      {
      if (!element->isTuplet())
            element->score()->addElement(element);
      endUndoRedo(false);
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* AddElement::name() const
      {
      static char buffer[64];
      if (element->isTextBase())
            snprintf(buffer, 64, "Add:    %s <%s> %p", element->name(),
               qPrintable(toTextBase(element)->plainText()), element);
      else if (element->isSegment())
            snprintf(buffer, 64, "Add:    <%s-%s> %p", element->name(), toSegment(element)->subTypeName(), element);
      else
            snprintf(buffer, 64, "Add:    <%s> %p", element->name(), element);
      return buffer;
      }

//---------------------------------------------------------
//   RemoveElement
//---------------------------------------------------------

RemoveElement::RemoveElement(Element* e)
      {
      element = e;

      Score* score = element->score();
      if (element->isChordRest()) {
            ChordRest* cr = toChordRest(element);
            if (cr->tuplet() && cr->tuplet()->elements().size() <= 1)
                  score->undo(new RemoveElement(cr->tuplet()));
            if (e->isChord()) {
                  Chord* chord = toChord(e);
                  // remove tremolo between 2 notes
                  if (chord->tremolo()) {
                        Tremolo* tremolo = chord->tremolo();
                        if (tremolo->twoNotes())
                              score->undo(new RemoveElement(tremolo));
                        }
                  for (const Note* note : chord->notes()) {
                        if (note->tieFor() && note->tieFor()->endNote())
                              score->undo(new RemoveElement(note->tieFor()));
                        if (note->tieBack())
                              score->undo(new RemoveElement(note->tieBack()));
                        for (Spanner* s : note->spannerBack()) {
                              score->undo(new RemoveElement(s));
                              }
                        for (Spanner* s : note->spannerFor()) {
                              score->undo(new RemoveElement(s));
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   RemoveElement::cleanup
//---------------------------------------------------------

void RemoveElement::cleanup(bool undo)
      {
      if (undo) {
            delete element;
            element = 0;
            }
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void RemoveElement::undo(EditData*)
      {
      if (!element->isTuplet())
            element->score()->addElement(element);
      if (element->isChordRest()) {
            if (element->isChord()) {
                  Chord* chord = toChord(element);
                  for (Note* note : chord->notes()) {
                        if (note->tieBack())
                              note->tieBack()->setEndNote(note);
                        if (note->tieFor() && note->tieFor()->endNote())
                              note->tieFor()->endNote()->setTieBack(note->tieFor());
                        }
                  }
            undoAddTuplet(toChordRest(element));
            }
      else if (element->isClef())
            element->score()->setLayout(element->staff()->nextClefTick(element->tick()));
      else if (element->isKeySig())
            element->score()->setLayout(element->staff()->nextKeyTick(element->tick()));
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void RemoveElement::redo(EditData*)
      {
      if (!element->isTuplet())
            element->score()->removeElement(element);
      if (element->isChordRest()) {
            undoRemoveTuplet(toChordRest(element));
            if (element->isChord()) {
                  Chord* chord = toChord(element);
                  for (Note* note : chord->notes()) {
                        if (note->tieFor() && note->tieFor()->endNote()) {
                              note->tieFor()->endNote()->setTieBack(0);
                              }
                        }
                  }
            }
      else if (element->isClef())
            element->score()->setLayout(element->staff()->nextClefTick(element->tick()));
      else if (element->isKeySig())
            element->score()->setLayout(element->staff()->nextKeyTick(element->tick()));
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* RemoveElement::name() const
      {
      static char buffer[64];
      if (element->isTextBase())
            snprintf(buffer, 64, "Remove: %s <%s> %p", element->name(),
               qPrintable(toTextBase(element)->plainText()), element);
      else if (element->isSegment())
            snprintf(buffer, 64, "Remove: <%s-%s> %p", element->name(), toSegment(element)->subTypeName(), element);
      else
            snprintf(buffer, 64, "Remove: %s %p", element->name(), element);
      return buffer;
      }

//---------------------------------------------------------
//   InsertPart
//---------------------------------------------------------

InsertPart::InsertPart(Part* p, int i)
      {
      part = p;
      idx  = i;
      }

void InsertPart::undo(EditData*)
      {
      part->score()->removePart(part);
      }

void InsertPart::redo(EditData*)
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

void RemovePart::undo(EditData*)
      {
      part->score()->insertPart(part, idx);
      }

void RemovePart::redo(EditData*)
      {
      part->score()->removePart(part);
      }

//---------------------------------------------------------
//   InsertStaff
//---------------------------------------------------------

InsertStaff::InsertStaff(Staff* p, int _ridx)
      {
      staff = p;
      ridx  = _ridx;
      }

void InsertStaff::undo(EditData*)
      {
      staff->score()->removeStaff(staff);
      }

void InsertStaff::redo(EditData*)
      {
      staff->score()->insertStaff(staff, ridx);
      }

//---------------------------------------------------------
//   RemoveStaff
//---------------------------------------------------------

RemoveStaff::RemoveStaff(Staff* p)
      {
      staff = p;
      ridx  = staff->rstaff();
      }

void RemoveStaff::undo(EditData*)
      {
      staff->score()->insertStaff(staff, ridx);
      }

void RemoveStaff::redo(EditData*)
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

void InsertMStaff::undo(EditData*)
      {
      measure->removeMStaff(mstaff, idx);
      }

void InsertMStaff::redo(EditData*)
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

void RemoveMStaff::undo(EditData*)
      {
      measure->insertMStaff(mstaff, idx);
      }

void RemoveMStaff::redo(EditData*)
      {
      measure->removeMStaff(mstaff, idx);
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

void SortStaves::redo(EditData*)
      {
      score->sortStaves(list);
      }

void SortStaves::undo(EditData*)
      {
      score->sortStaves(rlist);
      }

//---------------------------------------------------------
//   ChangePitch
//---------------------------------------------------------

ChangePitch::ChangePitch(Note* _note, int _pitch, int _tpc1, int _tpc2)
      {
      note  = _note;
      pitch = _pitch;
      tpc1  = _tpc1;
      tpc2  = _tpc2;
      }

void ChangePitch::flip(EditData*)
      {
      int f_pitch = note->pitch();
      int f_tpc1  = note->tpc1();
      int f_tpc2  = note->tpc2();
      // do not change unless necessary
      if (f_pitch == pitch && f_tpc1 == tpc1 && f_tpc2 == tpc2)
            return;

      note->setPitch(pitch, tpc1, tpc2);
      pitch = f_pitch;
      tpc1  = f_tpc1;
      tpc2  = f_tpc2;

      note->score()->setLayout(note->tick());
      }

//---------------------------------------------------------
//   ChangeFretting
//
//    To use with tablatures to force a specific note fretting;
//    Pitch, string and fret must be changed all together; otherwise,
//    if they are not consistent among themselves, the refretting algorithm may re-assign
//    fret and string numbers for (potentially) all the notes of all the chords of a segment.
//---------------------------------------------------------

ChangeFretting::ChangeFretting(Note* _note, int _pitch, int _string, int _fret, int _tpc1, int _tpc2)
      {
      note  = _note;
      pitch = _pitch;
      string= _string;
      fret  = _fret;
      tpc1  = _tpc1;
      tpc2  = _tpc2;
      }

void ChangeFretting::flip(EditData*)
      {
      int f_pitch = note->pitch();
      int f_string= note->string();
      int f_fret  = note->fret();
      int f_tpc1  = note->tpc1();
      int f_tpc2  = note->tpc2();
      // do not change unless necessary
      if (f_pitch == pitch && f_string == string && f_fret == fret && f_tpc1 == tpc1 && f_tpc2 == tpc2) {
            return;
            }

      note->setPitch(pitch, tpc1, tpc2);
      note->setString(string);
      note->setFret(fret);
      pitch = f_pitch;
      string= f_string;
      fret  = f_fret;
      tpc1  = f_tpc1;
      tpc2  = f_tpc2;
      note->score()->setLayout(note->tick());
      }

//---------------------------------------------------------
//   ChangeElement
//---------------------------------------------------------

ChangeElement::ChangeElement(Element* oe, Element* ne)
      {
      oldElement = oe;
      newElement = ne;
      }

void ChangeElement::flip(EditData*)
      {
      const LinkedElements* links = oldElement->links();
      if (links) {
            newElement->linkTo(oldElement);
            oldElement->unlink();
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

      if (newElement->isKeySig()) {
            KeySig* ks = toKeySig(newElement);
            if (!ks->generated())
                  ks->staff()->setKey(ks->tick(), ks->keySigEvent());
            }
      else if (newElement->isDynamic())
            newElement->score()->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
      else if (newElement->isTempoText()) {
            TempoText* t = toTempoText(oldElement);
            score->setTempo(t->segment(), t->tempo());
            }
//      if (newElement->isSegmentFlag()) {
      if (newElement->isSpannerSegment()) {
            SpannerSegment* os = toSpannerSegment(oldElement);
            SpannerSegment* ns = toSpannerSegment(newElement);
            if (os->system())
                  os->system()->remove(os);
            if (ns->system())
                  ns->system()->add(ns);
            }
      qSwap(oldElement, newElement);
      oldElement->triggerLayout();
      newElement->triggerLayout();
      // score->setLayoutAll();
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

void InsertStaves::undo(EditData*)
      {
      measure->removeStaves(a, b);
      }

void InsertStaves::redo(EditData*)
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

void RemoveStaves::undo(EditData*)
      {
      measure->insertStaves(a, b);
      }

void RemoveStaves::redo(EditData*)
      {
      measure->removeStaves(a, b);
      }

//---------------------------------------------------------
//   ChangeKeySig::flip
//---------------------------------------------------------

void ChangeKeySig::flip(EditData*)
      {
      KeySigEvent oe = keysig->keySigEvent();
      bool sc        = keysig->showCourtesy();

      keysig->setKeySigEvent(ks);
      keysig->setShowCourtesy(showCourtesy);

      int tick = keysig->segment()->tick();

      // update keys if keysig was not generated
      if (!keysig->generated())
            keysig->staff()->setKey(tick, ks);

      showCourtesy = sc;
      ks           = oe;
      keysig->score()->setLayoutAll();  //TODO: reduce update to range covered by keysig
      }

//---------------------------------------------------------
//   ChangeMeasureLen
//---------------------------------------------------------

ChangeMeasureLen::ChangeMeasureLen(Measure* m, Fraction l)
      {
      measure     = m;
      len         = l;
      }

void ChangeMeasureLen::flip(EditData*)
      {
      Fraction oLen = measure->len();

      //
      // move EndBarLine and TimeSigAnnounce
      // to end of measure:
      //

      Segment* s = measure->first();
      std::list<Segment*> sl;
      for (; s;) {
            Segment* ns = s->next();
            if (!s->isEndBarLineType() && !s->isTimeSigAnnounceType()) {
                  s = ns;
                  continue;
                  }
            s->setRtick(len.ticks());
            sl.push_back(s);
            measure->remove(s);
            s = ns;
            }
      measure->setLen(len);
      measure->score()->fixTicks();
      len = oLen;
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

void TransposeHarmony::flip(EditData*)
      {
      int baseTpc1 = harmony->baseTpc();
      int rootTpc1 = harmony->rootTpc();
      harmony->setBaseTpc(baseTpc);
      harmony->setRootTpc(rootTpc);
      harmony->setXmlText(harmony->harmonyName());
      rootTpc = rootTpc1;
      baseTpc = baseTpc1;
      }

//---------------------------------------------------------
//   ExchangeVoice
//---------------------------------------------------------

ExchangeVoice::ExchangeVoice(Measure* m, int _val1, int _val2, int _staff)
      {
      measure = m;
      val1    = _val1;
      val2    = _val2;
      staff   = _staff;
      }

void ExchangeVoice::undo(EditData*)
      {
      measure->exchangeVoice(val2, val1, staff);
      measure->checkMultiVoices(staff);
      }

void ExchangeVoice::redo(EditData*)
      {
      measure->exchangeVoice(val1, val2, staff);
      }

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

ChangeInstrumentShort::ChangeInstrumentShort(int _tick, Part* p, QList<StaffName> t)
      {
      tick = _tick;
      part = p;
      text = t;
      }

void ChangeInstrumentShort::flip(EditData*)
      {
      QList<StaffName> s = part->shortNames(tick);
      part->setShortNames(text, tick);
      text = s;
      part->score()->setLayoutAll();
      }

//---------------------------------------------------------
//   ChangeInstrumentLong
//---------------------------------------------------------

ChangeInstrumentLong::ChangeInstrumentLong(int _tick, Part* p, QList<StaffName> t)
      {
      tick = _tick;
      part = p;
      text = t;
      }

void ChangeInstrumentLong::flip(EditData*)
      {
      QList<StaffName> s = part->longNames(tick);
      part->setLongNames(text, tick);
      text = s;
      part->score()->setLayoutAll();
      }

//---------------------------------------------------------
//   EditText::undo
//---------------------------------------------------------

void EditText::undo(EditData*)
      {
/*      if (!text->styled()) {
            for (int i = 0; i < undoLevel; ++i)
                  text->undo();
            }
      */
      undoRedo();
      }

//---------------------------------------------------------
//   EditText::redo
//---------------------------------------------------------

void EditText::redo(EditData*)
      {
/*
      if (!text->styled()) {
            for (int i = 0; i < undoLevel; ++i)
                  text->redo();
            }
      */
      undoRedo();
      }

//---------------------------------------------------------
//   EditText::undoRedo
//---------------------------------------------------------

void EditText::undoRedo()
      {
      QString s = text->xmlText();
      text->setXmlText(oldText);
      oldText = s;
      text->triggerLayout();
      }

//---------------------------------------------------------
//   ChangePatch
//---------------------------------------------------------

void ChangePatch::flip(EditData*)
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
            qWarning("ChangePatch: no seq");
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

      score->setInstrumentsChanged(true);

      MScore::seq->sendEvent(event);
      channel->updateInitList();
      }

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

ChangeStaff::ChangeStaff(Staff* _staff,  bool _invisible, ClefTypeList _clefType,
   qreal _userDist, Staff::HideMode _hideMode, bool _showIfEmpty, bool _cutaway, bool hide)
      {
      staff       = _staff;
      invisible   = _invisible;
      clefType    = _clefType;
      userDist    = _userDist;
      hideMode    = _hideMode;
      showIfEmpty = _showIfEmpty;
      cutaway     = _cutaway;
      hideSystemBarLine = hide;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStaff::flip(EditData*)
      {
      bool invisibleChanged = staff->invisible() != invisible;
      ClefTypeList oldClefType = staff->defaultClefType();
      bool oldInvisible   = staff->invisible();
      qreal oldUserDist   = staff->userDist();
      Staff::HideMode oldHideMode    = staff->hideWhenEmpty();
      bool oldShowIfEmpty = staff->showIfEmpty();
      bool oldCutaway     = staff->cutaway();
      bool hide           = staff->hideSystemBarLine();

      staff->setInvisible(invisible);
      staff->setDefaultClefType(clefType);
      staff->setUserDist(userDist);
      staff->setHideWhenEmpty(hideMode);
      staff->setShowIfEmpty(showIfEmpty);
      staff->setCutaway(cutaway);
      staff->setHideSystemBarLine(hideSystemBarLine);

      invisible   = oldInvisible;
      clefType    = oldClefType;
      userDist    = oldUserDist;
      hideMode    = oldHideMode;
      showIfEmpty = oldShowIfEmpty;
      cutaway     = oldCutaway;
      hideSystemBarLine = hide;

      Score* score = staff->score();
      if (invisibleChanged) {
            int staffIdx = staff->idx();
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure())
                  m->staffLines(staffIdx)->setVisible(!staff->invisible());
            }
      staff->score()->setLayoutAll();
      staff->masterScore()->rebuildMidiMapping();
      staff->score()->setPlaylistDirty();
      }

//---------------------------------------------------------
//   ChangeStaffType::flip
//---------------------------------------------------------

void ChangeStaffType::flip(EditData*)
      {
      StaffType st = *staff->staffType(0);      // TODO

      staff->setStaffType(0, &staffType);

      staffType = st;

      Score* score = staff->score();
      score->setLayoutAll();
      }

//---------------------------------------------------------
//   ChangePart
//---------------------------------------------------------

ChangePart::ChangePart(Part* _part, Instrument* i, const QString& s)
      {
      instrument = i;
      part       = _part;
      partName   = s;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangePart::flip(EditData*)
      {
      Instrument* oi = part->instrument();
      QString s      = part->partName();
      part->setInstrument(instrument);
      part->setPartName(partName);

      Score* score = part->score();
      score->masterScore()->rebuildMidiMapping();
      score->setInstrumentsChanged(true);
      score->setPlaylistDirty();

      // check if notes need to be updated
      // true if changing into or away from TAB or from one TAB type to another

      score->setLayoutAll();

      partName   = s;
      instrument = oi;
      }

//---------------------------------------------------------
//   ChangeStyle
//---------------------------------------------------------

ChangeStyle::ChangeStyle(Score* s, const MStyle& st)
   : score(s), style(st)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStyle::flip(EditData*)
      {
      MStyle tmp = score->style();

      if (score->styleV(Sid::concertPitch) != style.value(Sid::concertPitch))
            score->cmdConcertPitchChanged(style.value(Sid::concertPitch).toBool(), true);
      if (score->styleV(Sid::MusicalSymbolFont) != style.value(Sid::MusicalSymbolFont)) {
            score->setScoreFont(ScoreFont::fontFactory(style.value(Sid::MusicalSymbolFont).toString()));
            }
      score->setStyle(style);
      score->styleChanged();
      style = tmp;
      }

//---------------------------------------------------------
//   ChangeStyleVal::flip
//---------------------------------------------------------

void ChangeStyleVal::flip(EditData*)
      {
      QVariant v = score->styleV(idx);
      if (v != value) {
            score->style().set(idx, value);
            if (idx == Sid::chordDescriptionFile) {
                  score->style().chordList()->unload();
                  score->style().chordList()->read(value.toString());
                  }
            score->styleChanged();
            }
      value = v;
      }

//---------------------------------------------------------
//   ChangeChordStaffMove
//---------------------------------------------------------

ChangeChordStaffMove::ChangeChordStaffMove(ChordRest* cr, int v)
   : chordRest(cr), staffMove(v)
      {
      }

void ChangeChordStaffMove::flip(EditData*)
      {
      int v = chordRest->staffMove();
      for (ScoreElement* e : chordRest->linkList()) {
            ChordRest* cr = toChordRest(e);
            cr->setStaffMove(staffMove);
            cr->triggerLayout();
            }
      staffMove = v;
      }

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

ChangeVelocity::ChangeVelocity(Note* n, Note::ValueType t, int o)
   : note(n), veloType(t), veloOffset(o)
      {
      }

void ChangeVelocity::flip(EditData*)
      {
      Note::ValueType t = note->veloType();
      int o       = note->veloOffset();
      note->setVeloType(veloType);
      note->setVeloOffset(veloOffset);
      veloType   = t;
      veloOffset = o;
      }

//---------------------------------------------------------
//   ChangeMStaffProperties
//---------------------------------------------------------

ChangeMStaffProperties::ChangeMStaffProperties(Measure* m, int i, bool v, bool s)
   : measure(m), staffIdx(i), visible(v), slashStyle(s)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeMStaffProperties::flip(EditData*)
      {
      bool v = measure->visible(staffIdx);
      bool s = measure->slashStyle(staffIdx);
      measure->setStaffVisible(staffIdx, visible);
      measure->setStaffSlashStyle(staffIdx, slashStyle);
      visible    = v;
      slashStyle = s;
      }

//---------------------------------------------------------
//   insertMeasures
//---------------------------------------------------------

void InsertRemoveMeasures::insertMeasures()
      {
      Score* score = fm->score();
      QList<Clef*> clefs;
      QList<KeySig*> keys;
      Segment* fs = nullptr;
      Segment* ls = nullptr;
      if (fm->isMeasure()) {
            score->setPlaylistDirty();
            fs = toMeasure(fm)->first();
            ls = toMeasure(lm)->last();
            for (Segment* s = fs; s && s != ls; s = s->next1()) {
                  if (!(s->segmentType() & (SegmentType::Clef | SegmentType::KeySig)))
                        continue;
                  for (int track = 0; track < score->ntracks(); track += VOICES) {
                        Element* e = s->element(track);
                        if (!e || e->generated())
                              continue;
                        if (e->isClef())
                              clefs.append(toClef(e));
                        else if (e->isKeySig())
                              keys.append(toKeySig(e));
                        }
                  }
            }
      score->measures()->insert(fm, lm);

      if (fm->isMeasure()) {
            score->fixTicks();
            score->insertTime(fm->tick(), lm->endTick() - fm->tick());

            // move ownership of Instrument back to part
            for (Segment* s = fs; s && s != ls; s = s->next1()) {
                  for (Element* e : s->annotations()) {
                        if (e->isInstrumentChange()) {
                              e->part()->setInstrument(toInstrumentChange(e)->instrument(), s->tick());
                              }
                        }
                  }
            for (Clef* clef : clefs)
                  clef->staff()->setClef(clef);
            for (KeySig* key : keys)
                  key->staff()->setKey(key->segment()->tick(), key->keySigEvent());
            }

      score->setLayoutAll();

      //
      // connect ties
      //

      if (!fm->isMeasure() || !fm->prevMeasure())
            return;
      Measure* m = fm->prevMeasure();
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            for (int track = 0; track < score->ntracks(); ++track) {
                  Element* e = seg->element(track);
                  if (e == 0 || !e->isChord())
                        continue;
                  Chord* chord = toChord(e);
                  foreach (Note* n, chord->notes()) {
                        Tie* tie = n->tieFor();
                        if (!tie)
                              continue;
                        if (!tie->endNote() || tie->endNote()->chord()->segment()->measure() != m) {
                              Note* nn = searchTieNote(n);
                              if (nn) {
                                    tie->setEndNote(nn);
                                    nn->setTieBack(tie);
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   removeMeasures
//---------------------------------------------------------

void InsertRemoveMeasures::removeMeasures()
      {
      Score* score = fm->score();

      int tick1 = fm->tick();
      int tick2 = lm->endTick();

      QList<System*> systemList;
      for (MeasureBase* mb = lm;; mb = mb->prev()) {
            System* system = mb->system();
            if (system) {
                  if (!systemList.contains(system)) {
                        systemList.push_back(system);
                        }
                  auto i = std::find(system->measures().begin(), system->measures().end(), mb);
                  if (i != system->measures().end())
                        system->measures().erase(i);
                  }
            if (mb == fm)
                  break;
            }
      score->measures()->remove(fm, lm);

      //      if (score->firstMeasure())    // any measures left?
      score->fixTicks();
      if (fm->isMeasure()) {
            score->setPlaylistDirty();

            // check if there is a clef at the end of last measure
            // remove clef from staff cleflist

            if (lm->isMeasure()) {
                  Measure* m = toMeasure(lm);
                  Segment* s = m->findSegment(SegmentType::Clef, tick2);
                  if (s) {
                        for (Element* e : s->elist()) {
                              Clef* clef = toClef(e);
                              if (clef)
                                    score->staff(clef->staffIdx())->removeClef(clef);
                              }
                        }
                  }
            if (score->firstMeasure())
                  score->insertTime(tick1, -(tick2 - tick1));
            for (Spanner* sp : score->unmanagedSpanners()) {
                  if ((sp->tick() >= tick1 && sp->tick() < tick2) || (sp->tick2() >= tick1 && sp->tick2() < tick2))
                        sp->removeUnmanaged();
                  }
            score->connectTies(true);   // ??
            }

      // remove empty systems

      for (System* s : systemList) {
            if (s->measures().empty()) {
qDebug("remove system");
                  Page* page = s->page();
                  if (page) {
                        // erase system from page
                        QList<System*>& systemList = page->systems();
                        auto i = std::find(systemList.begin(), systemList.end(), s);
                        if (i != systemList.end())
                              systemList.erase(i);
                        // erase system from score
                        auto k = std::find(score->systems().begin(), score->systems().end(), s);
                        if (k != score->systems().end())
                              score->systems().erase(k);
                        // finally delete system
                        score->deleteLater(s);
                        }
                  }
            }

      score->setLayoutAll();
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeImage::flip(EditData*)
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
//   AddExcerpt::undo
//---------------------------------------------------------

void AddExcerpt::undo(EditData*)
      {
      excerpt->oscore()->removeExcerpt(excerpt);
      }

//---------------------------------------------------------
//   AddExcerpt::redo
//---------------------------------------------------------

void AddExcerpt::redo(EditData*)
      {
      excerpt->oscore()->addExcerpt(excerpt);
      }

//---------------------------------------------------------
//   RemoveExcerpt::undo()
//---------------------------------------------------------

void RemoveExcerpt::undo(EditData*)
      {
      excerpt->oscore()->addExcerpt(excerpt);
      }

//---------------------------------------------------------
//   RemoveExcerpt::redo()
//---------------------------------------------------------

void RemoveExcerpt::redo(EditData*)
      {
      excerpt->oscore()->removeExcerpt(excerpt);
      }

//---------------------------------------------------------
//   SwapExcerpt::flip
//---------------------------------------------------------

void SwapExcerpt::flip(EditData*)
      {
      score->excerpts().swap(pos1, pos2);
      score->setExcerptsChanged(true);
      }

//---------------------------------------------------------
//   ChangeExcerptTitle::flip
//---------------------------------------------------------

void ChangeExcerptTitle::flip(EditData*)
      {
      QString s = title;
      title = excerpt->title();
      excerpt->setTitle(s);
      excerpt->oscore()->setExcerptsChanged(true);
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeBend::flip(EditData*)
      {
      QList<PitchValue> pv = bend->points();
      bend->score()->addRefresh(bend->canvasBoundingRect());
      bend->setPoints(points);
      points = pv;
      bend->layout();
      bend->score()->addRefresh(bend->canvasBoundingRect());
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeTremoloBar::flip(EditData*)
      {
      QList<PitchValue> pv = bend->points();
      bend->setPoints(points);
      points = pv;
      }

//---------------------------------------------------------
//   ChangeNoteEvents::flip
//---------------------------------------------------------

void ChangeNoteEvents::flip(EditData*)
      {
/*TODO:      QList<NoteEvent*> e = chord->playEvents();
      chord->setPlayEvents(events);
      events = e;
      */
      }

//---------------------------------------------------------
//   ChangeInstrument::flip
//---------------------------------------------------------

void ChangeInstrument::flip(EditData*)
      {
      Instrument* oi = is->instrument();  //new Instrument(*is->instrument());
      is->setInstrument(instrument);      //*instrument

      // transpose
      int tickStart = is->segment()->tick();
      auto i = is->staff()->part()->instruments()->find(tickStart);
      ++i;
      int tickEnd;
      if (i == is->staff()->part()->instruments()->end())
            tickEnd = -1;
      else
            tickEnd = i->first;
      is->score()->transpositionChanged(is->staff()->part(), oi->transpose(), tickStart, tickEnd);

      is->masterScore()->rebuildMidiMapping();
      is->score()->setInstrumentsChanged(true);
      is->score()->setLayoutAll();
      instrument = oi;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void SwapCR::flip(EditData*)
      {
      Segment* s1 = cr1->segment();
      Segment* s2 = cr2->segment();
      int track = cr1->track();

      Element* cr = s1->element(track);
      s1->setElement(track, s2->element(track));
      s2->setElement(track, cr);
      cr1->score()->setLayout(s1->tick());
      cr1->score()->setLayout(s2->tick());
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

void ChangeClefType::flip(EditData*)
      {
      ClefType ocl = clef->concertClef();
      ClefType otc = clef->transposingClef();

      clef->setConcertClef(concertClef);
      clef->setTransposingClef(transposingClef);

      clef->staff()->setClef(clef);
      Segment* segment = clef->segment();
      updateNoteLines(segment, clef->track());
      clef->score()->setLayoutAll();      // TODO: reduce layout to clef range

      concertClef     = ocl;
      transposingClef = otc;
      // layout the clef to align the currentClefType with the actual one immediately
      clef->layout();
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void MoveStaff::flip(EditData*)
      {
      Part* oldPart = staff->part();
      int idx = staff->rstaff();
      oldPart->removeStaff(staff);
      part->insertStaff(staff, rstaff);
      part = oldPart;
      rstaff = idx;
      staff->score()->setLayoutAll();
      }

//---------------------------------------------------------
//   ChangeStaffUserDist::flip
//---------------------------------------------------------

void ChangeStaffUserDist::flip(EditData*)
      {
      qreal v = staff->userDist();
      staff->setUserDist(dist);
      dist = v;
      staff->score()->setLayoutAll();
      }

//---------------------------------------------------------
//   ChangeProperty::flip
//---------------------------------------------------------

void ChangeProperty::flip(EditData*)
      {
      qCDebug(undoRedo) << element->name() << int(id) << "(" << propertyName(id) << ")" << element->getProperty(id) << "->" << property;

//      if (id == Pid::SPANNER_TICK)
//            static_cast<Element*>(element)->score()->removeSpanner(static_cast<Spanner*>(element));

      QVariant v       = element->getProperty(id);
      PropertyFlags ps = element->propertyFlags(id);

      element->setProperty(id, property);
      element->setPropertyFlags(id, flags);

#if 0
      if (id == Pid::SPANNER_TICK) {
            static_cast<Element*>(element)->score()->addSpanner(static_cast<Spanner*>(element));
            // while updating ticks for an Ottava, the parent staff calls updateOttava()
            // and expects to find the Ottava spanner(s) in the score lists;
            // thus, the above (re)setProperty() left the staff pitchOffset map in a wrong state
            // as the spanner has been removed from the score lists; redo the map here
            if (static_cast<Element*>(element)->type() == ElementType::OTTAVA)
                  static_cast<Element*>(element)->staff()->updateOttava();
            }
#endif
      property = v;
      flags = ps;
      }

//---------------------------------------------------------
//   ChangeMetaText::flip
//---------------------------------------------------------

void ChangeMetaText::flip(EditData*)
      {
      QString s = score->metaTag(id);
      score->setMetaTag(id, text);
      text = s;
      }

//---------------------------------------------------------
//   ChangeEventList
//---------------------------------------------------------

ChangeEventList::ChangeEventList(Chord* c, const QList<NoteEventList> l)
   : chord(c), events(l)
      {
      eventListType = PlayEventType::User;
      }

//---------------------------------------------------------
//   ChangeEventList::flip
//---------------------------------------------------------

void ChangeEventList::flip(EditData*)
      {
      int n = chord->notes().size();
      for (int i = 0; i < n; ++i) {
            Note* note = chord->notes()[i];
            note->playEvents().swap(events[i]);
            }
      PlayEventType t = chord->playEventType();
      chord->setPlayEventType(eventListType);
      eventListType = t;
      }

//---------------------------------------------------------
//   ChangeSynthesizerState::flip
//---------------------------------------------------------

void ChangeSynthesizerState::flip(EditData*)
      {
      std::swap(state, score->_synthesizerState);
      }

void AddBracket::redo(EditData*)
      {
      staff->setBracketType(level, type);
      staff->setBracketSpan(level, span);
      staff->score()->setLayoutAll();
      }

void AddBracket::undo(EditData*)
      {
      staff->setBracketType(level, BracketType::NO_BRACKET);
      staff->score()->setLayoutAll();
      }

void RemoveBracket::redo(EditData*)
      {
      staff->setBracketType(level, BracketType::NO_BRACKET);
      staff->score()->setLayoutAll();
      }

void RemoveBracket::undo(EditData*)
      {
      staff->setBracketType(level, type);
      staff->setBracketSpan(level, span);
      staff->score()->setLayoutAll();
      }

//---------------------------------------------------------
//   ChangeSpannerElements
//---------------------------------------------------------

void ChangeSpannerElements::flip(EditData*)
      {
      Element*    oldStartElement   = spanner->startElement();
      Element*    oldEndElement     = spanner->endElement();
      if (spanner->anchor() == Spanner::Anchor::NOTE) {
            // be sure new spanner elements are of the right type
            if (!startElement->isNote() || !endElement->isNote())
                  return;
            Note* newStartNote;
            Note* newEndNote;
            Note* oldStartNote;
            Note* oldEndNote;
            int   startDeltaTrack   = oldStartElement->track() - startElement->track();
            int   endDeltaTrack     = oldEndElement->track() - endElement->track();
            // scan all spanners linked to this one
            for (ScoreElement* el : spanner->linkList()) {
                  Spanner*    sp    = static_cast<Spanner*>(el);
                  newStartNote      = newEndNote = nullptr;
                  oldStartNote      = toNote(sp->startElement());
                  oldEndNote        = toNote(sp->endElement());
                  // if not the current spanner, but one linked to it, determine its new start and end notes
                  // as modifications 'parallel' to the modifications of the current spanner's start and end notes
                  if (sp != spanner) {
                        // determine the track where to expect the 'parallel' start element
                        int   newTrack    = sp->startElement()->track() + startDeltaTrack;
                        // look in notes linked to new start note for a note with
                        // same score as linked spanner and appropriate track
                        for (ScoreElement* newEl : startElement->linkList())
                              if (static_cast<Note*>(newEl)->score() == sp->score()
                                          && static_cast<Note*>(newEl)->track() == newTrack) {
                                    newStartNote = static_cast<Note*>(newEl);
                                    break;
                                    }
                        // similarly to determine the 'parallel' end element
                        newTrack    = sp->endElement()->track() + endDeltaTrack;
                        for (ScoreElement* newEl : endElement->linkList())
                              if (static_cast<Note*>(newEl)->score() == sp->score()
                                          && static_cast<Note*>(newEl)->track() == newTrack) {
                                    newEndNote = static_cast<Note*>(newEl);
                                    break;
                                    }
                        }
                  // if current spanner, just use stored start and end elements
                  else {
                        newStartNote = toNote(startElement);
                        newEndNote   = toNote(endElement);
                        }
                  // update spanner's start and end notes
                  if (newStartNote && newEndNote) {
                        oldStartNote->removeSpannerFor(sp);
                        oldEndNote->removeSpannerBack(sp);
                        sp->setNoteSpan(newStartNote, newEndNote);
                        newStartNote->addSpannerFor(sp);
                        newEndNote->addSpannerBack(sp);

                        if (sp->isGlissando())
                              oldEndNote->chord()->updateEndsGlissando();
                        }
                  }
            }
      else {
            spanner->setStartElement(startElement);
            spanner->setEndElement(endElement);
            }
      startElement = oldStartElement;
      endElement   = oldEndElement;
      if (spanner->isTie()) {
            Tie* tie = toTie(spanner);
            toNote(endElement)->setTieBack(0);
            tie->endNote()->setTieBack(tie);
            toNote(startElement)->setTieFor(0);
            tie->startNote()->setTieFor(tie);
            }
      spanner->score()->setLayout(spanner->tick());
      spanner->score()->setLayout(spanner->tick2());
      }

//---------------------------------------------------------
//   ChangeParent
//---------------------------------------------------------

void ChangeParent::flip(EditData*)
      {
      Element* p = element->parent();
      int si = element->staffIdx();
      p->remove(element);
      element->setParent(parent);
      element->setTrack(staffIdx * VOICES);
      parent->add(element);
      staffIdx = si;
      parent = p;
      }

//---------------------------------------------------------
//   ChangeMMRest
//---------------------------------------------------------

void ChangeMMRest::flip(EditData*)
      {
      Measure* mmr = m->mmRest();
      m->setMMRest(mmrest);
      mmrest = mmr;
      }

//---------------------------------------------------------
//   InsertTime
//---------------------------------------------------------

void InsertTime::redo(EditData*)
      {
      score->insertTime(tick, len);
      }

void InsertTime::undo(EditData*)
      {
      score->insertTime(tick, -len);
      }

//---------------------------------------------------------
//   ChangeNoteEvent::flip
//---------------------------------------------------------

void ChangeNoteEvent::flip(EditData*)
      {
      note->score()->setPlaylistDirty();
      NoteEvent e = *oldEvent;
      *oldEvent   = newEvent;
      newEvent    = e;

      // TODO:
      note->chord()->setPlayEventType(PlayEventType::User);
      }

//---------------------------------------------------------
//   LinkUnlink
//---------------------------------------------------------

void LinkUnlink::doLink()
      {
      Q_ASSERT(le);
      e->linkTo(le);
      }

void LinkUnlink::doUnlink()
      {
      // find appropriate target element to unlink
      // use current le if valid; pick something else in link list if not but that shouldn't happen!

      const LinkedElements* l = e->links();
      if (l) {
            // don't use current le if null or if it is no longer linked (shouldn't happen)
            if (le && !l->contains(le)) {
                  le = nullptr;
                  qWarning("current le %p no longer linked", le);
                  }
            if (!le) {
                  // shouldn't happen
                  // find something other than current element (e) in link list, so we can link if asked to redo
                  for (ScoreElement* ee : *l) {
                        if (e != ee) {
                              le = ee;
                              break;
                              }
                        }
                  qDebug("current le was null... we picked a new one le %p", le);
                  }
            }
      else
            qWarning("current element %p(%s) has no links", e, e->name());

      if (e)
            e->unlink();
      else
            qWarning("nothing found to unlink");
      }


//---------------------------------------------------------
//   ChangeStartEndSpanner::flip
//---------------------------------------------------------

void ChangeStartEndSpanner::flip(EditData*)
      {
      Element* s = spanner->startElement();
      Element* e = spanner->endElement();
      spanner->setStartElement(start);
      spanner->setEndElement(end);
      start = s;
      end   = e;
      }

//---------------------------------------------------------
//   ChangeMetaTags::flip
//---------------------------------------------------------

void ChangeMetaTags::flip(EditData*)
      {
      QMap<QString,QString> t = score->metaTags();
      score->setMetaTags(metaTags);
      metaTags = t;
      }

//---------------------------------------------------------
//   ChangeDrumset::flip
//---------------------------------------------------------

void ChangeDrumset::flip(EditData*)
      {
      Drumset d = *instrument->drumset();
      instrument->setDrumset(&drumset);
      drumset = d;
      }

//---------------------------------------------------------
//   undoChangeGap
//---------------------------------------------------------

void ChangeGap::flip(EditData*)
      {
      rest->setGap(v);
      v = !v;
      }

//---------------------------------------------------------
//  ChangeText::insertText
//---------------------------------------------------------

void ChangeText::insertText(EditData* ed)
      {
      TextCursor tc = c;
      c.text()->editInsertText(&tc, s);
      c.text()->triggerLayout();
      if (ed) {
            TextCursor* ttc = c.text()->cursor(*ed);
            *ttc = tc;
            }
      }

//---------------------------------------------------------
//  ChangeText::removeText
//---------------------------------------------------------

void ChangeText::removeText(EditData* ed)
      {
      TextCursor tc = c;
      TextBlock& l  = c.curLine();
      int column    = c.column();

      for (int n = 0; n < s.size(); ++n)
            l.remove(column);
      c.text()->triggerLayout();
      if (ed)
            *c.text()->cursor(*ed) = tc;
      }

//---------------------------------------------------------
//   SplitText
//---------------------------------------------------------

void SplitText::undo(EditData* ed)
      {
      TextCursor tc = c;
      tc.deleteChar();
      c.text()->triggerLayout();
      if (ed)
            *c.text()->cursor(*ed) = tc;
      }

void SplitText::redo(EditData* ed)
      {
      TextCursor tc = c;
      TextBase* t   = tc.text();
      int line      = tc.row();

      CharFormat* charFmt = tc.format();         // take current format
      t->textBlockList().insert(line + 1, tc.curLine().split(tc.column()));
      t->textBlock(line).setEol(true);
      t->setTextInvalid();
      if (t->textBlockList().last() != t->textBlock(line+1))
            t->textBlock(line+1).setEol(true);

      c.text()->triggerLayout();
      if (ed) {
            tc.setRow(line+1);
            tc.setColumn(0);
            tc.setFormat(*charFmt);             // restore orig. format at new line
            *c.text()->cursor(*ed) = tc;
            }
      }

//---------------------------------------------------------
//   JoinText
//---------------------------------------------------------

void JoinText::redo(EditData* ed)
      {
      TextBase* t   = c.text();
      int line      = c.row();
      t->setTextInvalid();
      t->triggerLayout();

      CharFormat* charFmt = c.format();         // take current format
      int col             = t->textBlock(line-1).columns();
      int eol             = t->textBlock(line).eol();
      t->textBlock(line-1).fragments().append(t->textBlock(line).fragments());
      int lines = t->rows();
      if (line < lines)
            t->textBlock(line).setEol(eol);
      t->textBlockList().removeAt(line);
      c.setRow(line-1);
      c.setColumn(col);
      c.setFormat(*charFmt);             // restore orig. format at new line
      c.clearSelection();
      if (ed)
            *t->cursor(*ed) = c;
      }

void JoinText::undo(EditData* ed)
      {
      TextBase* t   = c.text();
      int line      = c.row();
      t->setTextInvalid();
      t->triggerLayout();

      CharFormat* charFmt = c.format();         // take current format
      t->textBlockList().insert(line + 1, c.curLine().split(c.column()));
      c.curLine().setEol(true);

      c.setRow(line+1);
      c.setColumn(0);
      c.setFormat(*charFmt);             // restore orig. format at new line
      c.clearSelection();

      if (ed)
            *t->cursor(*ed) = c;
      }

}
