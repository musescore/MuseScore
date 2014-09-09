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

#include "note.h"
#include "rest.h"
#include "chord.h"
#include "key.h"
#include "sig.h"
#include "clef.h"
#include "score.h"
#include "slur.h"
#include "tie.h"
#include "hairpin.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "timesig.h"
#include "page.h"
#include "barline.h"
#include "tuplet.h"
#include "lyrics.h"
#include "image.h"
#include "keysig.h"
#include "beam.h"
#include "utils.h"
#include "harmony.h"
#include "system.h"
#include "navigate.h"
#include "articulation.h"
#include "drumset.h"
#include "measure.h"
#include "tempo.h"
#include "undo.h"
#include "stringdata.h"
#include "stafftype.h"
#include "tupletmap.h"
#include "tiemap.h"
#include "stem.h"
#include "iname.h"
#include "range.h"
#include "hook.h"
#include "pitchspelling.h"
#include "tempotext.h"
#include "dynamic.h"
#include "repeat.h"
#include "bracket.h"
#include "ottava.h"

namespace Ms {

//---------------------------------------------------------
//   getSelectedNote
//---------------------------------------------------------

Note* Score::getSelectedNote()
      {
      Element* el = selection().element();
      if (el) {
            if (el->type() == Element::Type::NOTE)
                  return static_cast<Note*>(el);
            }
      selectNoteMessage();
      return 0;
      }

//---------------------------------------------------------
//   getSelectedChordRest
//---------------------------------------------------------

ChordRest* Score::getSelectedChordRest() const
      {
      Element* el = selection().element();
      if (el) {
            if (el->type() == Element::Type::NOTE)
                  return static_cast<Note*>(el)->chord();
            else if (el->type() == Element::Type::REST || el->type() == Element::Type::REPEAT_MEASURE)
                  return static_cast<Rest*>(el);
            else if (el->type() == Element::Type::CHORD)
                  return static_cast<Chord*>(el);
            }
      selectNoteRestMessage();
      return 0;
      }

//---------------------------------------------------------
//   getSelectedChordRest2
//---------------------------------------------------------

void Score::getSelectedChordRest2(ChordRest** cr1, ChordRest** cr2) const
      {
      *cr1 = 0;
      *cr2 = 0;
      foreach(Element* e, selection().elements()) {
            if (e->type() == Element::Type::NOTE)
                  e = e->parent();
            if (e->isChordRest()) {
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  if (*cr1 == 0 || (*cr1)->tick() > cr->tick())
                        *cr1 = cr;
                  if (*cr2 == 0 || (*cr2)->tick() < cr->tick())
                        *cr2 = cr;
                  }
            }
      if (*cr1 == 0)
            selectNoteRestMessage();
      if (*cr1 == *cr2)
            *cr2 = 0;
      }

//---------------------------------------------------------
//   pos
//---------------------------------------------------------

int Score::pos()
      {
      Element* el = selection().element();
      if (selection().activeCR())
            el = selection().activeCR();
      if (el) {
            switch(el->type()) {
                  case Element::Type::NOTE:
                        el = el->parent();
                        // fall through
                  case Element::Type::REPEAT_MEASURE:
                  case Element::Type::REST:
                  case Element::Type::CHORD:
                        return static_cast<ChordRest*>(el)->tick();
                  default:
                        break;
                  }
            }
      return -1;
      }

//---------------------------------------------------------
//   addRest
//    create one Rest at tick with duration d
//    create segment if necessary
//---------------------------------------------------------

Rest* Score::addRest(int tick, int track, TDuration d, Tuplet* tuplet)
      {
      Measure* measure = tick2measure(tick);
      Rest* rest       = new Rest(this, d);
      if (d.type() == TDuration::DurationType::V_MEASURE)
            rest->setDuration(measure->stretchedLen(staff(track/VOICES)));
      else
            rest->setDuration(d.fraction());
      rest->setTrack(track);
      rest->setTuplet(tuplet);
      undoAddCR(rest, measure, tick);
      return rest;
      }

//---------------------------------------------------------
//   addRest
//---------------------------------------------------------

Rest* Score::addRest(Segment* s, int track, TDuration d, Tuplet* tuplet)
      {
      Rest* rest = new Rest(this, d);
      if (d.type() == TDuration::DurationType::V_MEASURE)
            rest->setDuration(s->measure()->stretchedLen(staff(track/VOICES)));
      else
            rest->setDuration(d.fraction());
      rest->setTrack(track);
      rest->setParent(s);
      rest->setTuplet(tuplet);
      undoAddCR(rest, tick2measure(s->tick()), s->tick());
      return rest;
      }

//---------------------------------------------------------
//   addChord
//    Create one Chord at tick with duration d
//    - create segment if necessary.
//    - Use chord "oc" as prototype;
//    - if "genTie" then tie to chord "oc"
//---------------------------------------------------------

Chord* Score::addChord(int tick, TDuration d, Chord* oc, bool genTie, Tuplet* tuplet)
      {
      Measure* measure = tick2measure(tick);
      if (measure->endTick() <= tick) {
            qDebug("Score::addChord(): end of score?");
            return 0;
            }

      Chord* chord = new Chord(this);
      chord->setTuplet(tuplet);
      chord->setTrack(oc->track());
      chord->setDurationType(d);
      chord->setDuration(d.fraction());

      foreach(Note* n, oc->notes()) {
            Note* nn = new Note(this);
            chord->add(nn);
            nn->setPitch(n->pitch());
            nn->setTpc1(n->tpc1());
            nn->setTpc2(n->tpc2());
            }
      undoAddCR(chord, measure, tick);

      //
      // now as both chords are in place
      // (have segments as parent) we can add ties:
      //
      if (genTie) {
            int n = oc->notes().size();
            for(int i = 0; i < n; ++i) {
                  Note* n  = oc->notes()[i];
                  Note* nn = chord->notes()[i];
                  Tie* tie = new Tie(this);
                  tie->setStartNote(n);
                  tie->setEndNote(nn);
                  tie->setTrack(n->track());
                  undoAddElement(tie);
                  }
            }

      measure->cmdUpdateNotes(chord->staffIdx());
      return chord;
      }

//---------------------------------------------------------
//   addClone
//---------------------------------------------------------

ChordRest* Score::addClone(ChordRest* cr, int tick, const TDuration& d)
      {
qDebug("addClone %s at %d %s", cr->name(), tick, qPrintable(d.fraction().print()));
      ChordRest* newcr;
      // change a RepeatMeasure() into an Rest()
      if (cr->type() == Element::Type::REPEAT_MEASURE)
            newcr = new Rest(*static_cast<Rest*>(cr));
      else
            newcr = static_cast<ChordRest*>(cr->clone());
      newcr->rxpos() = 0.0;
      newcr->setDurationType(d);
      newcr->setDuration(d.fraction());
      newcr->setTuplet(cr->tuplet());
      newcr->setSelected(false);

      undoAddCR(newcr, cr->measure(), tick);
      return newcr;
      }

//---------------------------------------------------------
//   setRest
//    create one or more rests to fill "l"
//---------------------------------------------------------

Rest* Score::setRest(int tick, int track, Fraction l, bool useDots, Tuplet* tuplet, bool useFullMeasureRest)
      {
      Measure* measure = tick2measure(tick);
      Rest* r = 0;
      Staff* staff = Score::staff(track / VOICES);

      while (!l.isZero()) {
            //
            // divide into measures
            //
            Fraction f;
            if (tuplet) {
                  int ticks = (tuplet->tick() + tuplet->actualTicks()) - tick;

                  f = Fraction::fromTicks(ticks);
                  for (Tuplet* t = tuplet; t; t = t->tuplet())
                        f *= t->ratio();
                  //
                  // restrict to tuplet len
                  //
                  if (f < l)
                        l = f;
                  }
            else if (measure->tick() < tick)
                  f = Fraction::fromTicks(measure->tick() + measure->ticks() - tick);
            else
                  f = measure->len();
            f *= staff->timeStretch(tick);
            f.reduce();

            if (f > l)
                  f = l;

            if ((track % VOICES) && !measure->hasVoice(track) && (tick == measure->tick())) {
                  l -= f;
                  measure = measure->nextMeasure();
                  if (!measure)
                        break;
                  tick = measure->tick();
                  continue;
                  }

            if ((measure->timesig() == measure->len())   // not in pickup measure
               && (measure->tick() == tick)
               && (measure->stretchedLen(staff) == f)
               && (useFullMeasureRest)) {
                  Rest* rest = addRest(tick, track, TDuration(TDuration::DurationType::V_MEASURE), tuplet);
                  tick += rest->actualTicks();
                  if (r == 0)
                        r = rest;
                  }
            else {
                  //
                  // compute list of durations which will fit l
                  //
                  QList<TDuration> dList = toDurationList(f, useDots);
                  if (dList.isEmpty())
                        return 0;
                  foreach(TDuration d, dList) {
                        qDebug("    duration %d/%d", d.fraction().numerator(), d.fraction().denominator());
                        }

                  Rest* rest = 0;
                  if (((tick - measure->tick()) % dList[0].ticks()) == 0) {
                        foreach(TDuration d, dList) {
                              rest = addRest(tick, track, d, tuplet);
                              if (r == 0)
                                    r = rest;
                              tick += rest->actualTicks();
                              }
                        }
                  else {
                        for (int i = dList.size() - 1; i >= 0; --i) {
                              rest = addRest(tick, track, dList[i], tuplet);
                              if (r == 0)
                                    r = rest;
                              tick += rest->actualTicks();
                              }
                        }
                  }
            l -= f;

            measure = measure->nextMeasure();
            if (!measure)
                  break;
            tick = measure->tick();
            }
      return r;
      }

//---------------------------------------------------------
//   addNote from NoteVal
//---------------------------------------------------------

Note* Score::addNote(Chord* chord, NoteVal& noteVal)
      {
      Note* note = new Note(this);
      note->setParent(chord);
      note->setTrack(chord->track());
      note->setNval(noteVal);
      undoAddElement(note);
      _playNote = true;
      select(note, SelectType::SINGLE, 0);
      if (!chord->staff()->isTabStaff())
            _is.moveToNextInputPos();
      return note;
      }

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures from fm to lm
//---------------------------------------------------------

bool Score::rewriteMeasures(Measure* fm, Measure* lm, const Fraction& ns)
      {
      int measures = 1;
      for (Measure* m = fm; m != lm; m = m -> nextMeasure())
            ++measures;

      ScoreRange range;
      range.read(fm->first(), lm->last());
      if (!range.canWrite(ns))
            return false;

      //
      // calculate number of required measures = nm
      //
      Fraction k = range.duration() / ns;
      int nm     = (k.numerator() + k.denominator() - 1)/ k.denominator();

      Fraction nd(nm * ns.numerator(), ns.denominator()); // total new duration

      // evtl. we have to fill the last measure
      Fraction fill = nd - range.duration();
      range.fill(fill);

      for (Score* s : scoreList()) {
            Measure* m1 = s->tick2measure(fm->tick());
            Measure* m2 = s->tick2measure(lm->tick());
            s->undoRemoveMeasures(m1, m2);

            Measure* nlm = 0;
            Measure* nfm = 0;
            int tick     = 0;
            bool endBarGenerated = m1->endBarLineGenerated();
            for (int i = 0; i < nm; ++i) {
                  Measure* m = new Measure(s);
                  m->setPrev(nlm);
                  if (nlm)
                        nlm->setNext(m);
                  m->setTimesig(ns);
                  m->setLen(ns);
                  m->setTick(tick);
                  m->setEndBarLineType(BarLineType::NORMAL, endBarGenerated);
                  tick += m->ticks();
                  nlm = m;
                  if (nfm == 0)
                        nfm = m;
                  }
            nlm->setEndBarLineType(m2->endBarLineType(), m2->endBarLineGenerated(),
               m2->endBarLineVisible(), m2->endBarLineColor());
            //
            // insert new calculated measures
            //
            nfm->setPrev(m1->prev());
            nlm->setNext(m2->next());
            s->undo(new InsertMeasures(nfm, nlm));
            }

      if (!range.write(rootScore(), fm->tick()))
            qFatal("Cannot write measures");
      connectTies(true);

      return true;
      }

//---------------------------------------------------------
//   warnTupletCrossing
//---------------------------------------------------------

static void warnTupletCrossing()
      {
      if (!MScore::noGui) {
            const char* tt = QT_TRANSLATE_NOOP("addRemoveTimeSig", "MuseScore");
            const char* mt = QT_TRANSLATE_NOOP("addRemoveTimeSig", "Cannot rewrite measures:\n"
               "Tuplet would cross measure");

            QMessageBox::warning(0, qApp->translate("addRemoveTimeSig", tt),
               qApp->translate("addRemoveTimeSig", mt));
            }
      }

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures up to the next time signature
//---------------------------------------------------------

bool Score::rewriteMeasures(Measure* fm, const Fraction& ns)
      {
      Measure* lm  = fm;
      Measure* fm1 = fm;

      //
      // split into Measure segments fm-lm
      //
      for (MeasureBase* m = fm; ; m = m->next()) {
            if (!m || (m->type() != Element::Type::MEASURE)
              || (static_cast<Measure*>(m)->first(Segment::Type::TimeSig) && m != fm))
                  {
                  if (!rewriteMeasures(fm1, lm, ns)) {
                        warnTupletCrossing();
                        for (Measure* m = fm1; m; m = m->nextMeasure()) {
                              if (m->first(Segment::Type::TimeSig))
                                    break;
                              Fraction fr(ns);
                              undoChangeProperty(m, P_ID::TIMESIG_NOMINAL, QVariant::fromValue(fr));
                              }
                        return false;
                        }
                  if (!m || m->type() == Element::Type::MEASURE)
                        break;
                  while (m->type() != Element::Type::MEASURE) {
                        m = m->next();
                        if (!m)
                              break;
                        }
                  fm1 = static_cast<Measure*>(m);
                  if (fm1 == 0)
                        break;
                  }
            lm  = static_cast<Measure*>(m);
            }
      return true;
      }

//---------------------------------------------------------
//   cmdAddTimeSig
//
//    Add or change time signature at measure in response
//    to gui command (drop timesig on measure or timesig)
//---------------------------------------------------------

void Score::cmdAddTimeSig(Measure* fm, int staffIdx, TimeSig* ts, bool local)
      {
      Fraction ns  = ts->sig();
      int tick     = fm->tick();
      TimeSig* lts = staff(staffIdx)->timeSig(tick);

      Fraction stretch;
      Fraction lsig;                // last signature
      if (lts) {
            stretch = lts->stretch();
            lsig    = lts->sig();
            }
      else {
            stretch.set(1,1);
            lsig.set(4,4);          // set to default
            }

      int track    = staffIdx * VOICES;
      Segment* seg = fm->undoGetSegment(Segment::Type::TimeSig, tick);
      TimeSig* ots = static_cast<TimeSig*>(seg->element(track));
      if (ots) {
            //
            //  ignore if there is already a timesig
            //  with same values
            //
            if ((ots->timeSigType() == ts->timeSigType())
               && (ots->sig().identical(ts->sig()))
               && (ots->stretch() == ts->stretch())
               && ots->groups() == ts->groups()) {
                  delete ts;
                  return;
                  }
            }

      if (local) {
            ts->setParent(seg);
            ts->setTrack(track);
            ts->setStretch((ns / fm->timesig()).reduced());
            ts->setSelected(false);
            undoAddElement(ts);
            timesigStretchChanged(ts, fm, staffIdx);
            return;
            }

      if (ots && ots->sig() == ts->sig() && ots->stretch() == ts->stretch()) {
            foreach (Score* score, scoreList()) {
                  Measure* fm = score->tick2measure(tick);
                  for (Measure* m = fm; m; m = m->nextMeasure()) {
                        if ((m != fm) && m->first(Segment::Type::TimeSig))
                              break;
                        bool changeActual = m->len() == m->timesig();
                        undoChangeProperty(m, P_ID::TIMESIG_NOMINAL, QVariant::fromValue(ns));
                        if (changeActual)
                              undoChangeProperty(m, P_ID::TIMESIG_ACTUAL,  QVariant::fromValue(ns));
                        undoChangeProperty(ots, P_ID::GROUPS,  QVariant::fromValue(ts->groups()));
                        }
                  }
            int n = nstaves();
            for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
                  TimeSig* nsig = static_cast<TimeSig*>(seg->element(staffIdx * VOICES));
                  undoChangeProperty(nsig, P_ID::TIMESIG_TYPE, int(ts->timeSigType()));
                  }
            }
      else {
            Score* score = rootScore();
            Measure* fm  = score->tick2measure(tick);
            //
            // rewrite all measures up to the next time signature
            //
            if (fm == score->firstMeasure() && (fm->len() != fm->timesig())) {
                  // handle upbeat
                  undoChangeProperty(fm, P_ID::TIMESIG_NOMINAL, QVariant::fromValue(ns));
                  Measure* m = fm->nextMeasure();
                  Segment* s = m->findSegment(Segment::Type::TimeSig, m->tick());
                  fm = s ? 0 : fm->nextMeasure();
                  }
            else {
                  if (sigmap()->timesig(seg->tick()).timesig() == ts->sig())
                        fm = 0;
                  }
            if (fm) {
                  if (!score->rewriteMeasures(fm, ns)) {
                        delete ts;
                        return;
                        }
                  }

            foreach (Score* score, scoreList()) {
                  Measure* nfm = score->tick2measure(tick);
                  seg   = nfm->undoGetSegment(Segment::Type::TimeSig, nfm->tick());
                  int n = score->nstaves();
                  for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
                        TimeSig* nsig = static_cast<TimeSig*>(seg->element(staffIdx * VOICES));
                        if (nsig == 0) {
                              nsig = new TimeSig(score);
                              nsig->setTrack(staffIdx * VOICES);
                              nsig->setParent(seg);
                              nsig->setSig(ts->sig(), ts->timeSigType());
                              nsig->setGroups(ts->groups());
                              undoAddElement(nsig);
                              }
                        else {
                              undo(new ChangeTimesig(nsig, false, ts->sig(), ts->stretch(),
                                    ts->numeratorString(), ts->denominatorString(), ts->timeSigType()));
                              nsig->setSelected(false);
                              nsig->setDropTarget(0);       // DEBUG
                              }
                        }
                  }
            }
      delete ts;
      }

