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

#include "global/log.h"

#include "accidental.h"
#include "articulation.h"
#include "barline.h"
#include "beam.h"
#include "bend.h"
#include "bracket.h"
#include "chord.h"
#include "chordline.h"
#include "clef.h"
#include "dynamic.h"
#include "element.h"
#include "excerpt.h"
#include "fret.h"
#include "glissando.h"
#include "hairpin.h"
#include "harmony.h"
#include "input.h"
#include "instrchange.h"
#include "key.h"
#include "keysig.h"
#include "measure.h"
#include "note.h"
#include "noteevent.h"
#include "page.h"
#include "part.h"
#include "pitchspelling.h"
#include "score.h"
#include "segment.h"
#include "select.h"
#include "sequencer.h"
#include "spanner.h"
#include "staff.h"
#include "stafflines.h"
#include "stafftype.h"
#include "sym.h"
#include "system.h"
#include "tempotext.h"
#include "textedit.h"
#include "textline.h"
#include "tie.h"
#include "tremolo.h"
#include "tremolobar.h"
#include "tuplet.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"

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
      for (auto c : qAsConst(childList))
            delete c;
      }

//---------------------------------------------------------
//   UndoCommand::cleanup
//---------------------------------------------------------

void UndoCommand::cleanup(bool undo)
      {
      for (auto c : qAsConst(childList))
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
//   appendChildren
///   Append children of \p other into this UndoCommand.
///   Ownership over child commands of \p other is
///   transferred to this UndoCommand.
//---------------------------------------------------------

void UndoCommand::appendChildren(UndoCommand* other)
      {
      childList.append(other->childList);
      other->childList.clear();
      }

//---------------------------------------------------------
//   hasFilteredChildren
//---------------------------------------------------------

bool UndoCommand::hasFilteredChildren(UndoCommand::Filter f, const Element* target) const
      {
      for (UndoCommand* cmd : childList) {
            if (cmd->isFiltered(f, target))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   hasUnfilteredChildren
//---------------------------------------------------------

bool UndoCommand::hasUnfilteredChildren(const std::vector<UndoCommand::Filter>& filters, const Element* target) const
      {
      for (UndoCommand* cmd : childList) {
            bool filtered = false;
            for (UndoCommand::Filter f : filters) {
                  if (cmd->isFiltered(f, target)) {
                        filtered = true;
                        break;
                        }
                  }
            if (!filtered)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   filterChildren
//---------------------------------------------------------

void UndoCommand::filterChildren(UndoCommand::Filter f, Element* target)
      {
      QList<UndoCommand*> acceptedList;
      for (UndoCommand* cmd : qAsConst(childList)) {
            if (cmd->isFiltered(f, target))
                  delete cmd;
            else
                  acceptedList.push_back(cmd);
            }
      childList = std::move(acceptedList);
      }

//---------------------------------------------------------
//   unwind
//---------------------------------------------------------

void UndoCommand::unwind()
      {
      while (!childList.empty()) {
            UndoCommand* c = childList.takeLast();
            qDebug("unwind <%s>", c->name());
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
      cleanState = 0;
      stateList.push_back(cleanState);
      nextState = 1;
      }

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

UndoStack::~UndoStack()
      {
      int idx = 0;
      for (auto c : qAsConst(list))
            c->cleanup(idx++ < curIdx);
      qDeleteAll(list);
      }

//---------------------------------------------------------
//   beginMacro
//---------------------------------------------------------

void UndoStack::beginMacro(Score* score)
      {
      if (curCmd) {
            qDebug("already active");
            return;
            }
      curCmd = new UndoMacro(score);
      }

//---------------------------------------------------------
//   push
//---------------------------------------------------------

void UndoStack::push(UndoCommand* cmd, EditData* ed)
      {
      if (!curCmd) {
            // this can happen for layout() outside of a command (load)
            if (!ScoreLoad::loading())
                  qDebug("no active command, UndoStack");

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
            if (!ScoreLoad::loading())
                  qDebug("no active command, UndoStack %p", this);
            return;
            }
      curCmd->appendChild(cmd);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void UndoStack::remove(int idx)
      {
      Q_ASSERT(idx <= curIdx);
      Q_ASSERT(curIdx >= 0);
      // remove redo stack
      while (list.size() > curIdx) {
            UndoCommand* cmd = list.takeLast();
            stateList.pop_back();
            cmd->cleanup(false);  // delete elements for which UndoCommand() holds ownership
            delete cmd;
//            --curIdx;
            }
      while (list.size() > idx) {
            UndoCommand* cmd = list.takeLast();
            stateList.pop_back();
            cmd->cleanup(true);
            delete cmd;
            }
      curIdx = idx;
      }

//---------------------------------------------------------
//   mergeCommands
//---------------------------------------------------------

void UndoStack::mergeCommands(int startIdx)
      {
      Q_ASSERT(startIdx <= curIdx);

      if (startIdx >= list.size())
            return;

      UndoMacro* startMacro = list[startIdx];

      for (int idx = startIdx + 1; idx < curIdx; ++idx)
            startMacro->append(std::move(*list[idx]));
      remove(startIdx + 1); // TODO: remove from startIdx to curIdx only
      }

//---------------------------------------------------------
//   pop
//---------------------------------------------------------

void UndoStack::pop()
      {
      if (!curCmd) {
            if (!ScoreLoad::loading())
                  qDebug("no active command");
            return;
            }
      UndoCommand* cmd = curCmd->removeChild();
      cmd->undo(0);
      }

//---------------------------------------------------------
//   rollback
//---------------------------------------------------------

void UndoStack::rollback()
      {
      qDebug("==");
      Q_ASSERT(curCmd == 0);
      Q_ASSERT(curIdx > 0);
      int idx = curIdx - 1;
      list[idx]->unwind();
      remove(idx);
      }

//---------------------------------------------------------
//   endMacro
//---------------------------------------------------------

void UndoStack::endMacro(bool rollback)
      {
      if (curCmd == 0) {
            qDebug("not active");
            return;
            }
      if (rollback)
            delete curCmd;
      else {
            // remove redo stack
            while (list.size() > curIdx) {
                  UndoCommand* cmd = list.takeLast();
                  stateList.pop_back();
                  cmd->cleanup(false);  // delete elements for which UndoCommand() holds ownership
                  delete cmd;
                  }
            list.append(curCmd);
            stateList.push_back(nextState++);
            ++curIdx;
            }
      curCmd = 0;
      }

//---------------------------------------------------------
//   reopen
//---------------------------------------------------------

void UndoStack::reopen()
      {
      qDebug("curIdx %d size %d", curIdx, list.size());
      Q_ASSERT(curCmd == 0);
      Q_ASSERT(curIdx > 0);
      --curIdx;
      curCmd = list.takeAt(curIdx);
      stateList.erase(stateList.begin() + curIdx);
      for (auto i : curCmd->commands()) {
            qDebug("   <%s>", i->name());
            }
      }

//---------------------------------------------------------
//   setClean
//---------------------------------------------------------

void UndoStack::setClean()
      {
      cleanState = state();
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void UndoStack::undo(EditData* ed)
      {
      qCDebug(undoRedo) << "===";
      // Are we currently editing text?
      if (ed && ed->element && ed->element->isTextBase()) {
            TextEditData* ted = static_cast<TextEditData*>(ed->getData(ed->element));
            if (ted && ted->startUndoIdx == curIdx)
                  // No edits to undo, so do nothing
                  return;
            }
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
//   UndoMacro
//---------------------------------------------------------

bool UndoMacro::canRecordSelectedElement(const Element* e)
      {
      return e->isNote() || (e->isChordRest() && !e->isChord()) || (e->isTextBase() && !e->isInstrumentName()) || e->isFretDiagram();
      }

void UndoMacro::fillSelectionInfo(SelectionInfo& info, const Selection& sel)
      {
      info.staffStart = info.staffEnd = -1;
      info.elements.clear();

      if (sel.isList()) {
            for (Element* e : sel.elements()) {
                  if (canRecordSelectedElement(e))
                        info.elements.push_back(e);
                  else {
                        // don't remember selection we are unable to restore
                        info.elements.clear();
                        return;
                        }
                  }
            }
      else if (sel.isRange()) {
            info.staffStart = sel.staffStart();
            info.staffEnd = sel.staffEnd();
            info.tickStart = sel.tickStart();
            info.tickEnd = sel.tickEnd();
            }
      }

void UndoMacro::applySelectionInfo(const SelectionInfo& info, Selection& sel)
      {
      if (!info.elements.empty()) {
            for (Element* e : info.elements)
                  sel.add(e);
            }
      else if (info.staffStart != -1) {
            sel.setRangeTicks(info.tickStart, info.tickEnd, info.staffStart, info.staffEnd);
            }
      }

UndoMacro::UndoMacro(Score* s)
   : undoInputState(s->inputState()), score(s)
      {
      fillSelectionInfo(undoSelectionInfo, s->selection());
      }

void UndoMacro::undo(EditData* ed)
      {
      redoInputState = score->inputState();
      fillSelectionInfo(redoSelectionInfo, score->selection());
      score->deselectAll();

      // Undo for child commands.
      UndoCommand::undo(ed);

      score->setInputState(undoInputState);
      if (undoSelectionInfo.isValid()) {
            score->deselectAll();
            applySelectionInfo(undoSelectionInfo, score->selection());
            }
      }

void UndoMacro::redo(EditData* ed)
      {
      undoInputState = score->inputState();
      fillSelectionInfo(undoSelectionInfo, score->selection());
      score->deselectAll();

      // Redo for child commands.
      UndoCommand::redo(ed);

      score->setInputState(redoInputState);
      if (redoSelectionInfo.isValid()) {
            score->deselectAll();
            applySelectionInfo(redoSelectionInfo, score->selection());
            }
      }

void UndoMacro::append(UndoMacro&& other)
      {
      appendChildren(&other);
      if (score == other.score) {
            redoInputState = std::move(other.redoInputState);
            redoSelectionInfo = std::move(other.redoSelectionInfo);
            }
      }

//---------------------------------------------------------
//   CloneVoice
//---------------------------------------------------------

CloneVoice::CloneVoice(Segment* _sf, const Fraction& _lTick, Segment* _d, int _strack, int _dtrack, int _otrack, bool _linked)
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
      Fraction ticks = d->tick() + lTick - sf->tick();
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
                  auto spanners = s->spannerMap().findOverlapping(sf->tick().ticks(), lTick.ticks());
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
                  s->setRest(d->tick(), sTrack, ticks, false, 0);
            }
      else {
            s->cloneVoice(sTrack, dTrack, sf, ticks, linked);
            if (!linked && !(dTrack % VOICES))
                  s->setRest(d->tick(), dTrack, ticks, false, 0);
            }

      first = false;
      }

void CloneVoice::redo(EditData*)
      {
      Score* s = d->score();
      Fraction ticks = d->tick() + lTick - sf->tick();

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
                  auto spanners = s->spannerMap().findOverlapping(sf->tick().ticks(), lTick.ticks());
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
                  s->setRest(d->tick(), strack, ticks, false, 0);
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
            element->triggerLayout();
            element->score()->setLayout(element->staff()->nextClefTick(element->tick()), element->staffIdx());
            }
      else if (element->isKeySig()) {
            element->triggerLayout();
            element->score()->setLayout(element->staff()->nextKeyTick(element->tick()), element->staffIdx());
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
//   AddElement::isFiltered
//---------------------------------------------------------

bool AddElement::isFiltered(UndoCommand::Filter f, const Element* target) const
      {
      using Filter = UndoCommand::Filter;
      switch (f) {
            case Filter::AddElement:
                  return target == element;
            case Filter::AddElementLinked:
                  return target->linkList().contains(element);
            default:
                  break;
            }
      return false;
      }

//---------------------------------------------------------
//   removeNote
//    Helper function for RemoveElement class
//---------------------------------------------------------

static void removeNote(const Note* note)
      {
      Score* score = note->score();
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
                        removeNote(note);
                        }
                  }
            }
      else if (element->isNote()) {
            // Removing an individual note within a chord
            const Note* note = toNote(element);
            removeNote(note);
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
                        note->connectTiedNotes();
                        }
                  }
            undoAddTuplet(toChordRest(element));
            }
      else if (element->isClef())
            element->score()->setLayout(element->staff()->nextClefTick(element->tick()), element->staffIdx());
      else if (element->isKeySig())
            element->score()->setLayout(element->staff()->nextKeyTick(element->tick()), element->staffIdx());
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
                        note->disconnectTiedNotes();
                        }
                  }
            }
      else if (element->isClef())
            element->score()->setLayout(element->staff()->nextClefTick(element->tick()), element->staffIdx());
      else if (element->isKeySig())
            element->score()->setLayout(element->staff()->nextKeyTick(element->tick()), element->staffIdx());
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
//   RemoveElement::isFiltered
//---------------------------------------------------------

bool RemoveElement::isFiltered(UndoCommand::Filter f, const Element* target) const
      {
      using Filter = UndoCommand::Filter;
      switch (f) {
            case Filter::RemoveElement:
                  return target == element;
            case Filter::RemoveElementLinked:
                  return target->linkList().contains(element);
            default:
                  break;
            }
      return false;
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
//   MapExcerptTracks
//---------------------------------------------------------

MapExcerptTracks::MapExcerptTracks(Score* s, QList<int> l)
      {
      score = s;

      /*
       *    In list l [x] represents the previous index of the staffIdx x.
       *    If the a staff x is a newly added staff, l[x] = -1.
       *    For the "undo" all staves which value -1 are *not* remapped since
       *    it is assumed this staves are removed later.
       */
      for (int i = 0; i < l.size(); ++i) {
            if (l[i] >= 0)
                  rlist.insert(l[i], i);
            }
      list = l;
      }

void MapExcerptTracks::redo(EditData*)
      {
      score->mapExcerptTracks(list);
      }

void MapExcerptTracks::undo(EditData*)
      {
      score->mapExcerptTracks(rlist);
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

      note->triggerLayout();
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
      note->triggerLayout();
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
      if (!score->selection().isRange()) {
            if (oldElement->selected())
                  score->deselect(oldElement);
            if (newElement->selected())
                  score->select(newElement, SelectType::ADD);
            }
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
//   ChangeKeySig
//---------------------------------------------------------

ChangeKeySig::ChangeKeySig(KeySig* k, KeySigEvent newKeySig, bool sc, bool addEvtToStaff)
   : keysig(k), ks(newKeySig), showCourtesy(sc), evtInStaff(addEvtToStaff)
      {}

void ChangeKeySig::flip(EditData*)
      {
      Segment* segment = keysig->segment();
      Fraction tick = segment->tick();
      Staff* staff = keysig->staff();

      const bool curEvtInStaff = (staff->currentKeyTick(tick) == tick);
      KeySigEvent curKey = keysig->keySigEvent();
      const bool curShowCourtesy = keysig->showCourtesy();

      keysig->setKeySigEvent(ks);
      keysig->setShowCourtesy(showCourtesy);

      // Add/remove the corresponding key events, if appropriate.
      if (evtInStaff)
            staff->setKey(tick, ks); // replace
      else if (curEvtInStaff)
            staff->removeKey(tick); // if nothing to add instead, just remove.

      // If no keysig event corresponds to the key signature then this keysig
      // is probably generated. Otherwise it is probably added manually.
      // Set segment flags according to this, layout will change it if needed.
      segment->setEnabled(evtInStaff);
      segment->setHeader(!evtInStaff && segment->rtick() == Fraction(0,1));

      showCourtesy = curShowCourtesy;
      ks           = curKey;
      evtInStaff   = curEvtInStaff;
      keysig->triggerLayout();
      keysig->score()->setLayout(keysig->staff()->nextKeyTick(tick), keysig->staffIdx());
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
      Fraction oLen = measure->ticks();

      //
      // move EndBarLine and TimeSigAnnounce
      // to end of measure:
      //

      std::list<Segment*> sl;
      for (Segment* s = measure->first(); s; s = s->next()) {
            if (!s->isEndBarLineType() && !s->isTimeSigAnnounceType())
                  continue;
            s->setRtick(len);
            sl.push_back(s);
            measure->remove(s);
            }
      measure->setTicks(len);
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
      harmony->realizedHarmony().setDirty(true); //harmony should be re-realized after transposition
      int baseTpc1 = harmony->baseTpc();
      int rootTpc1 = harmony->rootTpc();
      harmony->setBaseTpc(baseTpc);
      harmony->setRootTpc(rootTpc);
      harmony->setXmlText(harmony->harmonyName());
      harmony->render();
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

ChangeInstrumentShort::ChangeInstrumentShort(const Fraction&_tick, Part* p, QList<StaffName> t)
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

ChangeInstrumentLong::ChangeInstrumentLong(const Fraction& _tick, Part* p, QList<StaffName> t)
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
      op.prog          = channel->program();
      op.bank          = channel->bank();
      op.synti         = channel->synti();

      channel->setProgram(patch.prog);
      channel->setBank(patch.bank);
      channel->setSynti(patch.synti);

      patch            = op;

      if (MScore::seq == 0) {
            qDebug("no seq");
            return;
            }

      NPlayEvent event;
      event.setType(ME_CONTROLLER);
      event.setChannel(channel->channel());

      int hbank = (channel->bank() >> 7) & 0x7f;
      int lbank = channel->bank() & 0x7f;

      event.setController(CTRL_HBANK);
      event.setValue(hbank);
      MScore::seq->sendEvent(event);

      event.setController(CTRL_LBANK);
      event.setValue(lbank);
      MScore::seq->sendEvent(event);

      event.setController(CTRL_PROGRAM);
      event.setValue(channel->program());

      score->setInstrumentsChanged(true);

      MScore::seq->sendEvent(event);
      }

//---------------------------------------------------------
//   SetUserBankController
//---------------------------------------------------------

void SetUserBankController::flip(EditData*)
      {
      bool oldVal = channel->userBankController();
      channel->setUserBankController(val);
      val = oldVal;
      }

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

ChangeStaff::ChangeStaff(Staff* _staff,  bool _invisible, ClefTypeList _clefType,
   qreal _userDist, Staff::HideMode _hideMode, bool _showIfEmpty, bool _cutaway, 
   bool _hideSystemBarLine, bool  _mergeMatchingRests)
      {
      staff       = _staff;
      invisible   = _invisible;
      clefType    = _clefType;
      userDist    = _userDist;
      hideMode    = _hideMode;
      showIfEmpty = _showIfEmpty;
      cutaway     = _cutaway;
      hideSystemBarLine  = _hideSystemBarLine;
      mergeMatchingRests = _mergeMatchingRests;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStaff::flip(EditData*)
      {
      bool invisibleChanged = staff->invisible(Fraction(0,1)) != invisible;
      ClefTypeList oldClefType = staff->defaultClefType();
      bool oldInvisible   = staff->invisible(Fraction(0,1));
      qreal oldUserDist   = staff->userDist();
      Staff::HideMode oldHideMode    = staff->hideWhenEmpty();
      bool oldShowIfEmpty = staff->showIfEmpty();
      bool oldCutaway     = staff->cutaway();
      bool oldHideSystemBarLine  = staff->hideSystemBarLine();
      bool oldMergeMatchingRests = staff->mergeMatchingRests();

      staff->setInvisible(Fraction(0,1),invisible);
      staff->setDefaultClefType(clefType);
      staff->setUserDist(userDist);
      staff->setHideWhenEmpty(hideMode);
      staff->setShowIfEmpty(showIfEmpty);
      staff->setCutaway(cutaway);
      staff->setHideSystemBarLine(hideSystemBarLine);
      staff->setMergeMatchingRests(mergeMatchingRests);

      invisible   = oldInvisible;
      clefType    = oldClefType;
      userDist    = oldUserDist;
      hideMode    = oldHideMode;
      showIfEmpty = oldShowIfEmpty;
      cutaway     = oldCutaway;
      hideSystemBarLine  = oldHideSystemBarLine;
      mergeMatchingRests = oldMergeMatchingRests;

      Score* score = staff->score();
      if (invisibleChanged) {
            int staffIdx = staff->idx();
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure())
                  m->staffLines(staffIdx)->setVisible(!staff->invisible(Fraction(0,1)));
            }
      staff->triggerLayout();
      staff->masterScore()->rebuildMidiMapping();
      staff->score()->setPlaylistDirty();
      }

//---------------------------------------------------------
//   ChangeStaffType::flip
//---------------------------------------------------------

void ChangeStaffType::flip(EditData*)
      {
      StaffType st = *staff->staffType(Fraction(0,1));      // TODO

      staff->setStaffType(Fraction(0,1), staffType);

      staffType = st;

      staff->triggerLayout();
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
      Instrument* oi = part->instrument();  //tick?
      QString s      = part->partName();
      part->setInstrument(instrument);
      part->setPartName(partName);

      part->updateHarmonyChannels(false);

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

static void changeChordStyle(Score* score)
      {
      score->style().chordList()->unload();
      qreal emag = score->styleD(Sid::chordExtensionMag);
      qreal eadjust = score->styleD(Sid::chordExtensionAdjust);
      qreal mmag = score->styleD(Sid::chordModifierMag);
      qreal madjust = score->styleD(Sid::chordModifierAdjust);
      score->style().chordList()->configureAutoAdjust(emag, eadjust, mmag, madjust);
      if (score->styleB(Sid::chordsXmlFile))
            score->style().chordList()->read("chords.xml");
      score->style().chordList()->read(score->styleSt(Sid::chordDescriptionFile));
      score->style().setCustomChordList(score->styleSt(Sid::chordStyle) == "custom");
      }

//---------------------------------------------------------
//   ChangeStyle
//---------------------------------------------------------

ChangeStyle::ChangeStyle(Score* s, const MStyle& st, const bool overlapOnly)
   : score(s), style(st), overlap(overlapOnly)
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
      if (score->styleV(Sid::musicalSymbolFont) != style.value(Sid::musicalSymbolFont)) {
            score->setScoreFont(ScoreFont::fontFactory(style.value(Sid::musicalSymbolFont).toString()));
            }

      score->setStyle(style, overlap);
      changeChordStyle(score);
      score->styleChanged();
      style = tmp;
      }

void ChangeStyle::undo(EditData* ed)
      {
      overlap = false;
      UndoCommand::undo(ed);
      }

//---------------------------------------------------------
//   ChangeStyleVal::flip
//---------------------------------------------------------

void ChangeStyleVal::flip(EditData*)
      {
      QVariant v = score->styleV(idx);
      if (v != value) {
            score->style().set(idx, value);
            switch (idx) {
                  case Sid::chordExtensionMag:
                  case Sid::chordExtensionAdjust:
                  case Sid::chordModifierMag:
                  case Sid::chordModifierAdjust:
                  case Sid::chordDescriptionFile:
                        changeChordStyle(score);
                        break;
                  case Sid::spatium:
                        score->spatiumChanged(v.toDouble(), value.toDouble());
                        break;
                  case Sid::defaultsVersion:
                        score->style().setDefaultStyleVersion(value.toInt());
                        break;
                  default:
                        break;
                  }
            score->styleChanged();
            }
      value = v;
      }

//---------------------------------------------------------
//   ChangePageNumberOffset::flip
//---------------------------------------------------------

void ChangePageNumberOffset::flip(EditData*)
      {
      int po = score->pageNumberOffset();

      score->setPageNumberOffset(pageOffset);
      score->setLayoutAll();

      pageOffset = po;
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
   : measure(m), staffIdx(i), visible(v), stemless(s)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeMStaffProperties::flip(EditData*)
      {
      bool v = measure->visible(staffIdx);
      bool s = measure->stemless(staffIdx);
      measure->setStaffVisible(staffIdx, visible);
      measure->setStaffStemless(staffIdx, stemless);
      visible    = v;
      stemless = s;
      }

//---------------------------------------------------------
//   getCourtesyClefs
//    remember clefs at the end of previous measure
//---------------------------------------------------------

std::vector<Clef*> InsertRemoveMeasures::getCourtesyClefs(Measure* m)
      {
      Score* score = m->score();
      std::vector<Clef*> startClefs;
      if (m->prev() && m->prev()->isMeasure()) {
            Measure* prevMeasure = toMeasure(m->prev());
            const Segment* clefSeg = prevMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef, prevMeasure->ticks());
            if (clefSeg) {
                  for (int st = 0; st < score->nstaves(); ++st) {
                        Element* clef = clefSeg->element(staff2track(st));
                        if (clef && clef->isClef())
                              startClefs.push_back(toClef(clef));
                        }
                  }
            }
      return startClefs;
      }

//---------------------------------------------------------
//   insertMeasures
//---------------------------------------------------------

void InsertRemoveMeasures::insertMeasures()
      {
      Score* score = fm->score();
      QList<Clef*> clefs;
      std::vector<Clef*> prevMeasureClefs;
      QList<KeySig*> keys;
      Segment* fs = 0;
      Segment* ls = 0;
      if (fm->isMeasure()) {
            score->setPlaylistDirty();
            fs = toMeasure(fm)->first();
            ls = toMeasure(lm)->last();
            for (Segment* s = fs; s && s != ls; s = s->next1()) {
                  if (!s->enabled() || !(s->segmentType() & (SegmentType::Clef | SegmentType::HeaderClef | SegmentType::KeySig)))
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
            prevMeasureClefs = getCourtesyClefs(toMeasure(fm));
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
            for (Clef* clef : prevMeasureClefs)
                  clef->staff()->setClef(clef);
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

      Fraction tick1 = fm->tick();
      Fraction tick2 = lm->endTick();

      if (fm->isMeasure() && lm->isMeasure()) {
            // remove beams from chordrests in affected area, they will be rebuilt later but we need
            // to avoid situations where notes from deleted measures remain in beams
            // when undoing, we need to check the previous measure as well as there could be notes in there
            // that need to have their beams recalculated (esp. when adding time signature)
            MeasureBase* prev = fm->prev();
            Segment* first = toMeasure(prev && prev->isMeasure() ? prev : fm)->first();
            for (Segment* s = first; s && s != toMeasure(lm)->last(); s = s->next1()) {
                  if (!s)
                        break;
                  if (!s->isChordRestType())
                        continue;

                  for (int track = 0; track < score->ntracks(); ++track) {
                        Element* e = s->element(track);
                        if (e && e->isChordRest())
                              toChordRest(e)->removeDeleteBeam(false);
                        }
                  }
            }

      QList<System*> systemList;
      for (MeasureBase* mb = lm;; mb = mb->prev()) {
            System* system = mb->system();
            if (system) {
                  if (!systemList.contains(system)) {
                        systemList.push_back(system);
                        }
                  system->removeMeasure(mb);
                  }
            if (mb == fm)
                  break;
            }

      // move subsequent StaffTypeChanges
      #if 0 // unimplemented
      if (moveStc) {
            for (Staff* staff : score->staves()) {
                  Fraction tickStart = tick1;
                  Fraction tickEnd = tick2;

                  // loop trhu, until the last one
                  auto stRange = staff->staffTypeRange(tickEnd);
                  int moveTick = stRange.first == tickEnd.ticks() ? stRange.first : stRange.second;

                  while (moveTick != -1) {
                        Fraction tick = Fraction::fromTicks(moveTick);
                        Fraction newTick = tick + tickStart - tickEnd;

                        Measure* measure = score->tick2measure(tick);
                        bool stIcon = false;

                        staff->moveStaffType(tick, newTick);

                        for (EngravingItem* el : measure->el()) {
                              if (el && el->isStaffTypeChange() && el->track() == staff->idx() * VOICES) {
                                    StaffTypeChange* stc = toStaffTypeChange(el);
                                    stc->setStaffType(staff->staffType(newTick), false);
                                    stIcon = true;
                                    break;
                                    }
                              }

                        if (!stIcon) {
                              LOG_UNDO() << "StaffTypeChange icon is missing in measure " << measure->no();
                              }

                        stRange = staff->staffTypeRange(tick);
                        moveTick = stRange.second;
                        }
                  }
            }
      #endif

      score->measures()->remove(fm, lm);

      if (fm->isMeasure()) {
            #if 0 // unimplemented
            score->setUpTempoMap();
            score->rebuildTempoAndTimeSigMaps(fm); // alternative?
            #endif
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

            // remember clefs at the end of previous measure
            const auto clefs = getCourtesyClefs(toMeasure(fm));

            score->insertTime(tick1, -(tick2 - tick1));

            // Restore clefs that were backed up. Events for them could be lost
            // as a result of the recent insertTime() call.
            for (Clef* clef : clefs)
                  clef->staff()->setClef(clef);

            std::set<Spanner*> spannersCopy = score->unmanagedSpanners();
            for (Spanner* sp : spannersCopy) {
                  if ((sp->tick() >= tick1 && sp->tick() < tick2) || (sp->tick2() >= tick1 && sp->tick2() < tick2)) {
                        sp->removeUnmanaged();
                        }
                  }
            }

      // remove empty systems

      for (System* s : systemList) {
            if (s->measures().empty()) {
                  Page* page = s->page();
                  if (page) {
                        // erase system from page
                        QList<System*>& sl = page->systems();
                        auto i = std::find(sl.begin(), sl.end(), s);
                        if (i != sl.end())
                              sl.erase(i);
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
      score->excerpts().swapItemsAt(pos1, pos2);
#else
      score->excerpts().swap(pos1, pos2);
#endif
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
//   ChangeNoteEventList::flip
//---------------------------------------------------------

void ChangeNoteEventList::flip(EditData*)
      {
      note->score()->setPlaylistDirty();
      // Get copy of current list.
      NoteEventList nel = note->playEvents();
      // Replace current copy with new list.
      note->setPlayEvents(newEvents);
      // Save copy of replaced list.
      newEvents = nel;
      // Get a copy of the current playEventType.
      PlayEventType petval = note->chord()->playEventType();
      // Replace current setting with new setting.
      note->chord()->setPlayEventType(newPetype);
      // Save copy of old setting.
      newPetype = petval;
      }

//---------------------------------------------------------
//   ChangeNoteEventList::flip
//---------------------------------------------------------

void ChangeChordPlayEventType::flip(EditData*)
      {
      chord->score()->setPlaylistDirty();
      // Flips data between NoteEventList's.
      size_t n = chord->notes().size();
      for (size_t i = 0; i < n; ++i) {
            Note* note = chord->notes()[i];
            note->playEvents().swap(events[int(i)]);
            }
      // Flips PlayEventType between chord and undo.
      PlayEventType curPetype = chord->playEventType();
      chord->setPlayEventType(petype);
      petype = curPetype;
      }

//---------------------------------------------------------
//   ChangeInstrument::flip
//---------------------------------------------------------

void ChangeInstrument::flip(EditData*)
      {
      Part* part = is->staff()->part();
      Fraction tickStart = is->segment()->tick();
      Instrument* oi = is->instrument();  //new Instrument(*is->instrument());

      // set instrument in both part and instrument change element
      is->setInstrument(instrument);      //*instrument
      part->setInstrument(instrument, tickStart);

      // update score
      is->masterScore()->rebuildMidiMapping();
      is->masterScore()->updateChannel();
      is->score()->setInstrumentsChanged(true);
      is->triggerLayoutAll();

      // remember original instrument
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

      if (cr1->isChord() && cr2->isChord() && toChord(cr1)->tremolo()
         && (toChord(cr1)->tremolo() == toChord(cr2)->tremolo())) {
            Tremolo* t = toChord(cr1)->tremolo();
            Chord* c1 = t->chord1();
            Chord* c2 = t->chord2();
            t->setParent(toChord(c2));
            t->setChords(toChord(c2), toChord(c1));
            }

      Element* cr = s1->element(track);
      s1->setElement(track, s2->element(track));
      s2->setElement(track, cr);
      cr1->score()->setLayout(s1->tick(), cr1->staffIdx(), cr1);
      cr1->score()->setLayout(s2->tick(), cr1->staffIdx(), cr1);
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
      clef->triggerLayoutAll();      // TODO: reduce layout to clef range

      concertClef     = ocl;
      transposingClef = otc;
      // layout the clef to align the currentClefType with the actual one immediately
      clef->layout();
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------
#if 0 // MoveStaff is commented out in mscore/instrwidget.cpp, not used anywhere else
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
#endif

//---------------------------------------------------------
//   ChangeProperty::flip
//---------------------------------------------------------

void ChangeProperty::flip(EditData*)
      {
      qCDebug(undoRedo) << element->name() << int(id) << "(" << propertyName(id) << ")" << element->getProperty(id) << "->" << property;

      QVariant v       = element->getProperty(id);
      PropertyFlags ps = element->propertyFlags(id);

      element->setProperty(id, property);
      element->setPropertyFlags(id, flags);
      property = v;
      flags = ps;
      }

//---------------------------------------------------------
//   ChangeBracketProperty::flip
//---------------------------------------------------------

void ChangeBracketProperty::flip(EditData* ed)
      {
      element = staff->brackets()[level];
      ChangeProperty::flip(ed);
      level = toBracketItem(element)->column();
      }

//---------------------------------------------------------
//   ChangeTextLineProperty::flip
//---------------------------------------------------------

void ChangeTextLineProperty::flip(EditData* ed)
      {
      ChangeProperty::flip(ed);
      if (element->isTextLine())
            toTextLine(element)->initStyle();
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
      staff->triggerLayout();
      }

void AddBracket::undo(EditData*)
      {
      staff->setBracketType(level, BracketType::NO_BRACKET);
      staff->triggerLayout();
      }

void RemoveBracket::redo(EditData*)
      {
      staff->setBracketType(level, BracketType::NO_BRACKET);
      staff->triggerLayout();
      }

void RemoveBracket::undo(EditData*)
      {
      staff->setBracketType(level, type);
      staff->setBracketSpan(level, span);
      staff->triggerLayout();
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
            if (!startElement || !startElement->isNote() || !endElement || !endElement->isNote())
                  return;
            Note* oldStartNote = toNote(oldStartElement);
            Note* oldEndNote = toNote(oldEndElement);
            Note* newStartNote = toNote(startElement);
            Note* newEndNote = toNote(endElement);
            // update spanner's start and end notes
            if (newStartNote && newEndNote) {
                  spanner->setNoteSpan(newStartNote, newEndNote);
                  if (spanner->isTie()) {
                        Tie* tie = toTie(spanner);
                        oldStartNote->setTieFor(nullptr);
                        oldEndNote->setTieBack(nullptr);
                        newStartNote->setTieFor(tie);
                        newEndNote->setTieBack(tie);
                        }
                  else {
                        oldStartNote->removeSpannerFor(spanner);
                        oldEndNote->removeSpannerBack(spanner);
                        newStartNote->addSpannerFor(spanner);
                        newEndNote->addSpannerBack(spanner);
                        if (spanner->isGlissando())
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
      spanner->triggerLayout();
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
      element->setTrack(staffIdx * VOICES + element->voice());
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
//   InsertTimeUnmanagedSpanner::flip
//---------------------------------------------------------

void InsertTimeUnmanagedSpanner::flip(EditData*)
      {
      for (Score* s : score->scoreList()) {
            const auto unmanagedSpanners(s->unmanagedSpanners());
            for (Spanner* sp : unmanagedSpanners)
                  sp->insertTimeUnmanaged(tick, len);
            }
      len = -len;
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
      // Get a copy of the current playEventType.
      PlayEventType petval = note->chord()->playEventType();
      // Replace current setting with new setting.
      note->chord()->setPlayEventType(newPetype);
      // Save copy of old setting.
      newPetype = petval;
      }

//---------------------------------------------------------
//   LinkUnlink
//---------------------------------------------------------

LinkUnlink::~LinkUnlink()
      {
      if (le && mustDelete) {
            Q_ASSERT(le->size() <= 1);
            delete le;
            }
      }

void LinkUnlink::link()
      {
      if (le->size() == 1)
            le->front()->setLinks(le);
      mustDelete = false;
      le->append(e);
      e->setLinks(le);
      }

void LinkUnlink::unlink()
      {
      Q_ASSERT(le->contains(e));
      le->removeOne(e);
      if (le->size() == 1) {
            le->front()->setLinks(0);
            mustDelete = true;
            }

      e->setLinks(0);
      }

//---------------------------------------------------------
//   Link
//    link e1 to e2
//---------------------------------------------------------

Link::Link(ScoreElement* e1, ScoreElement* e2)
      {
      Q_ASSERT(e1->links() == 0);
      le = e2->links();
      if (!le) {
            if (e1->isStaff())
                  le = new LinkedElements(e1->score(), -1);
            else
                  le = new LinkedElements(e1->score());
            le->push_back(e2);
            }
      e = e1;
      }

//---------------------------------------------------------
//   Link::isFiltered
//---------------------------------------------------------

bool Link::isFiltered(UndoCommand::Filter f, const Element* target) const
      {
      using Filter = UndoCommand::Filter;
      if (f == Filter::Link)
            return e == target || le->contains(const_cast<Element*>(target));
      return false;
      }

//---------------------------------------------------------
//   Unlink
//---------------------------------------------------------

Unlink::Unlink(ScoreElement* _e)
      {
      e  = _e;
      le = e->links();
      Q_ASSERT(le);
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
//   FretDot
//---------------------------------------------------------

void FretDot::redo(EditData*)
      {
      undoData = FretUndoData(diagram);

      diagram->setDot(string, fret, add, dtype);
      diagram->triggerLayout();
      }


void FretDot::undo(EditData*)
      {
      undoData.updateDiagram();
      diagram->triggerLayout();
      }

//---------------------------------------------------------
//   FretMarker
//---------------------------------------------------------

void FretMarker::redo(EditData*)
      {
      undoData = FretUndoData(diagram);

      diagram->setMarker(string, mtype);
      diagram->triggerLayout();
      }

void FretMarker::undo(EditData*)
      {
      undoData.updateDiagram();
      diagram->triggerLayout();
      }

//---------------------------------------------------------
//   FretBarre
//---------------------------------------------------------

void FretBarre::redo(EditData*)
      {
      undoData = FretUndoData(diagram);

      diagram->setBarre(string, fret, add);
      diagram->triggerLayout();
      }

void FretBarre::undo(EditData*)
      {
      undoData.updateDiagram();
      diagram->triggerLayout();
      }

//---------------------------------------------------------
//   FretClear
//---------------------------------------------------------

void FretClear::redo(EditData*)
      {
      undoData = FretUndoData(diagram);

      diagram->clear();
      diagram->triggerLayout();
      }

void FretClear::undo(EditData*)
      {
      undoData.updateDiagram();
      diagram->triggerLayout();
      }

//---------------------------------------------------------
//   MoveTremolo
//---------------------------------------------------------

void MoveTremolo::redo(EditData*)
      {
      // Find new tremolo chords
      Measure* m1 = score->tick2measure(chord1Tick);
      Measure* m2 = score->tick2measure(chord2Tick);
      IF_ASSERT_FAILED(m1 && m2) {
            return;
            }
      Chord* c1 = m1->findChord(chord1Tick, track);
      Chord* c2 = m2->findChord(chord2Tick, track);
      IF_ASSERT_FAILED(c1 && c2) {
            return;
            }

      // Remember the old tremolo chords
      oldC1 = trem->chord1();
      oldC2 = trem->chord2();

      // Move tremolo away from old chords
      trem->chord1()->setTremolo(nullptr);
      trem->chord2()->setTremolo(nullptr);

      // Delete old tremolo on c1 and c2, if present
      if (c1->tremolo() && (c1->tremolo() != trem)) {
            if (c2->tremolo() == c1->tremolo())
                  c2->tremolo()->setChords(c1,c2);
            else
                  c1->tremolo()->setChords(c1,nullptr);
            Tremolo* oldTremolo  = c1->tremolo();
            c1->setTremolo(nullptr);
            delete oldTremolo;
            }
      if (c2->tremolo() && (c2->tremolo() != trem)) {
            c2->tremolo()->setChords(nullptr,c2);
            Tremolo* oldTremolo  = c2->tremolo();
            c2->setTremolo(nullptr);
            delete oldTremolo;
            }

      // Move tremolo to new chords
      c1->setTremolo(trem);
      c2->setTremolo(trem);
      trem->setChords(c1, c2);
      trem->setParent(c1);

      // Tremolo would cross barline, so remove it
      if (m1 != m2) {
            score->undoRemoveElement(trem);
            return;
            }
      // One of the notes crosses a barline, so remove the tremolo
      if (c1->ticks() != c2->ticks())
            score->undoRemoveElement(trem);
      }

void MoveTremolo::undo(EditData*)
      {
      // Move tremolo to old position
      trem->chord1()->setTremolo(nullptr);
      trem->chord2()->setTremolo(nullptr);
      oldC1->setTremolo(trem);
      oldC2->setTremolo(trem);
      trem->setChords(oldC1, oldC2);
      trem->setParent(oldC1);
      }

//---------------------------------------------------------
//   ChangeScoreOrder
//---------------------------------------------------------

void ChangeScoreOrder::flip(EditData*)
      {
      ScoreOrder* s = score->scoreOrder();
      score->setScoreOrder(order);
      order = s;
      }

}
