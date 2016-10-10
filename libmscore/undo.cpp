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
#include "sym.h"
#include "utils.h"
#include "glissando.h"

//      Q_LOGGING_CATEGORY(undoRedo, "undoRedo")

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
      if (staff->isDrumStaff() || staff->isTabStaff())
            return;
      for (Segment* s = segment->next1(); s; s = s->next1()) {
            if (s->segmentType() == Segment::Type::Clef && s->element(track) && !s->element(track)->generated())
                  break;
            if (s->segmentType() != Segment::Type::ChordRest)
                  continue;
            for (int t = track; t < track + VOICES; ++t) {
                  Chord* chord = static_cast<Chord*>(s->element(t));
                  if (chord && chord->type() == Element::Type::CHORD) {
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

void UndoCommand::undo()
      {
      int n = childList.size();
      for (int i = n-1; i >= 0; --i) {
            qCDebug(undoRedo) << "<" << childList[i]->name() << ">";
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
            qCDebug(undoRedo) << "<" << childList[i]->name() << ">";
            childList[i]->redo();
            }
      flip();
      }

//---------------------------------------------------------
//   unwind
//---------------------------------------------------------

void UndoCommand::unwind()
      {
      while (!childList.empty()) {
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

void UndoStack::push(UndoCommand* cmd)
      {
      if (!curCmd) {
            // this can happen for layout() outside of a command (load)
            // qWarning("UndoStack:push(): no active command, UndoStack %p", this);

            cmd->redo();
            delete cmd;
            return;
            }
#ifndef QT_NO_DEBUG
      if (!strcmp(cmd->name(), "ChangeProperty")) {
            ChangeProperty* cp = static_cast<ChangeProperty*>(cmd);
            qCDebug(undoRedo, "UndoStack::push <%s> id %s", cmd->name(), propertyName(cp->getId()));
            }
      else {
            qCDebug(undoRedo, "UndoStack::push <%s>", cmd->name());
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
            qWarning("UndoStack:push1(): no active command, UndoStack %p", this);
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
      cmd->undo();
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

void UndoStack::undo()
      {
      qCDebug(undoRedo) << "===";
      if (curIdx) {
            --curIdx;
            Q_ASSERT(curIdx >= 0);
            list[curIdx]->undo();
            }
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void UndoStack::redo()
      {
      qCDebug(undoRedo) << "===";
      if (canRedo())
            list[curIdx++]->redo();
      }

//---------------------------------------------------------
//   SaveState
//---------------------------------------------------------

SaveState::SaveState(Score* s)
   : undoInputState(s), redoInputState(s->inputState())
      {
      score          = s;
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

void Score::undoChangeProperty(ScoreElement* e, P_ID t, const QVariant& st, PropertyStyle ps)
      {
      if (propertyLink(t)) {
            for (ScoreElement* ee : e->linkList()) {
                  if (ee->getProperty(t) != st || ee->propertyStyle(t) != ps)
                        undo(new ChangeProperty(ee, t, st, ps));
                  }
            }
      else {
            if (e->getProperty(t) != st || e->propertyStyle(t) != ps)
                  undo(new ChangeProperty(e, t, st, ps));
            }
      }

//---------------------------------------------------------
//   undoPropertyChanged
//---------------------------------------------------------

void Score::undoPropertyChanged(Element* e, P_ID t, const QVariant& st)
      {
      if (propertyLink(t) && e->links()) {
            foreach (ScoreElement* ee, *e->links()) {
                  if (ee == e) {
                        if (ee->getProperty(t) != st)
                              undoStack()->push1(new ChangeProperty(ee, t, st));
                        }
                  else {
                        // property in linked element has not changed yet
                        // push() calls redo() to change it
                        if (ee->getProperty(t) != e->getProperty(t))
                              undoStack()->push(new ChangeProperty(ee, t, e->getProperty(t)));
                        }
                  }
            }
      else {
            if (e->getProperty(t) != st) {
                  undoStack()->push1(new ChangeProperty(e, t, st));
                  }
            }
      }

void Score::undoPropertyChanged(ScoreElement* e, P_ID t, const QVariant& st)
      {
      if (e->getProperty(t) != st)
            undoStack()->push1(new ChangeProperty(e, t, st));
      }

//---------------------------------------------------------
//   undoChangeElement
//---------------------------------------------------------

void Score::undoChangeElement(Element* oldElement, Element* newElement)
      {
      if (!oldElement)
            undoAddElement(newElement);
      else
            undo(new ChangeElement(oldElement, newElement));
      }

//---------------------------------------------------------
//   undoChangePitch
//---------------------------------------------------------

void Score::undoChangePitch(Note* note, int pitch, int tpc1, int tpc2)
      {
      for (ScoreElement* e : note->linkList()) {
            Note* n = static_cast<Note*>(e);
            undoStack()->push(new ChangePitch(n, pitch, tpc1, tpc2));
            }
      }

//---------------------------------------------------------
//   undoChangeFretting
//
//    To use with tablatures to force a specific note fretting;
//    Pitch, string and fret must be changed all together; otherwise,
//    if they are not consistent among themselves, the refretting algorithm may re-assign
//    fret and string numbers for (potentially) all the notes of all the chords of a segment.
//---------------------------------------------------------

void Score::undoChangeFretting(Note* note, int pitch, int string, int fret, int tpc1, int tpc2)
      {
      const LinkedElements* l = note->links();
      if (l) {
            for (ScoreElement* e : *l) {
                  Note* n = static_cast<Note*>(e);
                  undoStack()->push(new ChangeFretting(n, pitch, string, fret, tpc1, tpc2));
                  }
            }
      else
            undoStack()->push(new ChangeFretting(note, pitch, string, fret, tpc1, tpc2));
      }

//---------------------------------------------------------
//   undoChangeKeySig
//---------------------------------------------------------

void Score::undoChangeKeySig(Staff* ostaff, int tick, KeySigEvent key)
      {
      KeySig* lks = 0;

      for (Staff* staff : ostaff->staffList()) {
            if (staff->isDrumStaff())
                  continue;

            Score* score = staff->score();
            Measure* measure = score->tick2measure(tick);
            if (!measure) {
                  qWarning("measure for tick %d not found!", tick);
                  continue;
                  }
            Segment* s   = measure->undoGetSegment(Segment::Type::KeySig, tick);
            int staffIdx = staff->idx();
            int track    = staffIdx * VOICES;
            KeySig* ks   = toKeySig(s->element(track));

            Interval interval = staff->part()->instrument(tick)->transpose();
            KeySigEvent nkey  = key;
            bool concertPitch = score->styleB(StyleIdx::concertPitch);
            if (interval.chromatic && !concertPitch && !nkey.custom() && !nkey.isAtonal()) {
                  interval.flip();
                  nkey.setKey(transposeKey(key.key(), interval));
                  }
            if (ks) {
                  ks->undoChangeProperty(P_ID::GENERATED, false);
                  undo(new ChangeKeySig(ks, nkey, ks->showCourtesy()));
                  }
            else {
                  // do not create empty keysig unless custom or atonal
                  if (tick != 0 || nkey.key() != Key::C || nkey.custom() || nkey.isAtonal()) {
                        KeySig* nks = new KeySig(score);
                        nks->setParent(s);
                        nks->setTrack(track);
                        nks->setKeySigEvent(nkey);
                        undo(new AddElement(nks));
                        if (lks)
                              undo(new Link(lks, nks));
                        else
                              lks = nks;
                        }
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
      bool firstSeg = seg->measure()->first() == seg;

      Clef* gclef = 0;
      foreach (Staff* staff, ostaff->staffList()) {
            if (staff->staffType()->group() != ClefInfo::staffGroup(st))
                  continue;

            Score* score = staff->score();
            int tick     = seg->tick();
            Measure* measure = score->tick2measure(tick);
            if (!measure) {
                  qWarning("measure for tick %d not found!", tick);
                  continue;
                  }

            Segment* destSeg = measure->findSegment(Segment::Type::Clef, tick);

            // move measure-initial clef to last segment of prev measure

            if (firstSeg                        // if at start of measure
               && measure->prevMeasure()        // and there is a previous measure
               ) {
                  measure = measure->prevMeasure();
                  destSeg = measure->findSegment(Segment::Type::Clef, tick);
                  }

            if (!destSeg) {
                  destSeg = new Segment(measure, Segment::Type::Clef, seg->tick());
                  score->undoAddElement(destSeg);
                  }
            int staffIdx = staff->idx();
            int track    = staffIdx * VOICES;
            Clef* clef   = static_cast<Clef*>(destSeg->element(track));

            if (clef) {
                  //
                  // for transposing instruments, differentiate
                  // clef type for concertPitch
                  //
                  Instrument* i = staff->part()->instrument(tick);
                  ClefType cp, tp;
                  if (i->transpose().isZero()) {
                        cp = st;
                        tp = st;
                        }
                  else {
                        bool concertPitch = clef->concertPitch();
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
                  // change the clef in the mmRest if any
                  if (measure->hasMMRest()) {
                        Measure* mmMeasure = measure->mmRest();
                        Segment* mmDestSeg = mmMeasure->findSegment(Segment::Type::Clef, tick);
                        if (mmDestSeg) {
                              Clef* mmClef = static_cast<Clef*>(mmDestSeg->element(clef->track()));
                              if (mmClef)
                                    score->undo(new ChangeClefType(mmClef, cp, tp));
                              }
                        }
                  }
            else {
                  if (gclef) {
                        clef = static_cast<Clef*>(gclef->linkedClone());
                        clef->setScore(score);
                        }
                  else {
                        clef = new Clef(score);
                        gclef = clef;
                        }
                  clef->setTrack(track);
                  clef->setClefType(st);
                  clef->setParent(destSeg);
                  score->undo(new AddElement(clef));
                  clef->layout();
                  }
            }
      }

//---------------------------------------------------------
//   findLinkedVoiceElement
//---------------------------------------------------------

static Element* findLinkedVoiceElement(Element* e, Staff* nstaff)
      {
      Excerpt* se = e->score()->excerpt();
      Excerpt* de = nstaff->excerpt();
      int strack = e->track();
      int dtrack = nstaff->idx() * VOICES + e->voice();

      if (se)
            strack = se->tracks().key(strack);

      if (de) {
            QList<int> l = de->tracks().values(strack);
            if (l.isEmpty())
                  return 0;
            for (int i : l) {
                  if (nstaff->idx() * VOICES <= i && (nstaff->idx() + 1) * VOICES > i) {
                        dtrack = i;
                        break;
                        }
                  }
            }

      Score* score     = nstaff->score();
      Segment* segment = static_cast<Segment*>(e->parent());
      Measure* measure = segment->measure();
      Measure* m       = score->tick2measure(measure->tick());
      Segment* s       = m->findSegment(segment->segmentType(), segment->tick());
      return s->element(dtrack);
      }

//---------------------------------------------------------
//   findLinkedChord
//---------------------------------------------------------

static Chord* findLinkedChord(Chord* c, Staff* nstaff)
      {
      Excerpt* se = c->score()->excerpt();
      Excerpt* de = nstaff->excerpt();
      int strack = c->track();
      int dtrack = nstaff->idx() * VOICES + c->voice();

      if (se)
            strack = se->tracks().key(strack);

      if (de) {
            QList<int> l = de->tracks().values(strack);
            if (l.isEmpty())
                  return 0;
            for (int i : l) {
                  if (nstaff->idx() * VOICES <= i && (nstaff->idx() + 1) * VOICES > i) {
                        dtrack = i;
                        break;
                        }
                  }
            }

      Segment* s  = c->segment();
      Measure* nm = nstaff->score()->tick2measure(s->tick());
      Segment* ns = nm->findSegment(s->segmentType(), s->tick());
      Element* ne = ns->element(dtrack);
      if (ne->type() != Element::Type::CHORD)
            return 0;
      Chord* nc   = static_cast<Chord*>(ne);
      if (c->isGrace()) {
            Chord* pc = static_cast<Chord*>(c->parent());
            int index = 0;
            for (Chord* gc : pc->graceNotes()) {
                  if (c == gc)
                        break;
                  index++;
                  }
            if (index < nc->graceNotes().length())
                  nc = nc->graceNotes().at(index);
            }
      return nc;
      }

//---------------------------------------------------------
//   undoChangeChordRestLen
//---------------------------------------------------------

void Score::undoChangeChordRestLen(ChordRest* cr, const TDuration& d)
      {
      auto sl = cr->staff()->staffList();
      for (Staff* staff : sl) {
            ChordRest *ncr;
            if (cr->isGrace())
                  ncr = findLinkedChord(static_cast<Chord*>(cr), staff);
            else
                  ncr = static_cast<ChordRest*>(findLinkedVoiceElement(cr, staff));
            if (!ncr)
                  continue;
            ncr->undoChangeProperty(P_ID::DURATION_TYPE, QVariant::fromValue(d));
            ncr->undoChangeProperty(P_ID::DURATION, QVariant::fromValue(d.fraction()));
            }
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
      int tick = measure->tick();

      for (int staffIdx = staff1; staffIdx < staff2; ++staffIdx) {
            QSet<Staff*> sl;
            for (Staff* s : staff(staffIdx)->staffList())
                  sl.insert(s);

            int sTrack = staffIdx * VOICES;
            int s = sTrack + v1;
            int d = sTrack + v2;
            int diff = v2 - v1;

            //handle score and complete measures first
            undo(new ExchangeVoice(measure, s, d, staffIdx));

            for (Staff* st : sl) {
                  int stTrack = st->idx() * VOICES;
                  Measure* m = st->score()->tick2measure(tick);
                  Excerpt* ex = st->excerpt();
                  if (ex) {
                        QMultiMap<int, int> t = ex->tracks();
                        QList<int> ts = t.values(s);
                        QList<int> td = t.values(d);

                        for (int tss : ts) {
                              if (!(stTrack <= tss) || !(tss < stTrack + VOICES))
                                    continue;

                              int temp = t.key(tss);
                              QList<int> test = t.values(temp + diff);
                              bool hasVoice = false;
                              for (int te : test) {
                                    if (stTrack <= te && te < stTrack + VOICES && td.contains(te))
                                          hasVoice = true;
                                    }

                              if (!hasVoice) {
                                    undo(new CloneVoice(measure->first(), m->endTick(), m->first(), temp, tss, temp + diff));
                                    ts.removeOne(tss);
                                    }
                              }

                        for (int tdd : td) {
                              if (!(stTrack <= tdd) || !(tdd < stTrack + VOICES))
                                    continue;

                              int temp = t.key(tdd);
                              QList<int> test = t.values(temp - diff);
                              bool hasVoice = false;
                              for (int te : test) {
                                    if (stTrack <= te && te < stTrack + VOICES &&
                                        ts.contains(te))
                                          hasVoice = true;
                                    }

                              if (!hasVoice) {
                                    undo(new CloneVoice(measure->first(), m->endTick(), m->first(), temp, tdd, temp - diff));
                                    td.removeOne(tdd);
                                    }
                              }
                        }

                  }
            }

      // make sure voice 0 is complete

      if (v1 == 0 || v2 == 0) {
            for (int staffIdx = staff1; staffIdx < staff2; ++staffIdx) {
                  // check for complete timeline of voice 0
                  int ctick  = measure->tick();
                  int track = staffIdx * VOICES;
                  for (Segment* s = measure->first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
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

void CloneVoice::undo()
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

void CloneVoice::redo()
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
//    idx - index of staff in part
//---------------------------------------------------------

void Score::undoRemoveStaff(Staff* staff)
      {
      int idx = staff->idx();
      Q_ASSERT(idx >= 0);
      //
      //    adjust measures
      //
      for (Measure* m = staff->score()->firstMeasure(); m; m = m->nextMeasure()) {
            m->cmdRemoveStaves(idx, idx+1);
            if (m->hasMMRest())
                  m->mmRest()->cmdRemoveStaves(idx, idx+1);
            }

      undo(new RemoveStaff(staff));
      }

//---------------------------------------------------------
//   undoInsertStaff
//    idx - index of staff in part
//---------------------------------------------------------

void Score::undoInsertStaff(Staff* staff, int ridx, bool createRests)
      {
      undo(new InsertStaff(staff, ridx));
      int idx = staffIdx(staff->part()) + ridx;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            m->cmdAddStaves(idx, idx+1, createRests);
            if (m->hasMMRest())
                  m->mmRest()->cmdAddStaves(idx, idx+1, false);
            }
      // when newly adding an instrument,
      // this was already set when we created the staff
      // we don't have any better info at this point
      // and it dooesn't work to adjust bracket & barlines until all staves are added
      // TODO: adjust brackets only when appropriate
      //adjustBracketsIns(idx, idx+1);
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
      undoChangeProperty(e, P_ID::VISIBLE, v);
      e->setGenerated(false);
      }

//---------------------------------------------------------
//   undoAddElement
//---------------------------------------------------------

void Score::undoAddElement(Element* element)
      {
      QList<Staff* > staffList;
      Staff* ostaff = element->staff();
      int strack = -1;
      if (ostaff) {
            if (ostaff->excerpt() && strack > -1)
                  strack = ostaff->excerpt()->tracks().key(strack, -1);
            else
                  strack = ostaff->idx() * VOICES + element->track() % VOICES;
            }

      Element::Type et = element->type();

      //
      // some elements are replicated for all parts regardless of
      // linking:
      //

      if ((et == Element::Type::REHEARSAL_MARK)
         || ((et == Element::Type::STAFF_TEXT) && static_cast<StaffText*>(element)->systemFlag())
         || (et == Element::Type::JUMP)
         || (et == Element::Type::MARKER)
         || (et == Element::Type::TEMPO_TEXT)
         || (et == Element::Type::VOLTA)
         ) {
            foreach(Score* s, scoreList())
                  staffList.append(s->staff(0));

            foreach (Staff* staff, staffList) {
                  Score* score  = staff->score();
                  int staffIdx  = staff->idx();
                  int ntrack    = staffIdx * VOICES;
                  Element* ne;

                  if (staff->score() == ostaff->score())
                        ne = element;
                  else {
                        // only create linked volta for first staff
                        if (et == Element::Type::VOLTA && element->track() != 0)
                              continue;
                        ne = element->linkedClone();
                        ne->setScore(score);
                        ne->setSelected(false);
                        ne->setTrack(staffIdx * VOICES + element->voice());
                        }

                  if (et == Element::Type::VOLTA) {
                        Spanner* nsp = static_cast<Spanner*>(ne);
                        Spanner* sp = static_cast<Spanner*>(element);
                        int staffIdx1 = sp->track() / VOICES;
                        int staffIdx2 = sp->track2() / VOICES;
                        int diff = staffIdx2 - staffIdx1;
                        nsp->setTrack2((staffIdx + diff) * VOICES + (sp->track2() % VOICES));
                        undo(new AddElement(nsp));
                        }
                  else if (et == Element::Type::MARKER || et == Element::Type::JUMP) {
                        Measure* om = static_cast<Measure*>(element->parent());
                        Measure* m  = score->tick2measure(om->tick());
                        ne->setTrack(element->track());
                        ne->setParent(m);
                        undo(new AddElement(ne));
                        }
                  else {
                        Segment* segment  = static_cast<Segment*>(element->parent());
                        int tick          = segment->tick();
                        Measure* m        = score->tick2measure(tick);
                        Segment* seg      = m->undoGetSegment(Segment::Type::ChordRest, tick);
                        ne->setTrack(ntrack);
                        ne->setParent(seg);
                        undo(new AddElement(ne));
                        }
                  }
            return;
            }

      if (et == Element::Type::FINGERING
         || (et == Element::Type::IMAGE  && element->parent()->type() != Element::Type::SEGMENT)
         || (et == Element::Type::SYMBOL && element->parent()->type() != Element::Type::SEGMENT)
         || et == Element::Type::NOTE
         || et == Element::Type::TEXT
         || et == Element::Type::GLISSANDO
         || et == Element::Type::BEND
         || (et == Element::Type::CHORD && static_cast<Chord*>(element)->isGrace())
            ) {
            Element* parent       = element->parent();
            const LinkedElements* links = parent->links();
            // don't link part name
            if (et == Element::Type::TEXT) {
                  Text* t = static_cast<Text*>(element);
                  if (t->textStyleType() == TextStyleType::INSTRUMENT_EXCERPT)
                        links = 0;
                  }
            if (links == 0) {
                  undo(new AddElement(element));
                  if (element->type() == Element::Type::FINGERING && !element->isNudged())
                        element->score()->layoutFingering(static_cast<Fingering*>(element));
                  else if (element->type() == Element::Type::CHORD) {
#ifndef QT_NO_DEBUG
                        for (Note* n : static_cast<Chord*>(element)->notes()) {
                        //      if(n->tpc() == Tpc::TPC_INVALID)
                        //            n->setTpcFromPitch();
                              Q_ASSERT(n->tpc() != Tpc::TPC_INVALID);
                              }
#endif
                        }
                  return;
                  }
            foreach (ScoreElement* ee, *links) {
                  Element* e = static_cast<Element*>(ee);
                  Element* ne;
                  if (e == parent)
                        ne = element;
                  else {
                        if (element->type() == Element::Type::GLISSANDO) {    // and other spanners with Anchor::NOTE
                              Note* newEnd = Spanner::endElementFromSpanner(static_cast<Glissando*>(element), e);
                              if (newEnd) {
                                    ne = element->linkedClone();
                                    static_cast<Spanner*>(ne)->setNoteSpan(static_cast<Note*>(e), newEnd);
                                    }
                              else              //couldn't find suitable start note
                                    continue;
                              }
                        else
                              ne = element->linkedClone();
                        }
                  ne->setScore(e->score());
                  ne->setSelected(false);
                  ne->setParent(e);
                  undo(new AddElement(ne));
                  if (ne->type() == Element::Type::FINGERING)
                        e->score()->layoutFingering(static_cast<Fingering*>(ne));
                  else if (ne->type() == Element::Type::CHORD) {
#ifndef QT_NO_DEBUG
                        for (Note* n : static_cast<Chord*>(ne)->notes()) {
                              Q_ASSERT(n->tpc() != Tpc::TPC_INVALID);
                        //      n->setTpcFromPitch();
                              }
#endif
                        }
                  }
            return;
            }

      if (et == Element::Type::LAYOUT_BREAK) {
            LayoutBreak* lb = static_cast<LayoutBreak*>(element);
            if (lb->layoutBreakType() == LayoutBreak::Type::SECTION) {
                  Measure* m = lb->measure();
                  foreach(Score* s, scoreList()) {
                        if (s == lb->score())
                              undo(new AddElement(lb));
                        else {
                              Element* e = lb->linkedClone();
                              e->setScore(s);
                              Measure* nm = s->tick2measure(m->tick());
                              e->setParent(nm);
                              undo(new AddElement(e));
                              }
                        }
                  return;
                  }
            }

      if (ostaff == 0 || (
         et    != Element::Type::ARTICULATION
         && et != Element::Type::CHORDLINE
         && et != Element::Type::LYRICS
         && et != Element::Type::SLUR
         && et != Element::Type::TIE
         && et != Element::Type::NOTE
         && et != Element::Type::INSTRUMENT_CHANGE
         && et != Element::Type::HAIRPIN
         && et != Element::Type::OTTAVA
         && et != Element::Type::TRILL
         && et != Element::Type::TEXTLINE
         && et != Element::Type::PEDAL
         && et != Element::Type::BREATH
         && et != Element::Type::DYNAMIC
         && et != Element::Type::STAFF_TEXT
         && et != Element::Type::TREMOLO
         && et != Element::Type::ARPEGGIO
         && et != Element::Type::SYMBOL
         && et != Element::Type::TREMOLOBAR
         && et != Element::Type::FRET_DIAGRAM
         && et != Element::Type::HARMONY)
            ) {
            undo(new AddElement(element));
            return;
            }

      foreach (Staff* staff, ostaff->staffList()) {
            Score* score = staff->score();
            int staffIdx = staff->idx();

            QList<int> tr;
            if (staff->excerpt() && strack > -1)
                  tr = staff->excerpt()->tracks().values(strack);
            else
                  tr.append(strack);

            // Some elements in voice 1 of a staff should be copied to every track which has a linked voice in this staff
            if (tr.isEmpty() && (element->type() == Element::Type::SYMBOL
                || element->type() == Element::Type::IMAGE
                || element->type() == Element::Type::TREMOLOBAR
                || element->type() == Element::Type::DYNAMIC
                || element->type() == Element::Type::STAFF_TEXT
                || element->type() == Element::Type::FRET_DIAGRAM
                || element->type() == Element::Type::HARMONY
                || element->type() == Element::Type::HAIRPIN
                || element->type() == Element::Type::HAIRPIN
                || element->type() == Element::Type::OTTAVA
                || element->type() == Element::Type::TRILL
                || element->type() == Element::Type::TEXTLINE
                || element->type() == Element::Type::PEDAL)) {
                  tr.append(staffIdx * VOICES);
                  }

            int it = 0;
            for (int ntrack : tr) {
                  if ((ntrack & ~3) != staffIdx * VOICES) {
                        it++;
                        continue;
                        }

                  Element* ne;
                  if (staff == ostaff)
                        ne = element;
                  else {
                        if (staff->rstaff() != ostaff->rstaff()) {
                              switch (element->type()) {
                                    // exclude certain element types except on corresponding staff in part
                                    // this should be same list excluded in cloneStaff()
                                    case Element::Type::STAFF_TEXT:
                                    case Element::Type::FRET_DIAGRAM:
                                    case Element::Type::HARMONY:
                                    case Element::Type::FIGURED_BASS:
                                    case Element::Type::DYNAMIC:
                                    case Element::Type::LYRICS:   // not normally segment-attached
                                          continue;
                                    default:
                                          break;
                                    }
                              }
                        ne = element->linkedClone();
                        ne->setScore(score);
                        ne->setSelected(false);
                        ne->setTrack(staffIdx * VOICES + element->voice());
                        }

                  if (element->type() == Element::Type::ARTICULATION) {
                        Articulation* a  = static_cast<Articulation*>(element);
                        Segment* segment;
                        Segment::Type st;
                        Measure* m;
                        int tick;
                        if (a->parent()->isChordRest()) {
                              ChordRest* cr = a->chordRest();
                              segment       = cr->segment();
                              st            = Segment::Type::ChordRest;
                              tick          = segment->tick();
                              m             = score->tick2measure(tick);
                              }
                        else {
                              segment  = static_cast<Segment*>(a->parent()->parent());
                              st       = Segment::Type::EndBarLine;
                              tick     = segment->tick();
                              m        = score->tick2measure(tick);
                              if (m->tick() == tick)
                                    m = m->prevMeasure();
                              }
                        Segment* seg = m->findSegment(st, tick);
                        if (seg == 0) {
                              qWarning("undoAddSegment: segment not found");
                              break;
                              }
                        Articulation* na = static_cast<Articulation*>(ne);
                        na->setTrack(ntrack);
                        if (a->parent()->isChordRest()) {
                              ChordRest* cr = a->chordRest();
                              ChordRest* ncr;
                              if (cr->isGrace())
                                    ncr = findLinkedChord(static_cast<Chord*>(cr), score->staff(staffIdx));
                              else
                                    ncr = static_cast<ChordRest*>(seg->element(ntrack));
                              na->setParent(ncr);
                              }
                        else {
                              BarLine* bl = static_cast<BarLine*>(seg->element(ntrack));
                              na->setParent(bl);
                              }
                        undo(new AddElement(na));
                        }
                  else if (element->type() == Element::Type::CHORDLINE
                     || element->type() == Element::Type::LYRICS) {
                        ChordRest* cr    = static_cast<ChordRest*>(element->parent());
                        Segment* segment = cr->segment();
                        int tick         = segment->tick();
                        Measure* m       = score->tick2measure(tick);
                        Segment* seg     = m->findSegment(Segment::Type::ChordRest, tick);
                        if (seg == 0) {
                              qWarning("undoAddSegment: segment not found");
                              break;
                              }
                        ne->setTrack(ntrack);
                        ChordRest* ncr = static_cast<ChordRest*>(seg->element(ntrack));
                        ne->setParent(ncr);
                        undo(new AddElement(ne));
                        }
                  //
                  // elements with Segment as parent
                  //
                  else if (element->type() == Element::Type::SYMBOL
                     || element->type() == Element::Type::IMAGE
                     || element->type() == Element::Type::TREMOLOBAR
                     || element->type() == Element::Type::DYNAMIC
                     || element->type() == Element::Type::STAFF_TEXT
                     || element->type() == Element::Type::FRET_DIAGRAM
                     || element->type() == Element::Type::HARMONY) {
                        Segment* segment = static_cast<Segment*>(element->parent());
                        int tick         = segment->tick();
                        Measure* m       = score->tick2measure(tick);
                        Segment* seg     = m->undoGetSegment(Segment::Type::ChordRest, tick);
                        ne->setTrack(ntrack);
                        ne->setParent(seg);
                        undo(new AddElement(ne));
                        // transpose harmony if necessary
                        if (element->type() == Element::Type::HARMONY && ne != element) {
                              Harmony* h = static_cast<Harmony*>(ne);
                              if (score->styleB(StyleIdx::concertPitch) != element->score()->styleB(StyleIdx::concertPitch)) {
                                    Part* partDest = h->part();
                                    Interval interval = partDest->instrument(tick)->transpose();
                                    if (!interval.isZero()) {
                                          if (!score->styleB(StyleIdx::concertPitch))
                                                interval.flip();
                                          int rootTpc = transposeTpc(h->rootTpc(), interval, true);
                                          int baseTpc = transposeTpc(h->baseTpc(), interval, true);
                                          score->undoTransposeHarmony(h, rootTpc, baseTpc);
                                          }
                                    }
                              }
                        }
                  else if (element->type() == Element::Type::SLUR
                     || element->type() == Element::Type::HAIRPIN
                     || element->type() == Element::Type::OTTAVA
                     || element->type() == Element::Type::TRILL
                     || element->type() == Element::Type::TEXTLINE
                     || element->type() == Element::Type::PEDAL) {
                        Spanner* sp   = static_cast<Spanner*>(element);
                        Spanner* nsp  = static_cast<Spanner*>(ne);
                        int staffIdx1 = sp->track() / VOICES;
                        int staffIdx2 = sp->track2() / VOICES;
                        int diff      = staffIdx2 - staffIdx1;
                        nsp->setTrack2((staffIdx + diff) * VOICES + (sp->track2() % VOICES));
                        nsp->setTrack(ntrack);

                        QList<int> tl2;
                        if (staff->excerpt() && element->isSlur()) {
                              nsp->setTrack(ntrack);
                                    tl2 = staff->excerpt()->tracks().values(sp->track2());
                                    if (tl2.isEmpty()) {
                                          it++;
                                          continue;
                                          }
                                   nsp->setTrack2(tl2.at(it));
                              }
                        else if (!element->isSlur())
                              nsp->setTrack(ntrack & ~3);

                        // determine start/end element for slurs
                        // this is only necessary if start/end element is
                        //   a grace note, otherwise the element can be set to zero
                        //   and will later be calculated from tick/track values
                        //
                        if (element->type() == Element::Type::SLUR && sp != nsp) {
                              if (sp->startElement()) {
                                    QList<ScoreElement*> sel = sp->startElement()->linkList();
                                    for (ScoreElement* ee : sel) {
                                          Element* e = static_cast<Element*>(ee);
                                          if (e->score() == nsp->score() && e->track() == nsp->track()) {
                                                nsp->setStartElement(e);
                                                break;
                                                }
                                          }
                                    }
                              if (sp->endElement()) {
                                    QList<ScoreElement*> eel = sp->endElement()->linkList();
                                    for (ScoreElement* ee : eel) {
                                          Element* e = static_cast<Element*>(ee);
                                          if (e->score() == nsp->score() && e->track() == nsp->track2()) {
                                                nsp->setEndElement(e);
                                                break;
                                                }
                                          }
                                    }
                              }
                        undo(new AddElement(nsp));
                        }
                  else if (et == Element::Type::GLISSANDO)
                        undo(new AddElement(static_cast<Spanner*>(ne)));
                  else if (element->type() == Element::Type::TREMOLO && static_cast<Tremolo*>(element)->twoNotes()) {
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
                  else if (element->type() == Element::Type::TREMOLO && !static_cast<Tremolo*>(element)->twoNotes()) {
                        Chord* cr = static_cast<Chord*>(element->parent());
                        Chord* c1 = findLinkedChord(cr, score->staff(staffIdx));
                        ne->setParent(c1);
                        undo(new AddElement(ne));
                        }
                  else if (element->type() == Element::Type::ARPEGGIO) {
                        ChordRest* cr = static_cast<ChordRest*>(element->parent());
                        Segment* s    = cr->segment();
                        Measure* m    = s->measure();
                        Measure* nm   = score->tick2measure(m->tick());
                        Segment* ns   = nm->findSegment(s->segmentType(), s->tick());
                        Chord* c1     = static_cast<Chord*>(ns->element(staffIdx * VOICES + cr->voice()));
                        ne->setParent(c1);
                        undo(new AddElement(ne));
                        }
                  else if (element->type() == Element::Type::TIE) {
                        Tie* tie       = static_cast<Tie*>(element);
                        Note* n1       = tie->startNote();
                        Note* n2       = tie->endNote();
                        Chord* cr1     = n1->chord();
                        Chord* cr2     = n2 ? n2->chord() : 0;

                        // find corresponding notes in linked staff
                        // accounting for grace notes and cross-staff notation
                        int sm = 0;
                        if (cr1->staffIdx() != cr2->staffIdx())
                              sm = cr2->staffIdx() - cr1->staffIdx();
                        Chord* c1 = findLinkedChord(cr1, score->staff(staffIdx));
                        Chord* c2 = findLinkedChord(cr2, score->staff(staffIdx + sm));
                        Note* nn1 = c1->findNote(n1->pitch());
                        Note* nn2 = c2 ? c2->findNote(n2->pitch()) : 0;

                        // create tie
                        Tie* ntie = static_cast<Tie*>(ne);
                        QList<SpannerSegment*>& segments = ntie->spannerSegments();
                        foreach (SpannerSegment* segment, segments)
                              delete segment;
                        segments.clear();
                        ntie->setTrack(c1->track());
                        ntie->setStartNote(nn1);
                        ntie->setEndNote(nn2);
                        undo(new AddElement(ntie));
                        }
                  else if (element->type() == Element::Type::INSTRUMENT_CHANGE) {
                        InstrumentChange* is = static_cast<InstrumentChange*>(element);
                        Segment* s1    = is->segment();
                        Measure* m1    = s1->measure();
                        Measure* nm1   = score->tick2measure(m1->tick());
                        Segment* ns1   = nm1->findSegment(s1->segmentType(), s1->tick());
                        InstrumentChange* nis = static_cast<InstrumentChange*>(ne);
                        nis->setParent(ns1);
                        // ws: instrument should not be changed here
                        if (is->instrument()->channel().empty() || is->instrument()->channel(0)->program == -1)
                              nis->setInstrument(*staff->part()->instrument(s1->tick()));
                        else if (nis != is)
                              nis->setInstrument(*is->instrument());
                        undo(new AddElement(nis));
                        }
                  else if (element->type() == Element::Type::BREATH) {
                        Breath* breath   = static_cast<Breath*>(element);
                        int tick         = breath->segment()->tick();
                        Measure* m       = score->tick2measure(tick);
                        // breath appears before barline
                        if (m->tick() == tick)
                              m = m->prevMeasure();
                        Segment* seg     = m->undoGetSegment(Segment::Type::Breath, tick);
                        Breath* nbreath  = static_cast<Breath*>(ne);
                        nbreath->setScore(score);
                        nbreath->setTrack(ntrack);
                        nbreath->setParent(seg);
                        undo(new AddElement(nbreath));
                        }
                  else
                        qWarning("undoAddElement: unhandled: <%s>", element->name());
                  it++;
                  }
            }
      }

//---------------------------------------------------------
//   undoAddCR
//---------------------------------------------------------

void Score::undoAddCR(ChordRest* cr, Measure* measure, int tick)
      {
      Q_ASSERT(cr->type() != Element::Type::CHORD || !(static_cast<Chord*>(cr)->notes()).empty());
      Q_ASSERT(cr->isChordRest());

      Staff* ostaff = cr->staff();
      int strack = ostaff->idx() * VOICES + cr->voice();

      if (ostaff->excerpt() && !ostaff->excerpt()->tracks().isEmpty())
            strack = ostaff->excerpt()->tracks().key(strack, -1);

      Segment::Type segmentType = Segment::Type::ChordRest;

      Tuplet* t = cr->tuplet();

      foreach (Staff* staff, ostaff->staffList()) {
            QList<int> tracks;
            if (staff->excerpt() && !staff->excerpt()->tracks().isEmpty())
                  tracks = staff->excerpt()->tracks().values(strack);
            else
                  tracks.append(staff->idx() * VOICES + cr->voice());

            for (int ntrack : tracks) {
                  if (ntrack < staff->part()->startTrack() || ntrack >= staff->part()->endTrack())
                        continue;

                  Score* score = staff->score();
                  Measure* m   = (score == this) ? measure : score->tick2measure(tick);
                  if (!m)  {
                        qDebug("measure not found");
                        break;
                        }
                  Segment* seg = m->undoGetSegment(segmentType, tick);

                  Q_ASSERT(seg->segmentType() == segmentType);

                  ChordRest* newcr = (staff == ostaff) ? cr : toChordRest(cr->linkedClone());
                  newcr->setScore(score);

                  newcr->setTrack(ntrack);
                  newcr->setParent(seg);

#ifndef QT_NO_DEBUG
                  if (newcr->type() == Element::Type::CHORD) {
                        Chord* chord = static_cast<Chord*>(newcr);
                        // setTpcFromPitch needs to know the note tick position
                        foreach(Note* note, chord->notes()) {
                              // if (note->tpc() == Tpc::TPC_INVALID)
                              //      note->setTpcFromPitch();
                              Q_ASSERT(note->tpc() != Tpc::TPC_INVALID);
                              }
                        }
#endif
                  if (t) {
                        if (staff != ostaff) {
                              Tuplet* nt = 0;
                              if (t->elements().empty() || t->elements().front() == cr) {
                                    for (ScoreElement* e : t->linkList()) {
                                          Tuplet* nt1 = static_cast<Tuplet*>(e);
                                          if (nt1 == t)
                                                continue;
                                          if (nt1->score() == score && nt1->track() == newcr->track()) {
                                                nt = nt1;
                                                break;
                                                }
                                          }
                                    if (!nt) {
                                          nt = static_cast<Tuplet*>(t->linkedClone());
                                          nt->setTuplet(0);
                                          nt->setScore(score);
                                          nt->setTrack(newcr->track());
                                          }

                                    Tuplet* t2  = t;
                                    Tuplet* nt2 = nt;
                                    while (t2->tuplet()) {
                                          Tuplet* t = t2->tuplet();
                                          Tuplet* nt3 = 0;

                                          for (auto i : t->linkList()) {
                                                Tuplet* tt = static_cast<Tuplet*>(i);
                                                if (tt != t && tt->score() == score && tt->track() == t2->track()) {
                                                      nt3 = tt;
                                                      break;
                                                      }
                                                }
                                          if (nt3 == 0) {
                                                nt3 = static_cast<Tuplet*>(t->linkedClone());
                                                nt3->setScore(score);
                                                nt3->setTrack(nt2->track());
                                                }
                                          nt3->add(nt2);
                                          nt2->setTuplet(nt3);

                                          t2 = t;
                                          nt2 = nt3;
                                          }

                                    }
                              else {
                                    const LinkedElements* le = t->links();
                                    // search the linked tuplet
                                    if (le) {
                                          for (ScoreElement* ee : *le) {
                                                Element* e = static_cast<Element*>(ee);
                                                if (e->score() == score && e->track() == ntrack) {
                                                      nt = static_cast<Tuplet*>(e);
                                                      break;
                                                      }
                                                }
                                          }
                                    if (nt == 0)
                                          qWarning("linked tuplet not found");
                                    }
                              newcr->setTuplet(nt);
                              }
                        }

                  if (newcr->isRest() && (toRest(newcr)->isGap()) && !(toRest(newcr)->track() % VOICES))
                        toRest(newcr)->setGap(false);

                  undo(new AddElement(newcr));
                  }
            }
      }

//---------------------------------------------------------
//   undoRemoveElement
//---------------------------------------------------------

void Score::undoRemoveElement(Element* element)
      {
      QList<Segment*> segments;
      for (ScoreElement* ee : element->linkList()) {
            Element* e = static_cast<Element*>(ee);
            undo(new RemoveElement(e));
            if (e->parent() && (e->parent()->type() == Element::Type::SEGMENT)) {
                  Segment* s = static_cast<Segment*>(e->parent());
                  if (!segments.contains(s))
                        segments.append(s);
                  }
            }
      for (Segment* s : segments) {
            if (s->empty())
                  undo(new RemoveElement(s));
            }
      }

//---------------------------------------------------------
//   undoChangeTuning
//---------------------------------------------------------

void Score::undoChangeTuning(Note* n, qreal v)
      {
      undoChangeProperty(n, P_ID::TUNING, v);
      }

void Score::undoChangeUserMirror(Note* n, MScore::DirectionH d)
      {
      undoChangeProperty(n, P_ID::MIRROR_HEAD, int(d));
      }

//---------------------------------------------------------
//   undoChangePageFormat
//---------------------------------------------------------

void Score::undoChangePageFormat(PageFormat* p)
      {
      undoChangePageFormat(p, spatium(), pageNumberOffset());
      }

//---------------------------------------------------------
//   undoChangePageFormat
//---------------------------------------------------------

void Score::undoChangePageFormat(PageFormat* p, qreal v, int pageOffset)
      {
      undo(new ChangePageFormat(this, p, v, pageOffset));
      }

//---------------------------------------------------------
//   undoChangeTpc
//    TODO-TPC: check
//---------------------------------------------------------

void Score::undoChangeTpc(Note* note, int v)
      {
      note->undoChangeProperty(P_ID::TPC1, v);
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
      if (element->isChordRest1()) {
            if (isUndo)
                  undoRemoveTuplet(static_cast<ChordRest*>(element));
            else
                  undoAddTuplet(static_cast<ChordRest*>(element));
            }
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void AddElement::undo()
      {
      if (element->type() != Element::Type::TUPLET)
            element->score()->removeElement(element);
      endUndoRedo(true);
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void AddElement::redo()
      {
      if (element->type() != Element::Type::TUPLET)
            element->score()->addElement(element);
      endUndoRedo(false);
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* AddElement::name() const
      {
      static char buffer[64];
      if (element->isText())
            snprintf(buffer, 64, "Add: %s <%s> %p", element->name(),
               qPrintable(static_cast<Text*>(element)->plainText()), element);
      else
            snprintf(buffer, 64, "Add: <%s> %p", element->name(), element);
      return buffer;
      }

//---------------------------------------------------------
//   RemoveElement
//---------------------------------------------------------

RemoveElement::RemoveElement(Element* e)
      {
      element = e;

      Score* score = element->score();
      if (element->isChordRest1()) {
            ChordRest* cr = static_cast<ChordRest*>(element);
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
//   AddElement::cleanup
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

void RemoveElement::undo()
      {
      if (element->type() != Element::Type::TUPLET)
            element->score()->addElement(element);
      if (element->isChordRest1()) {
            if (element->isChord()) {
                  Chord* chord = toChord(element);
                  foreach(Note* note, chord->notes()) {
                        if (note->tieBack())
                              note->tieBack()->setEndNote(note);
                        if (note->tieFor() && note->tieFor()->endNote())
                              note->tieFor()->endNote()->setTieBack(note->tieFor());
                        }
                  }
            undoAddTuplet(static_cast<ChordRest*>(element));
            }
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void RemoveElement::redo()
      {
      if (element->type() != Element::Type::TUPLET)
            element->score()->removeElement(element);
      if (element->isChordRest1()) {
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
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* RemoveElement::name() const
      {
      static char buffer[64];
      if (element->isText())
            snprintf(buffer, 64, "Remove: %s <%s> %p", element->name(),
               qPrintable(static_cast<Text*>(element)->plainText()), element);
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

InsertStaff::InsertStaff(Staff* p, int _ridx)
      {
      staff = p;
      ridx  = _ridx;
      }

void InsertStaff::undo()
      {
      staff->score()->removeStaff(staff);
      }

void InsertStaff::redo()
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

void RemoveStaff::undo()
      {
      staff->score()->insertStaff(staff, ridx);
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

ChangePitch::ChangePitch(Note* _note, int _pitch, int _tpc1, int _tpc2)
      {
      note  = _note;
      pitch = _pitch;
      tpc1  = _tpc1;
      tpc2  = _tpc2;
      }

void ChangePitch::flip()
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

void ChangeFretting::flip()
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

void ChangeElement::flip()
      {
      const LinkedElements* links = oldElement->links();
      if (links) {
            oldElement->unlink();
            oldElement->linkTo(newElement);
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

      if (newElement->type() == Element::Type::KEYSIG) {
            KeySig* ks = static_cast<KeySig*>(newElement);
            if (!ks->generated())
                  ks->staff()->setKey(ks->tick(), ks->keySigEvent());
            }
      else if (newElement->type() == Element::Type::DYNAMIC)
            newElement->score()->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
      else if (newElement->type() == Element::Type::TEMPO_TEXT) {
            TempoText* t = static_cast<TempoText*>(oldElement);
            score->setTempo(t->segment(), t->tempo());
            }
      if (newElement->isSegmentFlag()) {
            SpannerSegment* os = static_cast<SpannerSegment*>(oldElement);
            SpannerSegment* ns = static_cast<SpannerSegment*>(newElement);
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
//   ChangeKeySig::flip
//---------------------------------------------------------

void ChangeKeySig::flip()
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

void ChangeMeasureLen::flip()
      {
      Fraction oLen = measure->len();

      //
      // move EndBarLine and TimeSigAnnounce
      // to end of measure:
      //
      int endTick = measure->tick() + len.ticks();
      for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            if (segment->segmentType() != Segment::Type::EndBarLine
               && segment->segmentType() != Segment::Type::TimeSigAnnounce)
                  continue;
            segment->setTick(endTick);
            }
      measure->setLen(len);
      measure->score()->fixTicks();
      len = oLen;
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
      staff->score()->setLayoutAll();
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
      int nspan         = barLine->span();
      int nspanFrom     = barLine->spanFrom();
      int nspanTo       = barLine->spanTo();

      barLine->setSpan(span);
      barLine->setSpanFrom(spanFrom);
      barLine->setSpanTo(spanTo);

      span              = nspan;
      spanFrom          = nspanFrom;
      spanTo            = nspanTo;
      barLine->score()->setLayout(barLine->tick());
      barLine->setCustomSpan(true);
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

void ExchangeVoice::undo()
      {
      measure->exchangeVoice(val2, val1, staff);
      measure->checkMultiVoices(staff);
      }

void ExchangeVoice::redo()
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

void ChangeInstrumentShort::flip()
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

void ChangeInstrumentLong::flip()
      {
      QList<StaffName> s = part->longNames(tick);
      part->setLongNames(text, tick);
      text = s;
      part->score()->setLayoutAll();
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
      staff->score()->setLayoutAll();
      }

//---------------------------------------------------------
//   EditText::undo
//---------------------------------------------------------

void EditText::undo()
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

void EditText::redo()
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
      score->setLayoutAll();

      pf->copy(f);
      spatium = os;
      pageOffset = po;
      }

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

ChangeStaff::ChangeStaff(Staff* _staff,  bool _invisible,
   qreal _userDist, Staff::HideMode _hideMode, bool _showIfEmpty, bool _cutaway, bool hide)
      {
      staff       = _staff;
      invisible   = _invisible;
      userDist    = _userDist;
      hideMode    = _hideMode;
      showIfEmpty = _showIfEmpty;
      cutaway     = _cutaway;
      hideSystemBarLine = hide;
      }

//---------------------------------------------------------
//   notifyTimeSigs
//    mark timesigs for layout
//---------------------------------------------------------

static void notifyTimeSigs(void*, Element* e)
      {
      if (e->type() == Element::Type::TIMESIG)
            static_cast<TimeSig*>(e)->setNeedLayout(true);
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStaff::flip()
      {
      bool invisibleChanged = staff->invisible() != invisible;

      bool oldInvisible   = staff->invisible();
      qreal oldUserDist   = staff->userDist();
      Staff::HideMode oldHideMode    = staff->hideWhenEmpty();
      bool oldShowIfEmpty = staff->showIfEmpty();
      bool oldCutaway     = staff->cutaway();
      bool hide           = staff->hideSystemBarLine();

      staff->setInvisible(invisible);
      staff->setUserDist(userDist);
      staff->setHideWhenEmpty(hideMode);
      staff->setShowIfEmpty(showIfEmpty);
      staff->setCutaway(cutaway);
      staff->setHideSystemBarLine(hideSystemBarLine);

      invisible   = oldInvisible;
      userDist    = oldUserDist;
      hideMode    = oldHideMode;
      showIfEmpty = oldShowIfEmpty;
      cutaway     = oldCutaway;
      hideSystemBarLine = hide;

      Score* score = staff->score();
      if (invisibleChanged) {
            int staffIdx = staff->idx();
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  MStaff* mstaff = m->mstaff(staffIdx);
                  mstaff->lines->setVisible(!staff->invisible());
                  }
            }
      staff->score()->setLayoutAll();
      staff->masterScore()->rebuildMidiMapping();
      staff->score()->setPlaylistDirty();

      score->scanElements(0, notifyTimeSigs);
      }

//---------------------------------------------------------
//   ChangeStaffType::flip
//---------------------------------------------------------

void ChangeStaffType::flip()
      {
      StaffType st = *staff->staffType();

      staff->setStaffType(&staffType);

      staffType = st;

      Score* score = staff->score();
      score->setLayoutAll();
      score->scanElements(0, notifyTimeSigs);
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

void ChangePart::flip()
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
            if (text->textStyle().name() == s) {
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
      score->setLayoutAll();
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
//   ChangeStyle
//---------------------------------------------------------

ChangeStyle::ChangeStyle(Score* s, const MStyle& st)
   : score(s), style(st)
      {
      }

static void updateTimeSigs(void*, Element* e)
      {
      if (e->type() == Element::Type::TIMESIG) {
            TimeSig* ts = static_cast<TimeSig*>(e);
            ts->setNeedLayout(true);
            }
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStyle::flip()
      {
      MStyle tmp = *score->style();

      if (score->style(StyleIdx::concertPitch) != style.value(StyleIdx::concertPitch))
            score->cmdConcertPitchChanged(style.value(StyleIdx::concertPitch).toBool(), true);
      if (score->style(StyleIdx::MusicalSymbolFont) != style.value(StyleIdx::MusicalSymbolFont)) {
            score->setScoreFont(ScoreFont::fontFactory(style.value(StyleIdx::MusicalSymbolFont).toString()));
            score->scanElements(0, updateTimeSigs);
            }
      score->setStyle(style);
      score->styleChanged();
      style = tmp;
      }

//---------------------------------------------------------
//   ChangeStyleVal::flip
//---------------------------------------------------------

void ChangeStyleVal::flip()
      {
      QVariant v = score->style(idx);
      if (v != value) {
            score->style()->set(idx, value);
            if (idx == StyleIdx::chordDescriptionFile) {
                  score->style()->chordList()->unload();
                  score->style()->chordList()->read(value.toString());
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

void ChangeChordStaffMove::flip()
      {
      int v = chordRest->staffMove();
      for (ScoreElement* e : chordRest->linkList()) {
            ChordRest* cr = static_cast<ChordRest*>(e);
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

void ChangeVelocity::flip()
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
//   undoInsertTime
//   acts on the linked scores as well
//---------------------------------------------------------

void Score::undoInsertTime(int tick, int len)
      {
      if (len == 0)
            return;

      QList<Spanner*> sl;
      for (auto i : _spanner.map()) {
            Spanner* s = i.second;
            if (s->tick2() < tick)
                  continue;
            bool append = false;
            if (len > 0) {
                  if (tick > s->tick() && tick < s->tick2())
                        append = true;
                  else if (tick <= s->tick())
                        append = true;
                  }
            else {
                  int tick2 = tick - len;
                  if (s->tick() >= tick2)
                        append = true;
                  else if ((s->tick() < tick) && (s->tick2() > tick2)) {
                        int t2 = s->tick2() + len;
                        if (t2 > s->tick())
                              append = true;
                        }
                  else if (s->tick() >= tick && s->tick2() <= tick2)
                        append = true;
                  else if (s->tick() > tick && s->tick2() > tick2)
                        append = true;
                  }
            for (Spanner* ss : sl) {
                  if (ss->linkList().contains(s)) {
                        append = false;
                        break;
                        }
                  }
            if (append)
                  sl.append(s);
            }
      for (Spanner* s : sl) {
            if (len > 0) {
                  if (tick > s->tick() && tick < s->tick2()) {
                        //
                        //  case a:
                        //  +----spanner--------+
                        //    +---add---
                        //
                        undoChangeProperty(s, P_ID::SPANNER_TICKS, s->ticks() + len);
                        }
                  else if (tick <= s->tick()) {
                        //
                        //  case b:
                        //       +----spanner--------
                        //  +---add---
                        // and
                        //            +----spanner--------
                        //  +---add---+
                        undoChangeProperty(s, P_ID::SPANNER_TICK, s->tick() + len);
                        }
                  }
            else {
                  int tick2 = tick - len;
                  if (s->tick() >= tick2) {
                        //
                        //  case A:
                        //  +----remove---+ +---spanner---+
                        //
                        int t = s->tick() + len;
                        if (t < 0)
                              t = 0;
                        undoChangeProperty(s, P_ID::SPANNER_TICK, t);
                        }
                  else if ((s->tick() < tick) && (s->tick2() > tick2)) {
                        //
                        //  case B:
                        //  +----spanner--------+
                        //    +---remove---+
                        //
                        int t2 = s->tick2() + len;
                        if (t2 > s->tick()) {
                              undoChangeProperty(s, P_ID::SPANNER_TICKS, s->ticks() + len);
                              }
                        }
//                  else if (s->tick() >= tick && s->tick2() < tick2) {
                  else if (s->tick() >= tick && s->tick2() <= tick2) {
                        //
                        //  case C:
                        //    +---spanner---+
                        //  +----remove--------+
                        //
                        undoRemoveElement(s);
                        }
                  else if (s->tick() > tick && s->tick2() > tick2) {
                        //
                        //  case D:
                        //       +----spanner--------+
                        //  +---remove---+
                        //
                        int d1 = s->tick() - tick;
                        int d2 = tick2 - s->tick();
                        int len = s->ticks() - d2;
                        if (len == 0) {
                              undoRemoveElement(s);
                              }
                        else {
                              undoChangeProperty(s, P_ID::SPANNER_TICK, s->tick() - d1);
                              undoChangeProperty(s, P_ID::SPANNER_TICKS, len);
                              }
                        }
                  }
            }

      for (auto i = _unmanagedSpanner.begin(); i != _unmanagedSpanner.end();) {
            auto ni = i;
            ++ni;
            (*i)->undoInsertTimeUnmanaged(tick, len); // may remove spanner from list
            i = ni;
            }
      }

//---------------------------------------------------------
//   undoRemoveMeasures
//---------------------------------------------------------

void Score::undoRemoveMeasures(Measure* m1, Measure* m2)
      {
      //
      //  handle ties which start before m1 and end in (m1-m2)
      //
      for (Segment* s = m1->first(); s != m2->last(); s = s->next1()) {
            if (s->segmentType() != Segment::Type::ChordRest)
                  continue;
            for (int track = 0; track < ntracks(); ++track) {
                  Chord* c = static_cast<Chord*>(s->element(track));
                  if (c == 0 || c->type() != Element::Type::CHORD)
                        continue;
                  for (Note* n : c->notes()) {
                        Tie* t = n->tieBack();
                        if (t && (t->startNote()->chord()->tick() < m1->tick()))
                              undoRemoveElement(t);
                        t = n->tieFor();
                        if (t && (t->endNote()->chord()->tick() >= m2->endTick()))
                              undoRemoveElement(t);
                        }
                  }
            }
      undo(new RemoveMeasures(m1, m2));
      }

//---------------------------------------------------------
//   insertMeasures
//---------------------------------------------------------

void InsertRemoveMeasures::insertMeasures()
      {
      Score* score = fm->score();
      QList<Clef*> clefs;
      QList<KeySig*> keys;
      if (fm->type() == Element::Type::MEASURE) {
            score->setPlaylistDirty();
            for (Segment* s = static_cast<Measure*>(fm)->first(); s != static_cast<Measure*>(lm)->last(); s = s->next1()) {
                  if (!(s->segmentType() & (Segment::Type::Clef | Segment::Type::KeySig)))
                        continue;
                  for (int track = 0; track < score->ntracks(); track += VOICES) {
                        Element* e = s->element(track);
                        if (!e || e->generated())
                              continue;
                        if (e->type() == Element::Type::CLEF)
                              clefs.append(static_cast<Clef*>(e));
                        else if (e->type() == Element::Type::KEYSIG)
                              keys.append(static_cast<KeySig*>(e));
                        }
                  }
            }
      score->measures()->insert(fm, lm);

      if (fm->type() == Element::Type::MEASURE) {
            score->fixTicks();
            score->insertTime(fm->tick(), lm->endTick() - fm->tick());

            // move ownership of Instrument back to part
            for (Segment* s = static_cast<Measure*>(fm)->first(); s != static_cast<Measure*>(lm)->last(); s = s->next1()) {
                  for (Element* e : s->annotations()) {
                        if (e->type() == Element::Type::INSTRUMENT_CHANGE) {
                              e->part()->setInstrument(static_cast<InstrumentChange*>(e)->instrument(), s->tick());
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

      if (fm->type() != Element::Type::MEASURE || !fm->prevMeasure())
            return;
      Measure* m = fm->prevMeasure();
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            for (int track = 0; track < score->ntracks(); ++track) {
                  Chord* chord = static_cast<Chord*>(seg->element(track));
                  if (chord == 0 || chord->type() != Element::Type::CHORD)
                        continue;
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
      score->measures()->remove(fm, lm);
      score->fixTicks();
      if (fm->isMeasure()) {
            score->setPlaylistDirty();

            // check if there is a clef at the end of last measure
            // remove clef from staff cleflist

            if (lm->isMeasure()) {
                  Measure* m = toMeasure(lm);
                  Segment* s = m->findSegment(Segment::Type::Clef, tick2);
                  if (s) {
                        for (Element* e : s->elist()) {
                              Clef* clef = toClef(e);
                              if (clef)
                                    score->staff(clef->staffIdx())->removeClef(clef);
                              }
                        }
                  }

            score->insertTime(tick1, -(tick2 - tick1));
            score->setLayoutAll();
            for (Spanner* sp : score->unmanagedSpanners()) {
                  if ((sp->tick() >= tick1 && sp->tick() < tick2) || (sp->tick2() >= tick1 && sp->tick2() < tick2))
                        sp->removeUnmanaged();
                  }
            score->connectTies(true);   // ??
            }
      score->setLayoutAll();
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
//   AddExcerpt::undo
//---------------------------------------------------------

void AddExcerpt::undo()
      {
      excerpt->oscore()->removeExcerpt(excerpt);
      }

//---------------------------------------------------------
//   AddExcerpt::redo
//---------------------------------------------------------

void AddExcerpt::redo()
      {
      excerpt->oscore()->addExcerpt(excerpt);
      }

//---------------------------------------------------------
//   RemoveExcerpt::undo()
//---------------------------------------------------------

void RemoveExcerpt::undo()
      {
      excerpt->oscore()->addExcerpt(excerpt);
      }

//---------------------------------------------------------
//   RemoveExcerpt::redo()
//---------------------------------------------------------

void RemoveExcerpt::redo()
      {
      excerpt->oscore()->removeExcerpt(excerpt);
      }

//---------------------------------------------------------
//   SwapExcerpt::flip()
//---------------------------------------------------------

void SwapExcerpt::flip()
      {
      score->excerpts().swap(pos1, pos2);
      score->setExcerptsChanged(true);
      }

//---------------------------------------------------------
//   ChangeExcerptTitle::flip()
//---------------------------------------------------------

void ChangeExcerptTitle::flip()
      {
      QString s = title;
      title = excerpt->title();
      excerpt->setTitle(s);
      excerpt->oscore()->setExcerptsChanged(true);
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeBend::flip()
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
//   ChangeInstrument::flip
//---------------------------------------------------------

void ChangeInstrument::flip()
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

void SwapCR::flip()
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

void ChangeClefType::flip()
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

void MoveStaff::flip()
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

void ChangeStaffUserDist::flip()
      {
      qreal v = staff->userDist();
      staff->setUserDist(dist);
      dist = v;
      staff->score()->setLayoutAll();
      }

//---------------------------------------------------------
//   ChangeProperty::flip
//---------------------------------------------------------

void ChangeProperty::flip()
      {
      qCDebug(undoRedo) << "ChangeProperty::flip():" << element->name() << propertyName(id) << element->getProperty(id) << "->" << property;

      if (id == P_ID::SPANNER_TICK || id == P_ID::SPANNER_TICKS)
            static_cast<Element*>(element)->score()->removeSpanner(static_cast<Spanner*>(element));

      QVariant v       = element->getProperty(id);
      PropertyStyle ps = element->propertyStyle(id);
      if (propertyStyle == PropertyStyle::STYLED)
            element->resetProperty(id);
      else
            element->setProperty(id, property);

      if (id == P_ID::SPANNER_TICK || id == P_ID::SPANNER_TICKS) {
            static_cast<Element*>(element)->score()->addSpanner(static_cast<Spanner*>(element));
            // while updating ticks for an Ottava, the parent staff calls updateOttava()
            // and expects to find the Ottava spanner(s) in the score lists;
            // thus, the above (re)setProperty() left the staff pitchOffset map in a wrong state
            // as the spanner has been removed from the score lists; redo the map here
            if (static_cast<Element*>(element)->type() == Element::Type::OTTAVA)
                  static_cast<Element*>(element)->staff()->updateOttava();
            }
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

      PlayEventType eventListType = PlayEventType::User;

      void flip();

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

void ChangeEventList::flip()
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
      staff->score()->setLayoutAll();
      }

void AddBracket::undo()
      {
      staff->setBracket(level, BracketType::NO_BRACKET);
      staff->score()->setLayoutAll();
      }

void RemoveBracket::redo()
      {
      staff->setBracket(level, BracketType::NO_BRACKET);
      staff->score()->setLayoutAll();
      }

void RemoveBracket::undo()
      {
      staff->setBracket(level, type);
      staff->setBracketSpan(level, span);
      staff->score()->setLayoutAll();
      }

//---------------------------------------------------------
//   ChangeSpannerElements
//---------------------------------------------------------

void ChangeSpannerElements::flip()
      {
      Element*    oldStartElement   = spanner->startElement();
      Element*    oldEndElement     = spanner->endElement();
      if (spanner->anchor() == Spanner::Anchor::NOTE) {
            // be sure new spanner elements are of the right type
            if (startElement->type() != Element::Type::NOTE || endElement->type() != Element::Type::NOTE)
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
                  oldStartNote      = static_cast<Note*>(sp->startElement());
                  oldEndNote        = static_cast<Note*>(sp->endElement());
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
                        newStartNote = static_cast<Note*>(startElement);
                        newEndNote   = static_cast<Note*>(endElement);
                        }
                  // update spanner's start and end notes
                  if (newStartNote && newEndNote) {
                        oldStartNote->removeSpannerFor(sp);
                        oldEndNote->removeSpannerBack(sp);
                        sp->setNoteSpan(newStartNote, newEndNote);
                        newStartNote->addSpannerFor(sp);
                        newEndNote->addSpannerBack(sp);

                        if (sp->type() == Element::Type::GLISSANDO)
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
      if (spanner->type() == Element::Type::TIE) {
            Tie* tie = static_cast<Tie*>(spanner);
            static_cast<Note*>(endElement)->setTieBack(0);
            tie->endNote()->setTieBack(tie);
            static_cast<Note*>(startElement)->setTieFor(0);
            tie->startNote()->setTieFor(tie);
            }
      spanner->score()->setLayout(spanner->tick());
      spanner->score()->setLayout(spanner->tick2());
      }

//---------------------------------------------------------
//   ChangeParent
//---------------------------------------------------------

void ChangeParent::flip()
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

void ChangeMMRest::flip()
      {
      Measure* mmr = m->mmRest();
      m->setMMRest(mmrest);
      mmrest = mmr;
      }

//---------------------------------------------------------
//   InsertTime
//---------------------------------------------------------

void InsertTime::redo()
      {
      score->insertTime(tick, len);
      }

void InsertTime::undo()
      {
      score->insertTime(tick, -len);
      }

//---------------------------------------------------------
//   ChangeNoteEvent::flip
//---------------------------------------------------------

void ChangeNoteEvent::flip()
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
      if (l != nullptr) {
            // don't use current le if null or if it is no longer linked (shouldn't happen)
            if (le && !l->contains(le)) {
                  le = nullptr;
                  qWarning("doUnlink(): current le %p no longer linked", le);
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
                  qDebug("Link::undo(): current le was null... we picked a new one le %p", le);
                  }
            }
      else
            qWarning("doUnlink(): current element %p has no links", e);

      if (le)
            le->unlink();
      else
            qWarning("doUnlink(): nothing found to unlink");
      }

void LinkStaff::redo()   { s1->linkTo(s2); }
void LinkStaff::undo()   { s1->unlink(s2); }
void UnlinkStaff::redo() { s1->unlink(s2); }
void UnlinkStaff::undo() { s1->linkTo(s2); }

//---------------------------------------------------------
//   ChangeStartEndSpanner::flip
//---------------------------------------------------------

void ChangeStartEndSpanner::flip()
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

void ChangeMetaTags::flip()
      {
      QMap<QString,QString> t = score->metaTags();
      score->setMetaTags(metaTags);
      metaTags = t;
      }

//---------------------------------------------------------
//   ChangeDrumset::flip
//---------------------------------------------------------

void ChangeDrumset::flip()
      {
      Drumset d = *instrument->drumset();
      instrument->setDrumset(&drumset);
      drumset = d;
      }

//---------------------------------------------------------
//   undoChangeBarLine
//---------------------------------------------------------

void Score::undoChangeBarLine(Measure* measure, BarLineType barType)
      {
      int tick = measure->tick();

      for (Score* s : scoreList()) {
            Measure* m  = s->tick2measure(tick);
            Measure* nm = m->nextMeasure();

            switch (barType) {
                  case BarLineType::END:
                  case BarLineType::NORMAL:
                  case BarLineType::DOUBLE:
                  case BarLineType::BROKEN:
                  case BarLineType::DOTTED:
                        {
                        s->undoChangeProperty(m, P_ID::REPEAT_END, false);
                        if (nm)
                              s->undoChangeProperty(nm, P_ID::REPEAT_START, false);
                        Segment* segment = m->findSegment(Segment::Type::EndBarLine, m->endTick());
                        if (segment) {
                              for (Element* e : segment->elist()) {
                                    if (e) {
                                          BarLine* bl = toBarLine(e);
                                          bl->undoChangeProperty(P_ID::BARLINE_TYPE, QVariant::fromValue(barType));
                                          bl->undoChangeProperty(P_ID::GENERATED, false);
                                          }
                                    }
                              }
                        }
                        break;
                  case BarLineType::START_REPEAT:
                        s->undoChangeProperty(m, P_ID::REPEAT_START, true);
                        break;
                  case BarLineType::END_REPEAT:
                        s->undoChangeProperty(m, P_ID::REPEAT_END, true);
                        if (nm)
                              s->undoChangeProperty(nm, P_ID::REPEAT_START, false);
                        break;
                  case BarLineType::END_START_REPEAT:
                        s->undoChangeProperty(m, P_ID::REPEAT_END, true);
                        if (nm)
                              s->undoChangeProperty(nm, P_ID::REPEAT_START, true);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   undoChangeGap
//---------------------------------------------------------

void ChangeGap::flip()
      {
      rest->setGap(v);
      v = !v;
      }

}