//---------------------------------------------------------
//   timesigStretchChanged
//---------------------------------------------------------

void Score::timesigStretchChanged(TimeSig* ts, Measure* fm, int staffIdx)
      {
      for (Measure* m = fm; m; m = m->nextMeasure()) {
            if ((m != fm) && m->first(Segment::Type::TimeSig))
                  break;
            int strack = staffIdx * VOICES;
            int etrack = strack + VOICES;
            for (Segment* s = m->first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
                  for (int track = strack; track < etrack; ++track) {
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (!cr)
                              continue;
                        if (cr->type() == Element::Type::REST && cr->durationType() == TDuration::DurationType::V_MEASURE)
                              cr->setDuration(ts->sig());
                        else
                              qDebug("timeSigChanged: not implemented: chord/rest does not fit");
                        }
                  }
            }
      }

//---------------------------------------------------------
//   cmdRemoveTimeSig
//---------------------------------------------------------

void Score::cmdRemoveTimeSig(TimeSig* ts)
      {
      Measure* m = ts->measure();

      //
      // we cannot remove a courtesy time signature
      //
      if (m->tick() != ts->segment()->tick())
            return;

      undoRemoveElement(ts->segment());

      Measure* pm = m->prevMeasure();
      Fraction ns(pm ? pm->timesig() : Fraction(4,4));

      rewriteMeasures(m, ns);
      }

//---------------------------------------------------------
//   cmdAddPitch
//---------------------------------------------------------

void Score::cmdAddPitch(int step, bool addFlag)
      {
      startCmd();
      addPitch(step, addFlag);
      endCmd();
      }

//---------------------------------------------------------
//   addPitch
//---------------------------------------------------------

void Score::addPitch(int step, bool addFlag)
      {
      Position pos;
      pos.segment   = inputState().segment();
      pos.staffIdx  = inputState().track() / VOICES;
      ClefType clef = staff(pos.staffIdx)->clef(pos.segment->tick());
      pos.line      = relStep(step, clef);

      if (addFlag) {
            Element* el = selection().element();
            if (el && el->type() == Element::Type::NOTE) {
                  Chord* chord = static_cast<Note*>(el)->chord();
                  NoteVal val;

                  Segment* s         = chord->segment();
                  AccidentalVal acci = s->measure()->findAccidental(s, chord->staffIdx(), pos.line);
                  int step           = absStep(pos.line, clef);
                  int octave         = step/7;
                  val.pitch          = step2pitch(step) + octave * 12 + int(acci);

                  if (!chord->concertPitch())
                        val.pitch += chord->staff()->part()->instr()->transpose().chromatic;
                  val.tpc = step2tpc(step % 7, acci);
                  addNote(chord, val);
                  endCmd();
                  return;
                  }
            }

      if (inputState().repitchMode())
            repitchNote(pos, !addFlag);
      else
            putNote(pos, !addFlag);
      }

//---------------------------------------------------------
//   addPitch
//---------------------------------------------------------

Note* Score::addPitch(NoteVal& nval, bool addFlag)
      {
      if (addFlag) {
            Chord* c = static_cast<Chord*>(_is.lastSegment()->element(_is.track()));

            if (c == 0 || c->type() != Element::Type::CHORD) {
                  qDebug("Score::addPitch: cr %s", c ? c->name() : "zero");
                  return 0;
                  }
            Note* note = addNote(c, nval);
            if (_is.lastSegment() == _is.segment())
                  _is.moveToNextInputPos();
            return note;
            }
      expandVoice();

      // insert note
      MScore::Direction stemDirection = MScore::Direction::AUTO;
      int track               = _is.track();
      if (_is.drumNote() != -1) {
            nval.pitch    = _is.drumNote();
            Drumset* ds   = _is.drumset();
            nval.headGroup  = ds->noteHead(nval.pitch);
            stemDirection = ds->stemDirection(nval.pitch);
            track         = ds->voice(nval.pitch) + (_is.track() / VOICES) * VOICES;
            _is.setTrack(track);
            expandVoice();
            }
      if (!_is.cr())
            return 0;
      Fraction duration;
      if (_is.repitchMode()) {
            duration = _is.cr()->duration();
            }
      else {
            duration = _is.duration().fraction();
            }
      Note* note = 0;
      Note* firstTiedNote = 0;
      Note* lastTiedNote = 0;
      if (_is.repitchMode() && _is.cr()->type() == Element::Type::CHORD) {
            // repitch mode for MIDI input (where we are given a pitch) is handled here
            // for keyboard input (where we are given a staff position), there is a separate function Score::repitchNote()
            // the code is similar enough that it could possibly be refactored
            Chord* chord = static_cast<Chord*>(_is.cr());
            note = new Note(this);
            note->setParent(chord);
            note->setTrack(chord->track());
            note->setNval(nval);
            lastTiedNote = note;
            if (!addFlag) {
                  QList<Note*> notes = chord->notes();
                  // break all ties into current chord
                  // these will exist only if user explicitly moved cursor to a tied-into note
                  // in ordinary use, cursor will autoamtically skip past these during note entry
                  for (Note* n : notes) {
                        if (n->tieBack())
                              undoRemoveElement(n->tieBack());
                        }
                  // for single note chords only, preserve ties by changing pitch of all forward notes
                  // the tie forward itself will be added later
                  // multi-note chords get reduced to single note chords anyhow since we remove the old notes below
                  // so there will be no way to preserve those ties
                  if (notes.size() == 1 && notes.first()->tieFor()) {
                        Note* tn = notes.first()->tieFor()->endNote();
                        while (tn) {
                              Chord* tc = tn->chord();
                              if (tc->notes().size() != 1) {
                                    undoRemoveElement(tn->tieBack());
                                    break;
                                    }
                              if (!firstTiedNote)
                                    firstTiedNote = tn;
                              lastTiedNote = tn;
                              undoChangePitch(tn, note->pitch(), note->tpc1(), note->tpc2());
                              if (tn->tieFor())
                                    tn = tn->tieFor()->endNote();
                              else
                                    break;
                              }
                        }
                  // remove all notes from chord
                  // the new note will be added below
                  while (!chord->notes().isEmpty())
                        undoRemoveElement(chord->notes().first());
                  }
            // add new note to chord
            undoAddElement(note);
            // recreate tie forward if there is a note to tie to
            // one-sided ties will not be recreated
            if (firstTiedNote) {
                  Tie* tie = new Tie(this);
                  tie->setStartNote(note);
                  tie->setEndNote(firstTiedNote);
                  tie->setTrack(note->track());
                  undoAddElement(tie);
                  }
            select(lastTiedNote);
            }
      else if (!_is.repitchMode()) {
            Segment* seg = setNoteRest(_is.segment(), track, nval, duration, stemDirection);
            if (seg) {
                  note = static_cast<Chord*>(seg->element(track))->upNote();
                  setLayoutAll(true);
                  }
            }

      if (_is.slur()) {
            //
            // extend slur
            //
            ChordRest* e = searchNote(_is.tick(), _is.track());
            if (e) {
                  int stick = 0;
                  Element* ee = _is.slur()->startElement();
                  if (ee->isChordRest())
                        stick = static_cast<ChordRest*>(ee)->tick();
                  else if (ee->type() == Element::Type::NOTE)
                        stick = static_cast<Note*>(ee)->chord()->tick();
                  if (stick == e->tick()) {
                        _is.slur()->setTick(stick);
                        _is.slur()->setStartElement(e);
                        }
                  else {
                        _is.slur()->setTick2(e->tick());
                        _is.slur()->setEndElement(e);
                        }
                  }
            else
                  qDebug("addPitch: cannot find slur note");
            setLayoutAll(true);
            }
      if (_is.repitchMode()) {
            // move cursor to next note, but skip tied ntoes (they were already repitched above)
            ChordRest* next = lastTiedNote ? nextChordRest(lastTiedNote->chord()) : nextChordRest(_is.cr());
            while (next && next->type() != Element::Type::CHORD)
                  next = nextChordRest(next);
            if (next)
                  _is.moveInputPos(next->segment());
            }
      else
            _is.moveToNextInputPos();
      return note;
      }

//---------------------------------------------------------
//   putNote
//    mouse click in state NoteType::ENTRY
//---------------------------------------------------------

void Score::putNote(const QPointF& pos, bool replace)
      {
      Position p;
      if (!getPosition(&p, pos, _is.voice())) {
            qDebug("cannot put note here, get position failed");
            return;
            }
      putNote(p, replace);
      }

void Score::putNote(const Position& p, bool replace)
      {
      Segment* s      = p.segment;
      int staffIdx    = p.staffIdx;
      int line        = p.line;
      int tick        = s->tick();
      Staff* st       = staff(staffIdx);
      ClefType clef   = st->clef(tick);

qDebug("putNote at tick %d staff %d line %d clef %d",
   tick, staffIdx, line, clef);

      _is.setTrack(staffIdx * VOICES + _is.voice());
      _is.setSegment(s);

      const Instrument* instr = st->part()->instr(s->tick());
      MScore::Direction stemDirection = MScore::Direction::AUTO;
      NoteVal nval;
      const StringData* stringData = 0;
      StaffType* tab = 0;

      switch (st->staffType()->group()) {
            case StaffGroup::PERCUSSION: {
                  if (_is.rest())
                        break;
                  Drumset* ds   = instr->drumset();
                  nval.pitch    = _is.drumNote();
                  if (nval.pitch < 0)
                        return;
                  nval.headGroup = ds->noteHead(nval.pitch);
                  if (nval.headGroup == NoteHead::Group::HEAD_INVALID)
                        return;
                  stemDirection = ds->stemDirection(nval.pitch);
                  break;
                  }
            case StaffGroup::TAB: {
                  if (_is.rest())
                        return;
                  stringData = instr->stringData();
                  tab = st->staffType();
                  int string = tab->visualStringToPhys(line);
                  if (string < 0 || string >= stringData->strings())
                      return;
                  // build a default NoteVal for that line
                  nval.string = string;
                  if (p.fret != FRET_NONE)       // if a fret is given, use it
                        nval.fret = p.fret;
                  else {                        // if no fret, use 0 as default
                        _is.setString(line);
                        nval.fret = 0;
                        }
                  nval.pitch = stringData->getPitch(string, nval.fret);
                  break;
                  }

            case StaffGroup::STANDARD: {
                  AccidentalVal acci = s->measure()->findAccidental(s, staffIdx, line);
                  int step   = absStep(line, clef);
                  int octave = step/7;
                  nval.pitch = step2pitch(step) + octave * 12 + int(acci);
                  if (!styleB(StyleIdx::concertPitch))
                        nval.pitch += instr->transpose().chromatic;
                  nval.tpc = step2tpc(step % 7, acci);
                  }
                  break;
            }

      expandVoice();
      ChordRest* cr = _is.cr();
      bool addToChord = false;

      if (cr) {
            // retrieve total duration of current chord
            TDuration d = cr->durationType();
            // if not in replace mode AND chord duration == input duration AND not rest input
            // we need to add to current chord (otherwise, we will need to replace it or create a new one)
            if (!replace
               && (d == _is.duration())
               && (cr->type() == Element::Type::CHORD)
               && !_is.rest())
                  {
                  if (st->isTabStaff()) {      // TAB
                        // if a note on same string already exists, update to new pitch/fret
                        foreach (Note* note, static_cast<Chord*>(cr)->notes())
                              if (note->string() == nval.string) {       // if string is the same
                                    // if adding a new digit will keep fret number within fret limit,
                                    // add a digit to existing fret number
                                    if (stringData) {
                                          int fret = note->fret() * 10 + nval.fret;
                                          if (fret <= stringData->frets() ) {
                                                nval.fret = fret;
                                                nval.pitch = stringData->getPitch(nval.string, nval.fret);
                                                }
                                          else
                                                qDebug("can't increase fret to %d", fret);
                                          }
                                    // set fret number (original or combined) in all linked notes
                                    int tpc1 = note->tpc1default(nval.pitch);
                                    int tpc2 = note->tpc2default(nval.pitch);
                                    undoChangeFretting(note, nval.pitch, nval.string, nval.fret, tpc1, tpc2);
                                    return;
                                    }
                        }
                  else {                        // not TAB
                        // if a note with the same pitch already exists in the chord, remove it
                        Chord* chord = static_cast<Chord*>(cr);
                        Note* note = chord->findNote(nval.pitch);
                        if (note) {
                              if (chord->notes().size() > 1)
                                    undoRemoveElement(note);
                              return;
                              }
                        }
                  addToChord = true;            // if no special case, add note to chord
                  }
            }
      if (addToChord && cr->type() == Element::Type::CHORD) {
            // if adding, add!
            addNote(static_cast<Chord*>(cr), nval);
            return;
            }
      else {
            // if not adding, replace current chord (or create a new one)

            if (_is.rest())
                  nval.pitch = -1;
            setNoteRest(_is.segment(), _is.track(), nval, _is.duration().fraction(), stemDirection);
            }
      if (!st->isTabStaff())
            _is.moveToNextInputPos();
      }

//---------------------------------------------------------
//   repitchNote
//---------------------------------------------------------

void Score::repitchNote(const Position& p, bool replace)
      {
      Segment* s      = p.segment;
      int tick        = s->tick();
      Staff* st       = staff(p.staffIdx);
      ClefType clef   = st->clef(tick);

      NoteVal nval;
      AccidentalVal acci = s->measure()->findAccidental(s, p.staffIdx, p.line);
      int step   = absStep(p.line, clef);
      int octave = step / 7;
      nval.pitch = step2pitch(step) + octave * 12 + int(acci);
      if (!styleB(StyleIdx::concertPitch))
            nval.pitch += st->part()->instr(s->tick())->transpose().chromatic;
      nval.tpc = step2tpc(step % 7, acci);

      Chord* chord;
      if (_is.cr()->type() == Element::Type::REST) { //skip rests
            ChordRest* next = nextChordRest(_is.cr());
            while(next && next->type() != Element::Type::CHORD)
                  next = nextChordRest(next);
            if (next)
                  _is.moveInputPos(next->segment());
            return;
            }
      else {
            chord = static_cast<Chord*>(_is.cr());
            }
      Note* note = new Note(this);
      note->setParent(chord);
      note->setTrack(chord->track());
      note->setNval(nval);

      Note* firstTiedNote = 0;
      Note* lastTiedNote = note;
      if (replace) {
            QList<Note*> notes = chord->notes();
            // break all ties into current chord
            // these will exist only if user explicitly moved cursor to a tied-into note
            // in ordinary use, cursor will autoamtically skip past these during note entry
            for (Note* n : notes) {
                  if (n->tieBack())
                        undoRemoveElement(n->tieBack());
                  }
            // for single note chords only, preserve ties by changing pitch of all forward notes
            // the tie forward itself will be added later
            // multi-note chords get reduced to single note chords anyhow since we remove the old notes below
            // so there will be no way to preserve those ties
            if (notes.size() == 1 && notes.first()->tieFor()) {
                  Note* tn = notes.first()->tieFor()->endNote();
                  while (tn) {
                        Chord* tc = tn->chord();
                        if (tc->notes().size() != 1) {
                              undoRemoveElement(tn->tieBack());
                              break;
                              }
                        if (!firstTiedNote)
                              firstTiedNote = tn;
                        lastTiedNote = tn;
                        undoChangePitch(tn, note->pitch(), note->tpc1(), note->tpc2());
                        if (tn->tieFor())
                              tn = tn->tieFor()->endNote();
                        else
                              break;
                        }
                  }
            // remove all notes from chord
            // the new note will be added below
            while (!chord->notes().isEmpty())
                  undoRemoveElement(chord->notes().first());
            }
      // add new note to chord
      undoAddElement(note);
      // recreate tie forward if there is a note to tie to
      // one-sided ties will not be recreated
      if (firstTiedNote) {
            Tie* tie = new Tie(this);
            tie->setStartNote(note);
            tie->setEndNote(firstTiedNote);
            tie->setTrack(note->track());
            undoAddElement(tie);
            }
      select(lastTiedNote);
      // move to next Chord
      ChordRest* next = nextChordRest(lastTiedNote->chord());
      while (next && next->type() != Element::Type::CHORD)
            next = nextChordRest(next);
      if (next)
            _is.moveInputPos(next->segment());
      }

//---------------------------------------------------------
//   cmdAddTie
//---------------------------------------------------------

void Score::cmdAddTie()
      {
      QList<Note*> noteList;
      Element* el = selection().element();
      if (el && el->type() == Element::Type::NOTE)
            noteList.append(static_cast<Note*>(el));
      else if (el && el->type() == Element::Type::STEM) {
            Chord* chord = static_cast<Stem*>(el)->chord();
            noteList = chord->notes();
            }
      else
            noteList = selection().noteList();
      if (noteList.isEmpty()) {
            qDebug("no notes selected");
            return;
            }

      startCmd();
      foreach (Note* note, noteList) {
            if (note->chord() &&  note->chord()->isGrace())
                  continue;
            if (note->tieFor()) {
                  qDebug("cmdAddTie: note %p has already tie? noteFor: %p", note, note->tieFor());
                  continue;
                  }
            Chord* chord = note->chord();

            if (noteEntryMode()) {
                  if (chord == _is.cr())      // if noteentry is started
                        break;

                  if (_is.cr() == 0)
                        expandVoice();
                  if (_is.cr() == 0)
                        break;

                  bool addFlag = false; // _is.cr()->type() == Element::Type::CHORD;

                  NoteVal nval(note->noteVal());
                  Note* n = addPitch(nval, addFlag);
                  if (n) {
                        // n is not necessarily next note if duration span over measure
                        Note* nnote = searchTieNote(note);
                        if (nnote) {
                              n->setLine(note->line());
                              n->setTpc(note->tpc());
                              Tie* tie = new Tie(this);
                              tie->setStartNote(note);
                              tie->setEndNote(nnote);
                              tie->setTrack(note->track());
                              undoAddElement(tie);
                              nextInputPos(n->chord(), false);
                              }
                        else
                              qDebug("cmdAddTie: no next note?");
                        }
                  }
            else {
                  Note* note2 = searchTieNote(note);
                  Part* part = chord->staff()->part();
                  int strack = part->staves()->front()->idx() * VOICES;
                  int etrack = strack + part->staves()->size() * VOICES;

                  for (Segment* seg = chord->segment()->next1(Segment::Type::ChordRest); seg; seg = seg->next1(Segment::Type::ChordRest)) {
                        bool noteFound = false;
                        for (int track = strack; track < etrack; ++track) {
                              ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
                              if (cr == 0 || cr->type() != Element::Type::CHORD)
                                    continue;
                              int staffIdx = cr->staffIdx() + cr->staffMove();
                              if (staffIdx != chord->staffIdx() + chord->staffMove())
                                    continue;
                              foreach(Note* n, static_cast<Chord*>(cr)->notes()) {
                                    if (n->pitch() == note->pitch()) {
                                          if (note2 == 0 || note->chord()->track() == chord->track())
                                                note2 = n;
                                          }
                                    else if (cr->track() == chord->track())
                                          noteFound = true;
                                    }
                              }
                        if (noteFound || note2)
                              break;
                        }
                  if (note2) {
                        Tie* tie = new Tie(this);
                        tie->setStartNote(note);
                        tie->setEndNote(note2);
                        tie->setTrack(note->track());
                        undoAddElement(tie);
                        }
                  }
            }
      endCmd();
      }

//---------------------------------------------------------
//   cmdAddOttava
//---------------------------------------------------------

void Score::cmdAddOttava(Ottava::Type type)
      {
      ChordRest* cr1;
      ChordRest* cr2;
      getSelectedChordRest2(&cr1, &cr2);
      if (!cr1)
            return;
      if (cr2 == 0)
            cr2 = cr1;

      Ottava* ottava = new Ottava(this);
      ottava->setOttavaType(type);

      ottava->setTrack(cr1->track());
      ottava->setTrack2(cr1->track());
      ottava->setTick(cr1->tick());
      ottava->setTick2(cr2->tick() + cr2->actualTicks());
      undoAddElement(ottava);
      if (!noteEntryMode())
            select(ottava, SelectType::SINGLE, 0);
      }

//---------------------------------------------------------
//   cmdSetBeamMode
//---------------------------------------------------------

void Score::cmdSetBeamMode(Beam::Mode mode)
      {
      ChordRest* cr = getSelectedChordRest();
      if (cr == 0)
            return;
      cr->setBeamMode(mode);
      _layoutAll = true;
      }

//---------------------------------------------------------
//   cmdFlip
//---------------------------------------------------------

void Score::cmdFlip()
      {
      const QList<Element*>& el = selection().elements();
      if (el.isEmpty()) {
            selectNoteSlurMessage();
            return;
            }
      foreach (Element* e, el) {
            if (e->type() == Element::Type::NOTE || e->type() == Element::Type::STEM || e->type() == Element::Type::HOOK) {
                  Chord* chord;
                  if (e->type() == Element::Type::NOTE)
                        chord = static_cast<Note*>(e)->chord();
                  else if (e->type() == Element::Type::STEM)
                        chord = static_cast<Stem*>(e)->chord();
                  else //if (e->type() == Element::Type::HOOK) // no other option left
                        chord = static_cast<Hook*>(e)->chord();
                  if (chord->beam())
                        e = chord->beam();  // fall trough
                  else {
                        MScore::Direction dir = chord->up() ? MScore::Direction::DOWN : MScore::Direction::UP;
                        undoChangeProperty(chord, P_ID::STEM_DIRECTION, int(dir));
                        }
                  }

            if (e->type() == Element::Type::BEAM) {
                  Beam* beam = static_cast<Beam*>(e);
                  MScore::Direction dir = beam->up() ? MScore::Direction::DOWN : MScore::Direction::UP;
                  undoChangeProperty(beam, P_ID::STEM_DIRECTION, int(dir));
                  }
            else if (e->type() == Element::Type::SLUR_SEGMENT) {
                  SlurTie* slur = static_cast<SlurSegment*>(e)->slurTie();
                  MScore::Direction dir = slur->up() ? MScore::Direction::DOWN : MScore::Direction::UP;
                  undoChangeProperty(slur, P_ID::SLUR_DIRECTION, int(dir));
                  }
            else if (e->type() == Element::Type::HAIRPIN_SEGMENT) {
                  Hairpin* h = static_cast<HairpinSegment*>(e)->hairpin();
                  Hairpin::Type st = h->hairpinType() == Hairpin::Type::CRESCENDO ? Hairpin::Type::CRESCENDO : Hairpin::Type::DECRESCENDO;
                  undoChangeProperty(h, P_ID::HAIRPIN_TYPE, int(st));
                  }
            else if (e->type() == Element::Type::ARTICULATION) {
                  Articulation* a = static_cast<Articulation*>(e);
                  if (a->articulationType() == ArticulationType::Staccato
                     || a->articulationType() == ArticulationType::Tenuto
                     || a->articulationType() == ArticulationType::Sforzatoaccent
                     || a->articulationType() == ArticulationType::FadeIn
                     || a->articulationType() == ArticulationType::FadeOut
                     || a->articulationType() == ArticulationType::VolumeSwell
                     || a->articulationType() == ArticulationType::WiggleSawtooth
                     || a->articulationType() == ArticulationType::WiggleSawtoothWide
                     || a->articulationType() == ArticulationType::WiggleVibratoLargeFaster
                     || a->articulationType() == ArticulationType::WiggleVibratoLargeSlowest) {
                        ArticulationAnchor aa = a->anchor();
                        if (aa == ArticulationAnchor::TOP_CHORD)
                              aa = ArticulationAnchor::BOTTOM_CHORD;
                        else if (aa == ArticulationAnchor::BOTTOM_CHORD)
                              aa = ArticulationAnchor::TOP_CHORD;
                        else if (aa == ArticulationAnchor::CHORD)
                              aa = a->up() ? ArticulationAnchor::BOTTOM_CHORD : ArticulationAnchor::TOP_CHORD;
                        if (aa != a->anchor())
                              undoChangeProperty(a, P_ID::ARTICULATION_ANCHOR, int(aa));
                        }
                  else {
                        MScore::Direction d = a->up() ? MScore::Direction::DOWN : MScore::Direction::UP;
                        undoChangeProperty(a, P_ID::DIRECTION, int(d));
                        }
                  return;   // no layoutAll
                  }
            else if (e->type() == Element::Type::TUPLET) {
                  Tuplet* tuplet = static_cast<Tuplet*>(e);
                  MScore::Direction d = tuplet->isUp() ? MScore::Direction::DOWN : MScore::Direction::UP;
                  undoChangeProperty(tuplet, P_ID::DIRECTION, int(d));
                  }
            else if (e->type() == Element::Type::NOTEDOT) {
                  Note* note = static_cast<Note*>(e->parent());
                  MScore::Direction d = note->dotIsUp() ? MScore::Direction::DOWN : MScore::Direction::UP;
                  undoChangeProperty(note, P_ID::DOT_POSITION, int(d));
                  // undo(new FlipNoteDotDirection(static_cast<Note*>(e->parent())));
                  }
            else if (
                 (e->type() == Element::Type::TEMPO_TEXT)
               | (e->type() == Element::Type::DYNAMIC)
               | (e->type() == Element::Type::HAIRPIN)
               | (e->type() == Element::Type::DYNAMIC)
               ) {
                  Element::Placement p = e->placement() == Element::Placement::ABOVE ? Element::Placement::BELOW : Element::Placement::ABOVE;
                  undoChangeProperty(e, P_ID::PLACEMENT, int(p));
                  }
            }
      _layoutAll = true;      // must be set in und/redo
      }

//---------------------------------------------------------
//   deleteItem
//---------------------------------------------------------

void Score::deleteItem(Element* el)
      {
      if (!el)
            return;
      switch (el->type()) {
            case Element::Type::INSTRUMENT_NAME: {
                  Part* part = el->staff()->part();
                  InstrumentName* in = static_cast<InstrumentName*>(el);
                  if (in->instrumentNameType() == InstrumentNameType::LONG)
                        undo(new ChangeInstrumentLong(0, part, QList<StaffName>()));
                  else if (in->instrumentNameType() == InstrumentNameType::SHORT)
                        undo(new ChangeInstrumentShort(0, part, QList<StaffName>()));
                  }
                  break;

            case Element::Type::TIMESIG:
                  cmdRemoveTimeSig(static_cast<TimeSig*>(el));
                  break;

            case Element::Type::KEYSIG:
                  undoRemoveElement(el);
                  cmdUpdateNotes();
                  break;

            case Element::Type::NOTE:
                  {
                  Chord* chord = static_cast<Chord*>(el->parent());
                  if (chord->notes().size() > 1) {
                        undoRemoveElement(el);
                        select(chord->downNote(), SelectType::SINGLE, 0);
                        break;
                        }
                  // else fall through
                  el = chord;
                  }

            case Element::Type::CHORD:
                  {
                  Chord* chord = static_cast<Chord*>(el);
                  removeChordRest(chord, false);

                  // replace with rest
                  if (chord->noteType() == NoteType::NORMAL) {
                        Rest* rest = new Rest(this, chord->durationType());
                        rest->setDurationType(chord->durationType());
                        rest->setDuration(chord->duration());

                        rest->setTrack(el->track());
                        rest->setParent(chord->parent());
                        Segment* segment = chord->segment();
                        undoAddCR(rest, segment->measure(), segment->tick());

                        Tuplet* tuplet = chord->tuplet();
                        if (tuplet) {
                              tuplet->add(rest);
                              rest->setTuplet(tuplet);
                              rest->setDurationType(chord->durationType());
                              }
                        select(rest, SelectType::SINGLE, 0);
                        }
                  else  {
                        // remove segment if empty
                        Segment* seg = chord->segment();
                        if (seg->isEmpty())
                              undoRemoveElement(seg);
                        }
                  }
                  break;

            case Element::Type::REPEAT_MEASURE:
                  {
                  RepeatMeasure* rm = static_cast<RepeatMeasure*>(el);
                  removeChordRest(rm, false);
                  Rest* rest = new Rest(this);
                  rest->setDurationType(TDuration::DurationType::V_MEASURE);
                  rest->setDuration(rm->measure()->len());
                  rest->setTrack(rm->track());
                  rest->setParent(rm->parent());
                  Segment* segment = rm->segment();
                  undoAddCR(rest, segment->measure(), segment->tick());
                  }

            case Element::Type::REST:
                  //
                  // only allow for voices != 0
                  //    e.g. voice 0 rests cannot be removed
                  //
                  {
                  Rest* rest = static_cast<Rest*>(el);
                  if (rest->tuplet() && rest->tuplet()->elements().empty())
                        undoRemoveElement(rest->tuplet());
                  if (el->voice() != 0)
                        undoRemoveElement(el);
                  }
                  break;

            case Element::Type::ACCIDENTAL:
                  if (el->parent()->type() == Element::Type::NOTE)
                        changeAccidental(static_cast<Note*>(el->parent()), Accidental::Type::NONE);
                  else
                        undoRemoveElement(el);
                  break;

            case Element::Type::BAR_LINE:
                  {
                  BarLine* bl  = static_cast<BarLine*>(el);
                  if (bl->parent()->type() != Element::Type::SEGMENT)
                        break;
                  Segment* seg   = static_cast<Segment*>(bl->parent());
                  bool normalBar = seg->measure()->endBarLineType() == BarLineType::NORMAL;
                  int tick       = seg->tick();
                  Segment::Type segType = seg->segmentType();

                  foreach(Score* score, scoreList()) {
                        Measure* m   = score->tick2measure(tick);
                        if (segType == Segment::Type::StartRepeatBarLine)
                              undoChangeProperty(m, P_ID::REPEAT_FLAGS, int(m->repeatFlags()) & ~int(Repeat::START));
                        else if (segType == Segment::Type::BarLine)
                              undoRemoveElement(el);
                        else if (segType == Segment::Type::EndBarLine) {
                              // if bar line has custom barLineType, change to barLineType of the whole measure
                              if (bl->customSubtype()) {
                                    undoChangeProperty(bl, P_ID::SUBTYPE, int(seg->measure()->endBarLineType()));
                                    }
                              // otherwise, if whole measure has special end bar line, change to normal
                              else if (!normalBar) {
                                    if (m->tick() >= tick)
                                          m = m->prevMeasure();
                                    undoChangeProperty(m, P_ID::REPEAT_FLAGS, int(m->repeatFlags()) & ~int(Repeat::END));
                                    Measure* nm = m->nextMeasure();
                                    if (nm)
                                          undoChangeProperty(nm, P_ID::REPEAT_FLAGS, int(nm->repeatFlags()) & ~int(Repeat::START));
                                    undoChangeEndBarLineType(m, BarLineType::NORMAL);
                                    m->setEndBarLineGenerated(true);
                                    }
                              }
                        }
                  }
                  break;

            case Element::Type::TUPLET:
                  cmdDeleteTuplet(static_cast<Tuplet*>(el), true);
                  break;

            case Element::Type::MEASURE: {
                  Measure* m = static_cast<Measure*>(el);
                  undoRemoveMeasures(m, m);
                  undoInsertTime(m->tick(), -(m->endTick() - m->tick()));
                  }
                  break;

            case Element::Type::BRACKET:
                  undoRemoveBracket(static_cast<Bracket*>(el));
                  break;

            case Element::Type::LAYOUT_BREAK:
                  {
                  undoRemoveElement(el);
                  LayoutBreak* lb = static_cast<LayoutBreak*>(el);
                  Measure* m = lb->measure();
                  if (m->isMMRest()) {
                        // propagate to original measure
                        m = m->mmRestLast();
                        foreach(Element* e, *m->el()) {
                              if (e->type() == Element::Type::LAYOUT_BREAK) {
                                    undoRemoveElement(e);
                                    break;
                                    }
                              }
                        }
                  }
                  break;

            case Element::Type::CLEF:
                  {
                  Clef* clef = static_cast<Clef*>(el);
                  Measure* m = clef->measure();
                  if (m->isMMRest()) {
                        // propagate to original measure
                        m = m->mmRestLast();
                        Segment* s = m->findSegment(Segment::Type::Clef, clef->segment()->tick());
                        if (s && s->element(clef->track())) {
                              Clef* c = static_cast<Clef*>(s->element(clef->track()));
                              undoRemoveElement(c);
                              }
                        }
                  else {
                        if (clef->generated()) {
                              // find the real clef if this is a cautionary one
                              Measure* m = clef->measure();
                              if (m && m->prevMeasure()) {
                                    int tick = m->tick();
                                    m = m->prevMeasure();
                                    Segment* s = m->findSegment(Segment::Type::Clef, tick);
                                    if (s && s->element(clef->track()))
                                          clef = static_cast<Clef*>(s->element(clef->track()));
                                    }
                              }
                        undoRemoveElement(clef);
                        }
                  }
                  break;

            case Element::Type::REHEARSAL_MARK:
            case Element::Type::TEMPO_TEXT:
                  {
                  Segment* s = static_cast<Segment*>(el->parent());
                  Measure* m = s->measure();
                  if (m->isMMRest()) {
                        // propagate to original measure/element
                        m = m->mmRestFirst();
                        Segment* ns = m->findSegment(Segment::Type::ChordRest, s->tick());
                        for (Element* e : ns->annotations()) {
                              if (e->type() == el->type() && e->track() == el->track()) {
                                    el = e;
                                    undoRemoveElement(el);
                                    break;
                                    }
                              }
                        }
                  else
                        undoRemoveElement(el);
                  }
                  break;

            case Element::Type::OTTAVA_SEGMENT:
            case Element::Type::HAIRPIN_SEGMENT:
            case Element::Type::TRILL_SEGMENT:
            case Element::Type::TEXTLINE_SEGMENT:
            case Element::Type::VOLTA_SEGMENT:
            case Element::Type::SLUR_SEGMENT:
            case Element::Type::PEDAL_SEGMENT:
                  el = static_cast<SpannerSegment*>(el)->spanner();

            default:
                  undoRemoveElement(el);
                  break;
            }
      }

//---------------------------------------------------------
//   cmdDeleteSelectedMeasures
//---------------------------------------------------------

void Score::cmdDeleteSelectedMeasures()
      {
      if (!selection().isRange())
            return;

      MeasureBase* is = selection().startSegment()->measure();
      Segment* seg    = selection().endSegment();
      MeasureBase* ie;
      // choose the correct last measure based on the end segment
      // this depends on whether a whole measure is selected or only a few notes within it
      if (seg)
            ie = seg->prev() ? seg->measure() : seg->measure()->prev();
      else
            ie = lastMeasure();

      // createEndBar if last measure is deleted
      bool createEndBar = false;
      if (ie->type() == Element::Type::MEASURE) {
            Measure* iem = static_cast<Measure*>(ie);
            createEndBar = (iem == lastMeasureMM()) && (iem->endBarLineType() == BarLineType::END);
            }

      // get the last deleted timesig in order to restore after deletion
      TimeSig* lastDeletedSig = 0;
      for (MeasureBase* mb = ie;; mb = mb->prev()) {
            if (mb->type() == Element::Type::MEASURE) {
                  Measure* m = static_cast<Measure*>(mb);
                  Segment* sts = m->findSegment(Segment::Type::TimeSig, m->tick());
                  if (sts) {
                        lastDeletedSig = static_cast<TimeSig*>(sts->element(0));
                        break;
                        }
                  }

            if (mb == is)
                  break;
            }

      int startTick = is->tick();
      int endTick   = ie->tick();

      foreach (Score* score, scoreList()) {
            Measure* is = score->tick2measure(startTick);
            Measure* ie = score->tick2measure(endTick);

            undoRemoveMeasures(is, ie);
            undoInsertTime(is->tick(), -(ie->endTick() - is->tick()));

            // adjust views
            Measure* focusOn = is->prevMeasure() ? is->prevMeasure() : firstMeasure();
            foreach(MuseScoreView* v, score->viewer)
                  v->adjustCanvasPosition(focusOn, false);

            if (createEndBar) {
                  Measure* lastMeasure = score->lastMeasure();
                  if (lastMeasure && lastMeasure->endBarLineType() == BarLineType::NORMAL)
                        undoChangeEndBarLineType(lastMeasure, BarLineType::END);
                  }

            // insert correct timesig after deletion
            Measure* mBeforeSel = is->prevMeasure();
            Measure* mAfterSel  = mBeforeSel ? mBeforeSel->nextMeasure() : firstMeasure();
            if (mAfterSel && lastDeletedSig) {
                  bool changed = true;
                  if (mBeforeSel) {
                        if (mBeforeSel->timesig() == mAfterSel->timesig()) {
                              changed = false;
                              }
                        }
                  Segment* s = mAfterSel->findSegment(Segment::Type::TimeSig, mAfterSel->tick());
                  if (!s && changed) {
                        Segment* ns = mAfterSel->undoGetSegment(Segment::Type::TimeSig, mAfterSel->tick());
                        for (int staffIdx = 0; staffIdx < score->nstaves(); staffIdx++) {
                              TimeSig* nts = new TimeSig(score);
                              nts->setTrack(staffIdx * VOICES);
                              nts->setParent(ns);
                              nts->setSig(lastDeletedSig->sig(), lastDeletedSig->timeSigType());
                              score->undoAddElement(nts);
                              }
                        }
                  }
            }

      select(0, SelectType::SINGLE, 0);
      _is.setSegment(0);        // invalidate position
      }

//---------------------------------------------------------
//   cmdDeleteSelection
//---------------------------------------------------------

void Score::cmdDeleteSelection()
      {
      if (selection().isRange()) {
            Segment* s1 = selection().startSegment();
            Segment* s2 = selection().endSegment();

            int stick1 = selection().tickStart();
            int stick2 = selection().tickEnd();

            Segment* ss1 = s1;
            if (ss1->segmentType() != Segment::Type::ChordRest)
                  ss1 = ss1->next1(Segment::Type::ChordRest);
            bool fullMeasure = ss1 && (ss1->measure()->first(Segment::Type::ChordRest) == ss1)
                  && (s2 == 0 || (s2->segmentType() == Segment::Type::EndBarLine));

            int tick2   = s2 ? s2->tick() : INT_MAX;
            int track1  = selection().staffStart() * VOICES;
            int track2  = selection().staffEnd() * VOICES;
            for (auto i : _spanner.findOverlapping(stick1, stick2 - 1)) {
                  Spanner* sp = i.value;
                  if (sp->track() >= track1 && sp->track() < track2) {
                        if (sp->tick() >= stick1 && sp->tick() < stick2
                            && sp->tick2() >= stick1 && sp->tick2() < stick2) {
                              undoRemoveElement(sp);
                              }
                        else if (sp->type() == Element::Type::SLUR
                                 && ((sp->tick() >= stick1 && sp->tick() < stick2)
                                     || (sp->tick2() >= stick1 && sp->tick2() < stick2))) {
                              undoRemoveElement(sp);
                              }
                        }
                  }
            for (int track = track1; track < track2; ++track) {
                  if (!selectionFilter().canSelectVoice(track))
                        continue;
                  Fraction f;
                  int tick  = -1;
                  Tuplet* tuplet = 0;
                  for (Segment* s = s1; s && (s->tick() < stick2); s = s->next1()) {
                        if (s->element(track) && s->segmentType() == Segment::Type::Breath) {
                              deleteItem(s->element(track));
                              continue;
                              }
                        foreach (Element* annotation, s->annotations()) {
                              if (!annotation->systemFlag() && annotation->track() == track)
                                    undoRemoveElement(annotation);
                              }

                        if (s->segmentType() != Segment::Type::ChordRest || !s->element(track))
                              continue;
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (tick == -1) {
                              // first ChordRest found:
                              int offset = cr->tick() - s->measure()->tick();
                              if (cr->measure()->tick() >= s1->tick() && offset) {
                                    f = Fraction::fromTicks(offset);
                                    tick = s->measure()->tick();
                                    }
                              else {
                                    tick   = s->tick();
                                    f      = Fraction();
                                    }
                              tuplet = cr->tuplet();
                              if (tuplet && (tuplet->tick() == tick) && ((tuplet->tick() + tuplet->actualTicks()) < tick2) ) {
                                    // remove complete top level tuplet

                                    Tuplet* t = cr->tuplet();
                                    while (t->tuplet())
                                          t = t->tuplet();
                                    cmdDeleteTuplet(t, false);
                                    f += t->duration();
                                    tuplet = 0;
                                    continue;
                                    }
                              }
                        if (tuplet != cr->tuplet()) {
                              Tuplet* t = cr->tuplet();
                              if (t && (((t->tick() + t->actualTicks()) <= tick2) || fullMeasure)) {
                                    // remove complete top level tuplet

                                    while (t->tuplet())
                                          t = t->tuplet();
                                    cmdDeleteTuplet(t, false);
                                    f += t->duration();
                                    tuplet = 0;
                                    continue;
                                    }
                              if (f.isValid())
                                    setRest(tick, track, f, false, tuplet);
                              tick = cr->tick();
                              tuplet = cr->tuplet();
                              removeChordRest(cr, true);
                              f = cr->duration();
                              }
                        else {
                              removeChordRest(cr, true);
                              f += cr->duration();
                              }
                        }
                  if (f.isValid() && !f.isZero()) {
                        fullMeasure = false;          // HACK

                        if (fullMeasure) {
                              // handle this as special case to be able to
                              // fix broken measures:

                              // ws: does not work as TimeSig may be already removed
                              for (Measure* m = s1->measure(); m; m = m->nextMeasure()) {
                                    Staff* staff = Score::staff(track / VOICES);
                                    int tick = m->tick();
                                    Fraction f = staff->timeSig(tick)->sig();
                                    setRest(tick, track, f, false, 0);
                                    if (s2 && (m == s2->measure()))
                                          break;
                                    }
                              }
                        else {
                              setRest(tick, track, f, false, tuplet);
                              }
                        }
                  }
            s1 = tick2segment(stick1);
            s2 = tick2segment(stick2,true);
            if (s1 == 0 || s2 == 0)
                  deselectAll();
            else {
                  _selection.setStartSegment(s1);
                  _selection.setEndSegment(s2);
                  _selection.updateSelectedElements();
                  }
            }
      else {
            // deleteItem modifies selection().elements() list,
            // so we need a local copy:
            QList<Element*> el(selection().elements());
            if (el.isEmpty())
                  qDebug("...nothing selected");
            foreach(Element* e, el)
                  deleteItem(e);
            }
      deselectAll();
      _layoutAll = true;
      }

//---------------------------------------------------------
//   cmdFullMeasureRest
//---------------------------------------------------------

void Score::cmdFullMeasureRest()
      {
      if (selection().isRange()) {
            Segment* s1 = selection().startSegment();
            Segment* s2 = selection().endSegment();
            int stick1 = selection().tickStart();
            int stick2 = selection().tickEnd();

            Segment* ss1 = s1;
            if (ss1->segmentType() != Segment::Type::ChordRest)
                  ss1 = ss1->next1(Segment::Type::ChordRest);
            bool fullMeasure = ss1 && (ss1->measure()->first(Segment::Type::ChordRest) == ss1)
                  && (s2 == 0 || (s2->segmentType() == Segment::Type::EndBarLine)
                        || (s2->segmentType() == Segment::Type::TimeSigAnnounce)
                        || (s2->segmentType() == Segment::Type::KeySigAnnounce));

            if (!fullMeasure) {
                  return;
            }

            int track1  = selection().staffStart() * VOICES;
            int track2  = selection().staffEnd() * VOICES;
            for (int track = track1; track < track2; ++track) {
                  int tick  = -1;
                  for (Segment* s = s1; s != s2; s = s->next1()) {
                        if (!(s->measure()->isOnlyRests(track))) // Don't remove anything from measures that contain notes
                              continue;
                        if (s->segmentType() != Segment::Type::ChordRest || !s->element(track))
                              continue;
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));

                        if (tick == -1) {
                              // first ChordRest found:
                              tick = s->measure()->tick();
                              removeChordRest(cr, true);
                              }
                        else {
                              removeChordRest(cr, true);
                              }
                        }
                  for (Measure* m = s1->measure(); m; m = m->nextMeasure()) {
                        if (!(track % VOICES) && m->isOnlyRests(track)) {
                              addRest(m->tick(), track, TDuration(TDuration::DurationType::V_MEASURE), 0);
                              }
                        if (s2 && (m == s2->measure()))
                              break;
                        }
                  }
            s1 = tick2segment(stick1);
            s2 = tick2segment(stick2);
            if (s1 == 0 || s2 == 0)
                  deselectAll();
            else {
                  _selection.setStartSegment(s1);
                  _selection.setEndSegment(s2);
                  _selection.updateSelectedElements();
                  }
            }
      _layoutAll = true;
      }


//---------------------------------------------------------
//   addLyrics
//    called from Keyboard Accelerator & menu
//---------------------------------------------------------

Lyrics* Score::addLyrics()
      {
      Element* el = selection().element();
      if (el == 0 || (el->type() != Element::Type::NOTE && el->type() != Element::Type::LYRICS)) {
            QMessageBox::information(0,
               QMessageBox::tr("MuseScore"),
               QMessageBox::tr("No note or lyrics selected:\n"
                  "Please select a single note or lyrics and retry operation\n"),
               QMessageBox::Ok, QMessageBox::NoButton);
            return 0;
            }
      ChordRest* cr;
      if (el->type() == Element::Type::NOTE) {
            cr = static_cast<Note*>(el)->chord();
            if(cr->isGrace())
                  cr = static_cast<ChordRest*>(cr->parent());
            }
      else if (el->type() == Element::Type::LYRICS)
            cr = static_cast<Lyrics*>(el)->chordRest();
      else
            return 0;

      QList<Lyrics*> ll = cr->lyricsList();
      int no = ll.size();
      Lyrics* lyrics = new Lyrics(this);
      lyrics->setTrack(cr->track());
      lyrics->setParent(cr);
      lyrics->setNo(no);
      undoAddElement(lyrics);
      select(lyrics, SelectType::SINGLE, 0);
      return lyrics;
      }

//---------------------------------------------------------
//   addHairpin
//---------------------------------------------------------

Hairpin* Score::addHairpin(bool decrescendo, int tickStart, int tickEnd, int track)
      {
      Hairpin* pin = new Hairpin(this);
      pin->setHairpinType(decrescendo ? Hairpin::Type::DECRESCENDO : Hairpin::Type::CRESCENDO);
      pin->setTrack(track);
      pin->setTrack2(track);
      pin->setTick(tickStart);
      pin->setTick2(tickEnd);
      undoAddElement(pin);
      return pin;
      }

//---------------------------------------------------------
//   cmdCreateTuplet
//    replace cr with tuplet
//---------------------------------------------------------

void Score::cmdCreateTuplet(ChordRest* ocr, Tuplet* tuplet)
      {
qDebug("cmdCreateTuplet at %d <%s> track %d duration <%s> ratio <%s> baseLen <%s>",
  ocr->tick(), ocr->name(), ocr->track(),
  qPrintable(ocr->duration().print()),
  qPrintable(tuplet->ratio().print()),
  qPrintable(tuplet->baseLen().fraction().print())
            );

      int track        = ocr->track();
      Measure* measure = ocr->measure();
      int tick         = ocr->tick();

      if (ocr->tuplet())
            tuplet->setTuplet(ocr->tuplet());
      undoRemoveElement(ocr);

      ChordRest* cr;
      if (ocr->type() == Element::Type::CHORD) {
            cr = new Chord(this);
            foreach (Note* oldNote, static_cast<Chord*>(ocr)->notes()) {
                  Note* note = new Note(this);
                  note->setPitch(oldNote->pitch());
                  note->setTpc1(oldNote->tpc1());
                  note->setTpc2(oldNote->tpc2());
                  cr->add(note);
                  }
            }
      else
            cr = new Rest(this);

      Fraction an     = (tuplet->duration() * tuplet->ratio()) / tuplet->baseLen().fraction();
      int actualNotes = an.numerator() / an.denominator();

      tuplet->setTrack(track);
      cr->setTuplet(tuplet);
      cr->setTrack(track);
      cr->setDurationType(tuplet->baseLen());
      cr->setDuration(tuplet->baseLen().fraction());

qDebug("tuplet note duration %s  actualNotes %d  ticks %d track %d tuplet track %d",
      qPrintable(tuplet->baseLen().name()), actualNotes, cr->actualTicks(), track, tuplet->track());

      undoAddCR(cr, measure, tick);

      int ticks = cr->actualTicks();

      for (int i = 0; i < (actualNotes-1); ++i) {
            tick += ticks;
            Rest* rest = new Rest(this);
            rest->setTuplet(tuplet);
            rest->setTrack(track);
            rest->setDurationType(tuplet->baseLen());
            rest->setDuration(tuplet->baseLen().fraction());
            undoAddCR(rest, measure, tick);
            }
      _layoutAll = true;
      }

//---------------------------------------------------------
//   colorItem
//---------------------------------------------------------

void Score::colorItem(Element* element)
      {
      QColor sc(element->color());
      QColor c = QColorDialog::getColor(sc);
      if (!c.isValid())
            return;

      foreach(Element* e, selection().elements()) {
            if (e->color() != c) {
                  undoChangeProperty(e, P_ID::COLOR, c);
                  e->setGenerated(false);
                  refresh |= e->abbox();
                  if (e->type() == Element::Type::BAR_LINE) {
                        Element* ep = e->parent();
                        if (ep->type() == Element::Type::SEGMENT && static_cast<Segment*>(ep)->segmentType() == Segment::Type::EndBarLine) {
                              Measure* m = static_cast<Segment*>(ep)->measure();
                              BarLine* bl = static_cast<BarLine*>(e);
                              m->setEndBarLineType(bl->barLineType(), false, e->visible(), e->color());
                              }
                        }
                  }
            }
      deselectAll();
      }

//---------------------------------------------------------
//   cmdExchangeVoice
//---------------------------------------------------------

void Score::cmdExchangeVoice(int s, int d)
      {
      if (!selection().isRange()) {
            selectStavesMessage();
            return;
            }
      int t1 = selection().tickStart();
      int t2 = selection().tickEnd();

      Measure* m1 = tick2measure(t1);
      Measure* m2 = tick2measure(t2);
      if (t2 > m2->tick())
            m2 = 0;

      for (;;) {
            undoExchangeVoice(m1, s, d, selection().staffStart(), selection().staffEnd());
            m1 = m1->nextMeasure();
            if ((m1 == 0) || (m2 && (m1->tick() == m2->tick())))
                  break;
            }
      }

//---------------------------------------------------------
//   cmdEnterRest
//---------------------------------------------------------

void Score::cmdEnterRest(const TDuration& d)
      {
      if (_is.track() == -1) {
            qDebug("cmdEnterRest: track -1");
            return;
            }
      startCmd();
      expandVoice();
      if (_is.cr() == 0) {
            qDebug("cannot enter rest here");
            return;
            }

      int track = _is.track();
      NoteVal nval;
      setNoteRest(_is.segment(), track, nval, d.fraction(), MScore::Direction::AUTO);
      _is.moveToNextInputPos();
      _is.setRest(false);  // continue with normal note entry
      endCmd();
      }

//---------------------------------------------------------
//   removeChordRest
//    remove chord or rest
//    remove associated segment if empty
//    remove beam
//    remove slurs
//---------------------------------------------------------

void Score::removeChordRest(ChordRest* cr, bool clearSegment)
      {
      QList<Segment*> segments;
      for (Element* e : cr->linkList()) {
            undo(new RemoveElement(e));
            if (clearSegment) {
                  Segment* s = cr->segment();
                  if (!segments.contains(s))
                        segments.append(s);
                  }
            }
      for (Segment* s : segments) {
            if (s->isEmpty())
                  undo(new RemoveElement(s));
            }
      if (cr->beam()) {
            Beam* beam = cr->beam();
            if (beam->generated()) {
                  beam->parent()->remove(beam);
                  delete beam;
                  }
            else
                  undoRemoveElement(beam);
            }
      }

//---------------------------------------------------------
//   cmdDeleteTuplet
//    remove tuplet and replace with rest
//---------------------------------------------------------

void Score::cmdDeleteTuplet(Tuplet* tuplet, bool replaceWithRest)
      {
      qDebug("Score::cmdDeleteTuplet elements %d  replace %d",
         tuplet->elements().size(), replaceWithRest);

      foreach(DurationElement* de, tuplet->elements()) {
            qDebug("   element %s", de->name());
            if (de->isChordRest())
                  removeChordRest(static_cast<ChordRest*>(de), true);
            else {
                  Q_ASSERT(de->type() == Element::Type::TUPLET);
                  cmdDeleteTuplet(static_cast<Tuplet*>(de), false);
                  }
            }
//      undoRemoveElement(tuplet);
      if (replaceWithRest) {
            Rest* rest = setRest(tuplet->tick(), tuplet->track(), tuplet->duration(), true, 0);
            if (tuplet->tuplet()) {
                  rest->setTuplet(tuplet->tuplet());
                  tuplet->tuplet()->add(rest);
                  }
            }
      }

//---------------------------------------------------------
//   nextInputPos
//---------------------------------------------------------

void Score::nextInputPos(ChordRest* cr, bool doSelect)
      {
      ChordRest* ncr = nextChordRest(cr);
      if ((ncr == 0) && (_is.track() % VOICES)) {
            Segment* s = tick2segment(cr->tick() + cr->actualTicks(), false, Segment::Type::ChordRest);
            int track = (cr->track() / VOICES) * VOICES;
            ncr = s ? static_cast<ChordRest*>(s->element(track)) : 0;
            }
      if (ncr) {
            _is.setSegment(ncr->segment());
            if (doSelect)
                  select(ncr, SelectType::SINGLE, 0);
            setPlayPos(ncr->tick());
            for (MuseScoreView* v : viewer)
                  v->moveCursor();
            }
      }

//---------------------------------------------------------
//   searchMeasureBase
//    search corresponding measure in linked score
//---------------------------------------------------------

static MeasureBase* searchMeasureBase(Score* score, MeasureBase* mb)
      {
      if (mb == 0)
            return nullptr;
      if (mb->type() == Element::Type::MEASURE) {
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  if (m->tick() == mb->tick())
                        return m;
                  }
            }
      else {
            if (!mb->links())
                  qDebug("searchMeasureBase: no links");
            else {
                  for (Element* m : *mb->links()) {
                        if (m->score() == score)
                              return static_cast<MeasureBase*>(m);
                        }
                  }
            }
      qDebug("searchMeasureBase: measure not found");
      return nullptr;
      }

//---------------------------------------------------------
//   insertMeasure
//    Create a new MeasureBase of type type and insert
//    before measure.
//    If measure is zero, append new MeasureBase.
//---------------------------------------------------------

MeasureBase* Score::insertMeasure(Element::Type type, MeasureBase* measure, bool createEmptyMeasures)
      {
      int tick;
      int ticks = 0;
      if (measure) {
            if (measure->type() == Element::Type::MEASURE && static_cast<Measure*>(measure)->isMMRest()) {
                  measure = static_cast<Measure*>(measure)->prev();
                  measure = measure ? measure->next() : firstMeasure();
                  deselectAll();
                  }
            tick = measure->tick();
            }
      else
            tick = last() ? last()->endTick() : 0;

      // use nominal time signature of previous measure
      Fraction f = sigmap()->timesig(tick - 1).nominal();

      QList<pair<Score*, MeasureBase*>> ml;
      for (Score* score : scoreList())
            ml.append(pair<Score*,MeasureBase*>(score, searchMeasureBase(score, measure)));

      MeasureBase* omb = nullptr;   // measure base in "this" score
      MeasureBase* rmb = nullptr;   // measure base in root score (for linking)

      for (pair<Score*, MeasureBase*> p : ml) {
            Score* score    = p.first;
            MeasureBase* im = p.second;
            MeasureBase* mb = static_cast<MeasureBase*>(Element::create(type, score));
            mb->setTick(tick);

            if (score == this)
                  omb = mb;

            if (type == Element::Type::MEASURE) {
                  if (score == rootScore())
                        omb = static_cast<Measure*>(mb);
                  bool createEndBar    = false;
                  if (!measure) {
                        Measure* lm = score->lastMeasure();
                        if (lm && lm->endBarLineType() == BarLineType::END) {
                              createEndBar = true;
                              score->undoChangeEndBarLineType(lm, BarLineType::NORMAL);
                              }
                        else if (lm == nullptr)
                              createEndBar = true;
                        }

                  Measure* m = static_cast<Measure*>(mb);
                  m->setTimesig(f);
                  m->setLen(f);
                  ticks = m->ticks();

                  QList<TimeSig*> tsl;
                  QList<KeySig*>  ksl;
                  QList<Clef*>    cl;

                  if (tick == 0) {
                        //
                        // remove time and key signatures
                        //
                        for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                              for (Segment* s = score->firstSegment(); s && s->tick() == 0; s = s->next()) {
                                    Element* e = s->element(staffIdx * VOICES);
                                    if (e == nullptr)
                                          continue;
                                    if (e->type() == Element::Type::KEYSIG) {
                                          KeySig* ks = static_cast<KeySig*>(e);
                                          ksl.append(ks);
                                          undo(new RemoveElement(ks));
                                          if (ks->segment()->isEmpty())
                                                undoRemoveElement(ks->segment());
                                          }
                                    else if (e->type() == Element::Type::TIMESIG) {
                                          TimeSig* ts = static_cast<TimeSig*>(e);
                                          tsl.append(ts);
                                          undo(new RemoveElement(ts));
                                          if (ts->segment()->isEmpty())
                                                undoRemoveElement(ts->segment());
                                          }
                                    else if (e->type() == Element::Type::CLEF) {
                                          Clef* clef = static_cast<Clef*>(e);
                                          cl.append(clef);
                                          undo(new RemoveElement(e));
                                          if (clef->segment()->isEmpty())
                                                undoRemoveElement(clef->segment());
                                          }
                                    }
                              }
                        }

                  undo(new InsertMeasure(m, im));

                  //
                  // if measure is inserted at tick zero,
                  // create key and time signature
                  //
                  foreach(TimeSig* ts, tsl) {
                        TimeSig* nts = new TimeSig(*ts);
                        Segment* s   = m->undoGetSegment(Segment::Type::TimeSig, 0);
                        nts->setParent(s);
                        undoAddElement(nts);
                        }
                  foreach(KeySig* ks, ksl) {
                        KeySig* nks = new KeySig(*ks);
                        Segment* s  = m->undoGetSegment(Segment::Type::KeySig, 0);
                        nks->setParent(s);
                        undoAddElement(nks);
                        }
                  foreach(Clef* clef, cl) {
                        Clef* nClef = new Clef(*clef);
                        Segment* s  = m->undoGetSegment(Segment::Type::Clef, 0);
                        nClef->setParent(s);
                        undoAddElement(nClef);
                        }
                  if (createEndBar) {
                        m->setEndBarLineType(BarLineType::END, false);
                        }
                  score->fixTicks();
                  }
            else {
                  // a frame, not a measure
                  if (score == rootScore())
                        rmb = mb;
                  else if (rmb && mb != rmb)
                        mb->linkTo(rmb);
                  undo(new InsertMeasure(mb, im));
                  }
            }
      undoInsertTime(tick, ticks);
      undo(new InsertTime(this, tick, ticks));

      if (omb && type == Element::Type::MEASURE && !createEmptyMeasures) {
            //
            // fill measure with rest
            //
            Score* _root = rootScore();
            for (int staffIdx = 0; staffIdx < _root->nstaves(); ++staffIdx) {
                  int track = staffIdx * VOICES;
                  int tick = omb->tick();
                  Segment* s = static_cast<Measure*>(omb)->findSegment(Segment::Type::ChordRest, tick);
                  if (s == 0 || s->element(track) == 0) {
                        // add rest to this staff and to all the staves linked to it
                        Rest* rest = new Rest(_root, TDuration(TDuration::DurationType::V_MEASURE));
                        Fraction timeStretch(_root->staff(staffIdx)->timeStretch(tick));
                        rest->setDuration(static_cast<Measure*>(omb)->len() / timeStretch);
                        rest->setTrack(track);
                        undoAddCR(rest, static_cast<Measure*>(omb), tick);
                        }
                  }
            }

      return omb;
      }

//---------------------------------------------------------
//   checkSpanner
//    check if spanners are still valid as anchors may
//    have changed or be removed.
//    Spanners need to have a start anchor. Slurs need a
//    start and end anchor.
//---------------------------------------------------------

void Score::checkSpanner(int startTick, int endTick)
      {
      QList<Spanner*> sl;
      auto spanners = _spanner.findOverlapping(startTick, endTick);
// printf("checkSpanner %d %d\n", startTick, endTick);
      for (auto i = spanners.begin(); i < spanners.end(); i++) {
            Spanner* s = i->value;

//            printf("   %s %d %d\n", s->name(), s->tick(), s->tick2());

            if (s->type() == Element::Type::SLUR) {
                  Segment* seg = tick2segmentMM(s->tick(), false, Segment::Type::ChordRest);
                  if (!seg || !seg->element(s->track()))
                        sl.append(s);
                  seg = tick2segmentMM(s->tick2(), false, Segment::Type::ChordRest);
                  if (!seg || !seg->element(s->track2()))
                        sl.append(s);
                  }
            else {
                  // remove spanner if there is no start element
                  s->computeStartElement();
                  if (!s->startElement())
                        sl.append(s);
                  }
            }
      for (auto s : sl)       // actually remove scheduled spanners
            undo(new RemoveElement(s));
      }
}

