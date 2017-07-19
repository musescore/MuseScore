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
#include "undo.h"
#include "tupletmap.h"
#include "tiemap.h"
#include "stem.h"
#include "iname.h"
#include "range.h"
#include "hook.h"
#include "repeat.h"
#include "textframe.h"
#include "accidental.h"
#include "ottava.h"
#include "instrchange.h"
#include "bracket.h"
#include "excerpt.h"
#include "breath.h"
#include "glissando.h"

namespace Ms {

//---------------------------------------------------------
//   getSelectedNote
//---------------------------------------------------------

Note* Score::getSelectedNote()
      {
      Element* el = selection().element();
      if (el && el->isNote())
            return toNote(el);
      MScore::setError(NO_NOTE_SELECTED);
      return 0;
      }

//---------------------------------------------------------
//   getSelectedChordRest
//---------------------------------------------------------

ChordRest* Score::getSelectedChordRest() const
      {
      Element* el = selection().element();
      if (el) {
            if (el->isNote())
                  return toNote(el)->chord();
            else if (el->isRest() || el->isRepeatMeasure())
                  return toRest(el);
            else if (el->isChord())
                  return toChord(el);
            }
      MScore::setError(NO_NOTE_REST_SELECTED);
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
            if (e->isNote())
                  e = e->parent();
            if (e->isChordRest()) {
                  ChordRest* cr = toChordRest(e);
                  if (*cr1 == 0 || (*cr1)->tick() > cr->tick())
                        *cr1 = cr;
                  if (*cr2 == 0 || (*cr2)->tick() < cr->tick())
                        *cr2 = cr;
                  }
            }
      if (*cr1 == 0)
            MScore::setError(NO_NOTE_REST_SELECTED);
      if (*cr1 == *cr2)
            *cr2 = 0;
      }

//---------------------------------------------------------
//   getSelectedChordRests
//---------------------------------------------------------

QSet<ChordRest*> Score::getSelectedChordRests() const
      {
      QSet<ChordRest*> set;
      for (Element* e : selection().elements()) {
            if (e->isNote())
                  e = e->parent();
            if (e->isChordRest()) {
                  set.insert(static_cast<ChordRest*>(e));
                  }
            }
      return set;
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
            switch (el->type()) {
                  case ElementType::NOTE:
                        el = el->parent();
                        // fall through
                  case ElementType::REPEAT_MEASURE:
                  case ElementType::REST:
                  case ElementType::CHORD:
                        return toChordRest(el)->tick();
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
            nn->setPitch(n->pitch());
            nn->setTpc1(n->tpc1());
            nn->setTpc2(n->tpc2());
            chord->add(nn);
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

      return chord;
      }

//---------------------------------------------------------
//   addClone
//---------------------------------------------------------

ChordRest* Score::addClone(ChordRest* cr, int tick, const TDuration& d)
      {
      ChordRest* newcr;
      // change a RepeatMeasure() into an Rest()
      if (cr->isRepeatMeasure())
            newcr = new Rest(*toRest(cr));
      else
            newcr = toChordRest(cr->clone());
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
               && !tuplet
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
                  std::vector<TDuration> dList = toDurationList(f, useDots);
                  if (dList.empty())
                        return 0;

                  Rest* rest = 0;
                  if (((tick - measure->tick()) % dList[0].ticks()) == 0) {
                        for (const TDuration& d : dList) {
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
      setPlayNote(true);
      setPlayChord(true);
      select(note, SelectType::SINGLE, 0);
      if (!chord->staff()->isTabStaff(chord->tick())) {
            NoteEntryMethod entryMethod = _is.noteEntryMethod();
            if (entryMethod != NoteEntryMethod::REALTIME_AUTO && entryMethod != NoteEntryMethod::REALTIME_MANUAL)
                  _is.moveToNextInputPos();
            }
      return note;
      }

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures from fm to lm (including)
//    If staffIdx is valid (>= 0), then rewrite a local
//    timesig change.
//---------------------------------------------------------

bool Score::rewriteMeasures(Measure* fm, Measure* lm, const Fraction& ns, int staffIdx)
      {
      if (staffIdx >= 0) {
            // local timesig
            // don't actually rewrite, just update measure rest durations
            // abort if there is anything other than measure rests in range
            int strack = staffIdx * VOICES;
            int etrack = strack + VOICES;
            for (Measure* m = fm; ; m = m->nextMeasure()) {
                  for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                        for (int track = strack; track < etrack; ++track) {
                              ChordRest* cr = toChordRest(s->element(track));
                              if (!cr)
                                    continue;
                              if (cr->isRest() && cr->durationType() == TDuration::DurationType::V_MEASURE)
                                    cr->undoChangeProperty(P_ID::DURATION, QVariant::fromValue(ns));
                              else
                                    return false;
                              }
                        }
                  if (m == lm)
                        break;
                  }
            return true;
            }
      int measures = 1;
      bool fmr = true;
      for (Measure* m = fm; m; m = m->nextMeasure()) {
            if (!m->isFullMeasureRest())
                  fmr = false;
            if (m == lm)
                  break;
            ++measures;
            }

      if (!fmr) {
            // check for local time signatures
            for (Measure* m = fm; m; m = m -> nextMeasure()) {
                  for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                        if (staff(staffIdx)->timeStretch(m->tick()) != Fraction(1,1)) {
                              // we cannot change a staff with a local time signature
                              return false;
                              }
                        if (m == lm)
                              break;
                        }
                  }
            }

      ScoreRange range;
      range.read(fm->first(), lm->last());

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

            int tick1 = m1->tick();
            int tick2 = m2->endTick();
            auto spanners = s->spannerMap().findContained(tick1, tick2);
            for (auto i : spanners)
                  undo(new RemoveElement(i.value));
            s->undoRemoveMeasures(m1, m2);

            Measure* nfm = 0;
            Measure* nlm = 0;
            int tick     = 0;
//            bool endBarGenerated = m1->endBarLineGenerated();
            for (int i = 0; i < nm; ++i) {
                  Measure* m = new Measure(s);
                  m->setPrev(nlm);
                  if (nlm)
                        nlm->setNext(m);
                  m->setTimesig(ns);
                  m->setLen(ns);
                  m->setTick(tick);
//                  m->setEndBarLineType(BarLineType::NORMAL, endBarGenerated);
                  tick += m->ticks();
                  nlm = m;
                  if (nfm == 0)
                        nfm = m;
                  }
//            nlm->setEndBarLineType(m2->endBarLineType(), m2->endBarLineGenerated(),
//               m2->endBarLineVisible(), m2->endBarLineColor());
            //
            // insert new calculated measures
            //
            nfm->setPrev(m1->prev());
            nlm->setNext(m2->next());
            s->undo(new InsertMeasures(nfm, nlm));
            }
      if (!fill.isZero())
            undoInsertTime(lm->endTick(), fill.ticks());

      if (!range.write(masterScore(), fm->tick()))
            return false;
      connectTies(true);

      if (noteEntryMode()) {
            // set input cursor to possibly re-written segment
            int icTick = inputPos();
            Segment* icSegment = tick2segment(icTick, false, SegmentType::ChordRest);
            if (!icSegment) {
                  // this can happen if cursor was on a rest
                  // and in the rewriting it got subsumed into a full measure rest
                  Measure* icMeasure = tick2measure(icTick);
                  if (!icMeasure)                     // shouldn't happen, but just in case
                        icMeasure = firstMeasure();
                  icSegment = icMeasure->first(SegmentType::ChordRest);
                  }
            inputState().setSegment(icSegment);
            }

      return true;
      }

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures up to the next time signature or section break
//---------------------------------------------------------

bool Score::rewriteMeasures(Measure* fm, const Fraction& ns, int staffIdx)
      {
      Measure* lm  = fm;
      Measure* fm1 = fm;
      Measure* nm  = nullptr;
      LayoutBreak* sectionBreak = nullptr;

      // disable local time sig modifications in linked staves
      if (staffIdx != -1 && excerpts().size() > 0) {
            MScore::setError(CANNOT_CHANGE_LOCAL_TIMESIG);
            return false;
            }

      //
      // split into Measure segments fm-lm
      //
      for (MeasureBase* m = fm; ; m = m->next()) {

            if (!m || !m->isMeasure() || lm->sectionBreak()
              || (toMeasure(m)->first(SegmentType::TimeSig) && m != fm))
                  {

                  // save section break to reinstate after rewrite
                  if (lm->sectionBreak())
                        sectionBreak = new LayoutBreak(*lm->sectionBreakElement());

                  if (!rewriteMeasures(fm1, lm, ns, staffIdx)) {
                        if (staffIdx >= 0) {
                              MScore::setError(CANNOT_CHANGE_LOCAL_TIMESIG);
                              // restore measure rests that were prematurely modified
                              Fraction fr(staff(staffIdx)->timeSig(fm->tick())->sig());
                              for (Measure* m = fm1; m; m = m->nextMeasure()) {
                                    ChordRest* cr = m->findChordRest(m->tick(), staffIdx * VOICES);
                                    if (cr && cr->isRest() && cr->durationType() == TDuration::DurationType::V_MEASURE)
                                          cr->undoChangeProperty(P_ID::DURATION, QVariant::fromValue(fr));
                                    else
                                          break;
                                    }
                              }
                        else {
                              // this can be hit for local time signatures as well
                              // (if we are rewriting all staves, but one has a local time signature)
                              // TODO: detect error conditions better, have clearer error messages
                              // and perform necessary fixups
                              MScore::setError(TUPLET_CROSSES_BAR);
                              }
                        for (Measure* m = fm1; m; m = m->nextMeasure()) {
                              if (m->first(SegmentType::TimeSig))
                                    break;
                              Fraction fr(ns);
                              undoChangeProperty(m, P_ID::TIMESIG_NOMINAL, QVariant::fromValue(fr));
                              }
                        return false;
                        }

                  // after rewrite, lm is not necessarily valid
                  // m is first MeasureBase after rewritten range
                  // m->prevMeasure () is new last measure of range
                  // set nm to first true Measure after rewritten range
                  // we may use this to reinstate time signatures
                  if (m && m->prevMeasure())
                        nm = m->prevMeasure()->nextMeasure();
                  else
                        nm = nullptr;

                  if (sectionBreak) {
                        // reinstate section break, then stop rewriting
                        if (m && m->prevMeasure()) {
                              sectionBreak->setParent(m->prevMeasure());
                              undoAddElement(sectionBreak);
                              }
                        else if (!m) {
                              sectionBreak->setParent(lastMeasure());
                              undoAddElement(sectionBreak);
                              }
                        else {
                              qDebug("unable to restore section break");
                              nm = nullptr;
                              sectionBreak = nullptr;
                              }
                        break;
                        }

                  // stop rewriting at end of score
                  // or at a measure (which means we found a time signature segment)
                  if (!m || m->isMeasure())
                        break;

                  // skip frames
                  while (!m->isMeasure()) {
                        if (m->sectionBreak()) {
                              // frame has a section break; we can stop skipping ahead
                              sectionBreak = m->sectionBreakElement();
                              break;
                              }
                        m = m->next();
                        if (!m)
                              break;
                        }
                  // stop rewriting if we encountered a section break on a frame
                  // or if there is a time signature on first measure after the frame
                  if (sectionBreak || (m && toMeasure(m)->first(SegmentType::TimeSig)))
                        break;

                  // set up for next range to rewrite
                  fm1 = toMeasure(m);
                  if (fm1 == 0)
                        break;
                  }

            // if we didn't break the loop already,
            // we must have an ordinary measure
            // add measure to range to rewrite
            lm = toMeasure(m);
            }

      // if any staves don't have time signatures at the point where we stopped,
      // we need to reinstate their previous time signatures
      if (!nm)
            return true;
      Segment* s = nm->undoGetSegment(SegmentType::TimeSig, nm->tick());
      for (int i = 0; i < nstaves(); ++i) {
            if (!s->element(i * VOICES)) {
                  TimeSig* ots = staff(i)->timeSig(nm->tick());
                  if (ots) {
                        TimeSig* nts = new TimeSig(*ots);
                        nts->setParent(s);
                        if (sectionBreak) {
                              nts->setGenerated(false);
                              nts->setShowCourtesySig(false);
                              }
                        undoAddElement(nts);
                        }
                  }
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
      deselectAll();

      Fraction ns  = ts->sig();
      int tick     = fm->tick();
      TimeSig* lts = staff(staffIdx)->timeSig(tick);
      if (local) {
            Fraction stretch = (ns / fm->timesig()).reduced();
            ts->setStretch(stretch);
            }

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
      Segment* seg = fm->undoGetSegment(SegmentType::TimeSig, tick);
      TimeSig* ots = toTimeSig(seg->element(track));

      if (ots && (*ots == *ts)) {
            //
            //  ignore if there is already a timesig
            //  with same values
            //
            delete ts;
            return;
            }

//      if (ots && ots->sig().identical(ns) && ots->stretch() == ts->stretch()) {
      if (ots && ots->sig() == ns && ots->stretch() == ts->stretch()) {
            //
            // the measure duration does not change,
            // so its ok to just update the time signatures
            //
            TimeSig* nts = staff(staffIdx)->nextTimeSig(tick+1);
            Measure* lm  = nts ? nts->segment()->measure() : 0;
            for (Score* score : scoreList()) {
                  Measure* fm = score->tick2measure(tick);
                  for (Measure* m = fm; m != lm; m = m->nextMeasure()) {
                        bool changeActual = m->len() == m->timesig();
                        undoChangeProperty(m, P_ID::TIMESIG_NOMINAL, QVariant::fromValue(ns));
                        if (changeActual)
                              undoChangeProperty(m, P_ID::TIMESIG_ACTUAL,  QVariant::fromValue(ns));
                        }
                  }
            int n = nstaves();
            for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
                  TimeSig* nsig = toTimeSig(seg->element(staffIdx * VOICES));
                  nsig->undoChangeProperty(P_ID::TIMESIG_TYPE,       int(ts->timeSigType()));
                  nsig->undoChangeProperty(P_ID::NUMERATOR_STRING,   ts->numeratorString());
                  nsig->undoChangeProperty(P_ID::DENOMINATOR_STRING, ts->denominatorString());
                  nsig->undoChangeProperty(P_ID::GROUPS,             QVariant::fromValue(ts->groups()));
                  }
            }
      else {
            Score* score = masterScore();
            Measure* fm  = score->tick2measure(tick);

            //
            // rewrite all measures up to the next time signature
            //
            if (fm == score->firstMeasure() && (fm->len() != fm->timesig())) {
                  // handle upbeat
                  undoChangeProperty(fm, P_ID::TIMESIG_NOMINAL, QVariant::fromValue(ns));
                  Measure* m = fm->nextMeasure();
                  Segment* s = m->findSegment(SegmentType::TimeSig, m->tick());
                  fm = s ? 0 : fm->nextMeasure();
                  }
            else {
                  if (sigmap()->timesig(seg->tick()).nominal().identical(ns)) {
                        // no change to global time signature,
                        // but we need to rewrite any staves with local time signatures
                        for (int i = 0; i < nstaves(); ++i) {
                              if (staff(i)->timeSig(tick) && staff(i)->timeSig(tick)->isLocal()) {
                                    if (!score->rewriteMeasures(fm, ns, i)) {
                                          undoStack()->current()->unwind();
                                          return;
                                          }
                                    }
                              }
                        fm = 0;
                        }
                  }

            // try to rewrite the measures first
            // we will only add time signatures if this succeeds
            // this means, however, that the rewrite cannot depend on the time signatures being in place
            if (fm) {
                  if (!score->rewriteMeasures(fm, ns, local ? staffIdx : -1)) {
                        undoStack()->current()->unwind();
                        return;
                        }
                  }

            // add the time signatures
            foreach (Score* score, scoreList()) {
                  Measure* nfm = score->tick2measure(tick);
                  seg   = nfm->undoGetSegment(SegmentType::TimeSig, nfm->tick());
                  int startStaffIdx, endStaffIdx;
                  if (local) {
                        if (score == this) {
                              startStaffIdx = staffIdx;
                              endStaffIdx   = startStaffIdx + 1;
                              }
                        else {
                              // TODO: get index for this score
                              qDebug("cmdAddTimeSig: unable to write local time signature change to linked score");
                              startStaffIdx = 0;
                              endStaffIdx   = 0;
                              }
                        }
                  else {
                        startStaffIdx = 0;
                        endStaffIdx   = score->nstaves();
                        }
                  for (int staffIdx = startStaffIdx; staffIdx < endStaffIdx; ++staffIdx) {
                        TimeSig* nsig = toTimeSig(seg->element(staffIdx * VOICES));
                        if (nsig == 0) {
                              nsig = new TimeSig(*ts);
                              nsig->setScore(score);
                              nsig->setTrack(staffIdx * VOICES);
                              nsig->setParent(seg);
                              undoAddElement(nsig);
                              }
                        else {
                              nsig->undoChangeProperty(P_ID::SHOW_COURTESY, ts->showCourtesySig());
                              nsig->undoChangeProperty(P_ID::TIMESIG_TYPE, int(ts->timeSigType()));
                              nsig->undoChangeProperty(P_ID::TIMESIG, QVariant::fromValue(ts->sig()));
                              nsig->undoChangeProperty(P_ID::NUMERATOR_STRING, ts->numeratorString());
                              nsig->undoChangeProperty(P_ID::DENOMINATOR_STRING, ts->denominatorString());

                              // HACK do it twice to accommodate undo
                              nsig->undoChangeProperty(P_ID::TIMESIG_TYPE, int(ts->timeSigType()));
                              nsig->undoChangeProperty(P_ID::TIMESIG_STRETCH, QVariant::fromValue(ts->stretch()));
                              nsig->undoChangeProperty(P_ID::GROUPS,  QVariant::fromValue(ts->groups()));
                              nsig->setSelected(false);
                              nsig->setDropTarget(0);       // DEBUG
                              }
                        }
                  }
            }
      delete ts;
      }

//---------------------------------------------------------
//   cmdRemoveTimeSig
//---------------------------------------------------------

void Score::cmdRemoveTimeSig(TimeSig* ts)
      {
      if (ts->isLocal() && excerpts().size() > 0) {
            MScore::setError(CANNOT_CHANGE_LOCAL_TIMESIG);
            return;
            }

      Measure* m = ts->measure();
      Segment* s = ts->segment();

      //
      // we cannot remove a courtesy time signature
      //
      if (m->tick() != s->tick())
            return;
      int tick = m->tick();

      // if we remove all time sigs from segment, segment will be already removed by now
      // but this would leave us no means of detecting that we have have measures in a local timesig
      // in cases where we try deleting the local time sig
      // known bug: this means we do not correctly detect non-empty measures when deleting global timesig change after a local one
      // see http://musescore.org/en/node/51596
      // Delete the time sig segment from the root score, we will rewriteMeasures from it
      // since it contains all the music while the part doesn't
      Score* rScore = masterScore();
      Measure* rm = rScore->tick2measure(m->tick());
      Segment* rs = rm->findSegment(SegmentType::TimeSig, s->tick());
      rScore->undoRemoveElement(rs);

      Measure* pm = m->prevMeasure();
      Fraction ns(pm ? pm->timesig() : Fraction(4,4));

      if (!rScore->rewriteMeasures(rm, ns, -1)) {
            undoStack()->current()->unwind();
            }
      else {
            m = tick2measure(tick);       // old m may have been replaced
            // hack: fix measure rest durations for staves with local time signatures
            // if a time signature was deleted to reveal a previous local one,
            // then rewriteMeasures() got the measure rest durations wrong
            // (if we fixed it to work for delete, it would fail for add)
            // so we will fix measure rest durations here
            // TODO: fix rewriteMeasures() to get this right
            for (int i = 0; i < nstaves(); ++i) {
                  TimeSig* ts = staff(i)->timeSig(tick);
                  if (ts && ts->isLocal()) {
                        for (Measure* nm = m; nm; nm = nm->nextMeasure()) {
                              // stop when time signature changes
                              if (staff(i)->timeSig(nm->tick()) != ts)
                                    break;
                              // fix measure rest duration
                              ChordRest* cr = nm->findChordRest(nm->tick(), i * VOICES);
                              if (cr && cr->isRest() && cr->durationType() == TDuration::DurationType::V_MEASURE)
                                    cr->undoChangeProperty(P_ID::DURATION, QVariant::fromValue(nm->stretchedLen(staff(i))));
                                    //cr->setDuration(nm->stretchedLen(staff(i)));
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//  addTiedMidiPitch
//---------------------------------------------------------

Note* Score::addTiedMidiPitch(int pitch, bool addFlag, Chord* prevChord)
      {
      Note* n = addMidiPitch(pitch, addFlag);
      if (prevChord) {
            Note* nn = prevChord->findNote(n->pitch());
            if (nn) {
                  Tie* tie = new Tie(this);
                  tie->setStartNote(nn);
                  tie->setEndNote(n);
                  tie->setTrack(n->track());
                  n->setTieBack(tie);
                  nn->setTieFor(tie);
                  undoAddElement(tie);
                  }
            }
      return n;
      }

//---------------------------------------------------------
//  addMidiPitch
//---------------------------------------------------------

Note* Score::addMidiPitch(int pitch, bool addFlag)
      {
      NoteVal nval(pitch);
      Staff* st = staff(inputState().track() / VOICES);

      // if transposing, interpret MIDI pitch as representing desired written pitch
      // set pitch based on corresponding sounding pitch
      if (!styleB(StyleIdx::concertPitch))
            nval.pitch += st->part()->instrument(inputState().tick())->transpose().chromatic;
      // let addPitch calculate tpc values from pitch
      //Key key   = st->key(inputState().tick());
      //nval.tpc1 = pitch2tpc(nval.pitch, key, Prefer::NEAREST);
      return addPitch(nval, addFlag);
      }

//---------------------------------------------------------
//   searchNote
//    search for note or rest before or at tick position tick
//    in staff
//---------------------------------------------------------

ChordRest* Score::searchNote(int tick, int track) const
      {
      ChordRest* ipe = 0;
      SegmentType st = SegmentType::ChordRest;
      for (Segment* segment = firstSegment(st); segment; segment = segment->next1(st)) {
            ChordRest* cr = segment->cr(track);
            if (!cr)
                  continue;
            if (cr->tick() == tick)
                  return cr;
            if (cr->tick() >  tick)
                  return ipe ? ipe : cr;
            ipe = cr;
            }
      return 0;
      }

//---------------------------------------------------------
//   regroupNotesAndRests
//    * combine consecutive rests into fewer rests of longer duration.
//    * combine tied notes/chords into fewer notes of longer duration.
//    Only operates on one voice - protects manual layout adjustment, etc.
//---------------------------------------------------------

void Score::regroupNotesAndRests(int startTick, int endTick, int track)
      {
      Segment* inputSegment = _is.segment(); // store this so we can get back to it later.
      Segment* seg = tick2segment(startTick, true, SegmentType::ChordRest);
      for (Measure* msr = seg->measure(); msr && msr->tick() < endTick; msr = msr->nextMeasure()) {
            int maxTick = endTick > msr->endTick() ? msr->endTick() : endTick;
            if (!seg || seg->measure() != msr)
                  seg = msr->first(SegmentType::ChordRest);
            for (; seg && seg->tick() + seg->ticks() < maxTick; seg = seg->next(SegmentType::ChordRest)) {
                  ChordRest* curr = seg->cr(track);
                  if (!curr)
                        continue; // voice is empty here
                  if (curr->isRest()) {
                        // combine consecutive rests
                        ChordRest* lastRest = curr;
                        for (Segment* s = seg->next(SegmentType::ChordRest); s && s->tick() + s->ticks() <= maxTick; s = s->next(SegmentType::ChordRest)) {
                              ChordRest* cr = s->cr(track);
                              if (!cr || !cr->isRest())
                                    break;
                              lastRest = cr;
                              }
                        int restTicks = lastRest->tick() + lastRest->duration().ticks() - curr->tick();
                        seg = setNoteRest(seg, curr->track(), NoteVal(), Fraction::fromTicks(restTicks), Direction::AUTO);
                        }
                  else {
                        // combine tied chords
                        Chord* chord = toChord(curr);
                        Chord* lastTiedChord = chord;
                        for (Chord* next = chord->nextTiedChord(); next && next->tick() + next->duration().ticks() <= maxTick; next = next->nextTiedChord()) {
                              lastTiedChord = next;
                              }
                        if (!lastTiedChord)
                              lastTiedChord = chord;
                        int numNotes = chord->notes().size();
                        int pitches[numNotes];
                        Tie* tieBack[numNotes];
                        Tie* tieFor[numNotes];
                        for (int i = 0; i < numNotes; i++) {
                              Note* n = chord->notes()[i];
                              Note* nn = lastTiedChord->notes()[i];
                              pitches[i] = n->pitch();
                              tieBack[i] = n->tieBack();
                              tieFor[i] = nn->tieFor();
                              n->setTieBack(0);
                              nn->setTieFor(0);
                              }
                        int noteTicks = lastTiedChord->tick() + lastTiedChord->duration().ticks() - chord->tick();
                        seg = setNoteRest(seg, curr->track(), NoteVal(pitches[0]), Fraction::fromTicks(noteTicks), Direction::AUTO);
                        Chord* newChord = toChord(seg->cr(track));
                        Note* n = newChord->notes().front();
                        undoRemoveElement(n);
                        for (int i = 0; i < numNotes; i++) {
                              NoteVal nval = NoteVal(pitches[i]);
                              n = addNote(newChord, nval);
                              if (tieBack[i]) {
                                    n->setTieBack(tieBack[i]);
                                    tieBack[i]->setEndNote(n);
                                    }
                              if (tieFor[i]) {
                                    n->setTieFor(tieFor[i]);
                                    tieFor[i]->setStartNote(n);
                                    }
                              }
                        }
                  }
            }
      // now put the input state back where it was before
      _is.setSegment(inputSegment);
      }

//---------------------------------------------------------
//   cmdAddTie
//---------------------------------------------------------

void Score::cmdAddTie()
      {
      std::vector<Note*> noteList;
      Element* el = selection().element();
      if (el && el->isNote()) {
            Note* n = toNote(el);
            if (noteEntryMode())
                  noteList = n->chord()->notes();
            else
                  noteList.push_back(n);
            }
      else if (el && el->isStem()) {
            Chord* chord = toStem(el)->chord();
            noteList = chord->notes();
            }
      else
            noteList = selection().noteList();
      if (noteList.empty()) {
            qDebug("no notes selected");
            return;
            }

      startCmd();
      Chord* lastAddedChord = nullptr;
      for (Note* note : noteList) {
            if (note->tieFor()) {
                  qDebug("cmdAddTie: note %p has already tie? noteFor: %p", note, note->tieFor());
                  continue;
                  }

            if (noteEntryMode()) {
                  // set cursor at position after note
                  _is.setSegment(note->chord()->segment());
                  _is.moveToNextInputPos();
                  _is.setLastSegment(_is.segment());

                  if (_is.cr() == 0)
                        expandVoice();
                  ChordRest* cr = _is.cr();
                  if (cr == 0)
                        break;

                  bool addFlag = lastAddedChord != nullptr;

                  // try to re-use existing note or chord
                  Note* n = nullptr;
                  if (cr->isChord()) {
                        Chord* c = toChord(cr);
                        Note* nn = c->findNote(note->pitch());
                        if (nn && nn->tpc() == note->tpc())
                              n = nn;           // re-use note
                        else
                              addFlag = true;   // re-use chord
                        }

                  // if no note to re-use, create one
                  NoteVal nval(note->noteVal());
                  if (!n)
                        n = addPitch(nval, addFlag);
                  else
                        select(n);

                  if (n) {
                        if (!lastAddedChord)
                              lastAddedChord = n->chord();
                        // n is not necessarily next note if duration span over measure
                        Note* nnote = searchTieNote(note);
                        while (nnote) {
                              // DEBUG: if duration spans over measure
                              // this does not set line for intermediate notes
                              // tpc was set correctly already
                              //n->setLine(note->line());
                              //n->setTpc(note->tpc());
                              Tie* tie = new Tie(this);
                              tie->setStartNote(note);
                              tie->setEndNote(nnote);
                              tie->setTrack(note->track());
                              undoAddElement(tie);
                              if (!addFlag || nnote->chord() == lastAddedChord) {
                                    break;
                                    }
                              else {
                                    note = nnote;
                                    _is.setLastSegment(_is.segment());
                                    nnote = addPitch(nval, true);
                                    }
                              }
                        }
                  }
            else {
                  Note* note2 = searchTieNote(note);
#if 0
                  // this code appears to mostly duplicate searchTieNote()
                  // not clear if it served any additional purpose
                  // it might have before we supported extended ties?
                  Part* part = chord->part();
                  int strack = part->staves()->front()->idx() * VOICES;
                  int etrack = strack + part->staves()->size() * VOICES;

                  for (Segment* seg = chord->segment()->next1(SegmentType::ChordRest); seg; seg = seg->next1(SegmentType::ChordRest)) {
                        bool noteFound = false;
                        for (int track = strack; track < etrack; ++track) {
                              ChordRest* cr = toChordRest(seg->element(track));
                              if (cr == 0 || !cr->isChord())
                                    continue;
                              int staffIdx = cr->staffIdx() + cr->staffMove();
                              if (staffIdx != chord->staffIdx() + chord->staffMove())
                                    continue;
                              foreach(Note* n, toChord(cr)->notes()) {
                                    if (n->pitch() == note->pitch()) {
                                          if (note2 == 0 || note->chord()->track() == chord->track())
                                                note2 = n;
                                          }
                                    // this may belong to *previous* if?
                                    else if (cr->track() == chord->track())
                                          noteFound = true;
                                    }
                              }
                        if (noteFound || note2)
                              break;
                        }
#endif
                  if (note2) {
                        Tie* tie = new Tie(this);
                        tie->setStartNote(note);
                        tie->setEndNote(note2);
                        tie->setTrack(note->track());
                        undoAddElement(tie);
                        }
                  }
            }
      if (lastAddedChord)
            nextInputPos(lastAddedChord, false);
      endCmd();
      }

//---------------------------------------------------------
//   cmdAddOttava
//---------------------------------------------------------

void Score::cmdAddOttava(OttavaType type)
      {
      Selection sel = selection();
      ChordRest* cr1;
      ChordRest* cr2;
      // add on each staff if possible
      if (sel.isRange() && sel.staffStart() != sel.staffEnd() - 1) {
            for (int staffIdx = sel.staffStart() ; staffIdx < sel.staffEnd(); ++staffIdx) {
                  ChordRest* cr1 = sel.firstChordRest(staffIdx * VOICES);
                  ChordRest* cr2 = sel.lastChordRest(staffIdx * VOICES);
                  if (!cr1)
                       continue;
                  if (cr2 == 0)
                       cr2 = cr1;
                  Ottava* ottava = new Ottava(this);
                  ottava->setOttavaType(type);
                  ottava->setTrack(cr1->track());
                  ottava->setTrack2(cr1->track());
                  ottava->setTick(cr1->tick());
                  ottava->setTick2(cr2->tick() + cr2->actualTicks());
                  undoAddElement(ottava);
                  }
            }
      else {
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
      }

//---------------------------------------------------------
//   cmdSetBeamMode
//---------------------------------------------------------

void Score::cmdSetBeamMode(Beam::Mode mode)
      {
      for (ChordRest* cr : getSelectedChordRests()) {
            if (cr)
                  undoChangeProperty(cr, P_ID::BEAM_MODE, int(mode));
            }
      }

//---------------------------------------------------------
//   cmdFlip
//---------------------------------------------------------

void Score::cmdFlip()
      {
      const QList<Element*>& el = selection().elements();
      if (el.empty()) {
            MScore::setError(NO_NOTE_SLUR_SELECTED);
            return;
            }
      for (Element* e : el) {
            if (e->isNote() || e->isStem() || e->isHook()) {
                  Chord* chord;
                  if (e->isNote())
                        chord = toNote(e)->chord();
                  else if (e->isStem())
                        chord = toStem(e)->chord();
                  else
                        chord = toHook(e)->chord();
                  if (chord->beam())
                        if (!selection().isRange())
                              e = chord->beam();  // fall through
                        else
                              continue;
                  else {
                        Direction dir = chord->up() ? Direction::DOWN : Direction::UP;
                        undoChangeProperty(chord, P_ID::STEM_DIRECTION, QVariant::fromValue<Direction>(dir));
                        }
                  }

            else if (e->isBeam()) {
                  Beam* beam = toBeam(e);
                  Direction dir = beam->up() ? Direction::DOWN : Direction::UP;
                  undoChangeProperty(beam, P_ID::STEM_DIRECTION, QVariant::fromValue<Direction>(dir));
                  }
            else if (e->isSlurTieSegment()) {
                  SlurTie* slur = toSlurTieSegment(e)->slurTie();
                  Direction dir = slur->up() ? Direction::DOWN : Direction::UP;
                  undoChangeProperty(slur, P_ID::SLUR_DIRECTION, QVariant::fromValue<Direction>(dir));
                  }
            else if (e->isHairpinSegment()) {
                  Hairpin* h = toHairpinSegment(e)->hairpin();
                  HairpinType st = h->hairpinType();
                  switch (st)  {
                        case HairpinType::CRESC_HAIRPIN:     st = HairpinType::DECRESC_HAIRPIN; break;
                        case HairpinType::DECRESC_HAIRPIN:   st = HairpinType::CRESC_HAIRPIN; break;
                        case HairpinType::CRESC_LINE:        st = HairpinType::DECRESC_LINE; break;
                        case HairpinType::DECRESC_LINE:      st = HairpinType::CRESC_LINE; break;
                        }
                  undoChangeProperty(h, P_ID::HAIRPIN_TYPE, int(st));
                  }
            else if (e->isArticulation()) {
                  Articulation* a = toArticulation(e);
                  ArticulationAnchor aa = a->anchor();
                  switch (aa) {
                        case ArticulationAnchor::TOP_CHORD:
                              aa = ArticulationAnchor::BOTTOM_CHORD;
                              break;
                        case ArticulationAnchor::BOTTOM_CHORD:
                              aa = ArticulationAnchor::TOP_CHORD;
                              break;
                        case ArticulationAnchor::CHORD:
                              aa = a->up() ? ArticulationAnchor::BOTTOM_CHORD : ArticulationAnchor::TOP_CHORD;
                              break;
                        case ArticulationAnchor::TOP_STAFF:
                              aa = ArticulationAnchor::BOTTOM_STAFF;
                              break;
                        case ArticulationAnchor::BOTTOM_STAFF:
                              aa = ArticulationAnchor::TOP_STAFF;
                              break;
                        }
                  // undoChangeProperty(a, P_ID::DIRECTION, a->up() ? Direction::DOWN : Direction::UP);
                  undoChangeProperty(a, P_ID::ARTICULATION_ANCHOR, int(aa));
                  }
            else if (e->isTuplet()) {
                  Tuplet* tuplet = toTuplet(e);
                  Direction d = tuplet->isUp() ? Direction::DOWN : Direction::UP;
                  undoChangeProperty(tuplet, P_ID::DIRECTION, QVariant::fromValue<Direction>(d));
                  }
            else if (e->isNoteDot()) {
                  Note* note = toNote(e->parent());
                  Direction d = note->dotIsUp() ? Direction::DOWN : Direction::UP;
                  undoChangeProperty(note, P_ID::DOT_POSITION, QVariant::fromValue<Direction>(d));
                  }
            else if (e->isTempoText()
               || e->isStaffText()
               || e->isDynamic()
               || e->isHairpin()
               || e->isOttavaSegment()
               || e->isTextLineSegment()
               || e->isPedalSegment()
               || e->isTrillSegment()) {
                  // getProperty() delegates call from spannerSegment to Spanner:
                  Element::Placement p = Element::Placement(e->getProperty(P_ID::PLACEMENT).toInt());
                  p = (p == Element::Placement::ABOVE) ? Element::Placement::BELOW : Element::Placement::ABOVE;
                  e->undoChangeProperty(P_ID::AUTOPLACE, true);
                  e->undoChangeProperty(P_ID::PLACEMENT, int(p));
                  }
            else if (e->isLyrics()) {
                  toLyrics(e)->chordRest()->flipLyrics(toLyrics(e));
                  }
            }
      }

//---------------------------------------------------------
//   deleteItem
//---------------------------------------------------------

void Score::deleteItem(Element* el)
      {
      if (!el)
            return;
//      qDebug("%s", el->name());

      switch (el->type()) {
            case ElementType::INSTRUMENT_NAME: {
                  Part* part = el->part();
                  InstrumentName* in = toInstrumentName(el);
                  if (in->instrumentNameType() == InstrumentNameType::LONG)
                        undo(new ChangeInstrumentLong(0, part, QList<StaffName>()));
                  else if (in->instrumentNameType() == InstrumentNameType::SHORT)
                        undo(new ChangeInstrumentShort(0, part, QList<StaffName>()));
                  }
                  break;

            case ElementType::TIMESIG: {
                  // timesig might already be removed
                  TimeSig* ts = toTimeSig(el);
                  Segment* s = ts->segment();
                  Measure* m = s->measure();
                  Segment* ns = m->findSegment(s->segmentType(), s->tick());
                  if (!ns || (ns->element(ts->track()) != ts)) {
                        qDebug("deleteItem: not found");
                        break;
                        }
                  cmdRemoveTimeSig(ts);
                  }
                  break;

            case ElementType::KEYSIG:
                  undoRemoveElement(el);
                  break;

            case ElementType::NOTE:
                  {
                  Chord* chord = toChord(el->parent());
                  if (chord->notes().size() > 1) {
                        undoRemoveElement(el);
                        select(chord->downNote(), SelectType::SINGLE, 0);
                        break;
                        }
                  // else fall through
                  el = chord;
                  }
                  // fall through

            case ElementType::CHORD:
                  {
                  Chord* chord = toChord(el);
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
                              QList<ScoreElement*> tl = tuplet->linkList();
                              for (ScoreElement* e : rest->linkList()) {
                                    DurationElement* de = static_cast<DurationElement*>(e);
                                    for (ScoreElement* ee : tl) {
                                          Tuplet* t = static_cast<Tuplet*>(ee);
                                          if (t->score() == de->score() && t->track() == de->track()) {
                                                de->setTuplet(t);
                                                t->add(de);
                                                break;
                                                }
                                          }
                                    }
                              }
                        //select(rest, SelectType::SINGLE, 0);
                        }
                  else  {
                        // remove segment if empty
                        Segment* seg = chord->segment();
                        if (seg->empty())
                              undoRemoveElement(seg);
                        }
                  }
                  break;

            case ElementType::REPEAT_MEASURE:
                  {
                  RepeatMeasure* rm = toRepeatMeasure(el);
                  removeChordRest(rm, false);
                  Rest* rest = new Rest(this);
                  rest->setDurationType(TDuration::DurationType::V_MEASURE);
                  rest->setDuration(rm->measure()->stretchedLen(rm->staff()));
                  rest->setTrack(rm->track());
                  rest->setParent(rm->parent());
                  Segment* segment = rm->segment();
                  undoAddCR(rest, segment->measure(), segment->tick());
                  }
                  // fall through

            case ElementType::REST:
                  //
                  // only allow for voices != 0
                  //    e.g. voice 0 rests cannot be removed
                  //
                  {
                  Rest* rest = toRest(el);
                  if (rest->tuplet() && rest->tuplet()->elements().empty())
                        undoRemoveElement(rest->tuplet());
                  if (el->voice() != 0) {
                        rest->undoChangeProperty(P_ID::GAP, true);
                        for (ScoreElement* r : el->linkList()) {
                              Rest* rest = static_cast<Rest*>(r);
                              if (rest->track() % VOICES)
                                    rest->undoChangeProperty(P_ID::GAP, true);
                              }

                        // delete them really when only gap rests are in the actual measure.
                        Measure* m = toRest(el)->measure();
                        int track = el->track();
                        if (m->isOnlyDeletedRests(track)) {
                              static const SegmentType st { SegmentType::ChordRest };
                              for (const Segment* s = m->first(st); s; s = s->next(st)) {
                                    Element* del = s->element(track);
                                    if (s->segmentType() != st || !del)
                                          continue;
                                    if (toRest(del)->isGap())
                                          undoRemoveElement(del);
                                    }
                              }
                        else {
                              // check if the other rest could be combined
                              Segment* s = toRest(el)->segment();

                              std::vector<Rest*> rests;
                              // find previous segment with cr in this track
                              Element* pe = 0;
                              for (Segment* ps = s->prev(SegmentType::ChordRest); ps; ps = ps->prev(SegmentType::ChordRest)) {
                                    Element* el = ps->element(track);
                                    if (el && el->isRest() && toRest(el)->isGap()) {
                                          pe = el;
                                          rests.push_back(toRest(el));
                                          }
                                    else if (el)
                                          break;
                                    }
                              // find next segment with cr in this track
                              Segment* ns;
                              Element* ne = 0;
                              for (ns = s->next(SegmentType::ChordRest); ns; ns = ns->next(SegmentType::ChordRest)) {
                                    Element* el = ns->element(track);
                                    if (el && el->isRest() && toRest(el)->isGap()) {
                                          ne = el;
                                          rests.push_back(toRest(el));
                                          }
                                    else if (el)
                                          break;
                                    }

                              int stick = 0;
                              int ticks = 0;

                              if (pe)
                                    stick = pe->tick();
                              else
                                    stick = s->tick();

                              if (ne)
                                    ticks = ne->tick() - stick + toRest(ne)->actualTicks();
                              else if (ns)
                                    ticks = ns->tick() - stick;
                              else
                                    ticks = m->ticks() + m->tick() - stick;

                              if (ticks != m->ticks() && ticks != s->ticks()) {
                                    undoRemoveElement(rest);
                                    for (Rest* r : rests) {
                                          undoRemoveElement(r);
                                          }

                                    Fraction f = Fraction::fromTicks(ticks);

                                    std::vector<TDuration> dList = toDurationList(f, true);
                                    if (dList.empty())
                                          break;

                                    for (const TDuration& d : dList) {
                                          Rest* rest = new Rest(this);
                                          rest->setDuration(d.fraction());
                                          rest->setDurationType(d);
                                          rest->setTrack(track);
                                          rest->setGap(true);
                                          undoAddCR(rest, m, stick);
                                          }
                                    }
                              }
                        // Set input position
                        // TODO If deleted element is last of a sequence, use prev?
                        if (noteEntryMode())
                              score()->move("prev-chord");
                        }
                  }
                  break;

            case ElementType::ACCIDENTAL:
                  if (el->parent()->isNote())
                        changeAccidental(toNote(el->parent()), AccidentalType::NONE);
                  else
                        undoRemoveElement(el);
                  break;

            case ElementType::BAR_LINE: {
                  BarLine* bl = toBarLine(el);
                  Segment* s = bl->segment();
                  Measure* m = s->measure();
                  if (s->isBeginBarLineType()) {
                        undoRemoveElement(el);
                        }
                  else {
                        if (bl->barLineType() == BarLineType::START_REPEAT)
                              m->undoChangeProperty(P_ID::REPEAT_START, false);
                        else if (bl->barLineType() == BarLineType::END_REPEAT)
                              m->undoChangeProperty(P_ID::REPEAT_END, false);
                        else
                              undoRemoveElement(el);
                        }
                  }
                  break;

            case ElementType::TUPLET:
                  cmdDeleteTuplet(toTuplet(el), true);
                  break;

            case ElementType::MEASURE: {
                  Measure* m = toMeasure(el);
                  undoRemoveMeasures(m, m);
                  undoInsertTime(m->tick(), -(m->endTick() - m->tick()));
                  }
                  break;

            case ElementType::BRACKET:
                  undoRemoveBracket(toBracket(el));
                  break;

            case ElementType::LAYOUT_BREAK:
                  {
                  undoRemoveElement(el);
                  LayoutBreak* lb = toLayoutBreak(el);
                  Measure* m = lb->measure();
                  if (m->isMMRest()) {
                        // propagate to original measure
                        m = m->mmRestLast();
                        foreach(Element* e, m->el()) {
                              if (e->isLayoutBreak()) {
                                    undoRemoveElement(e);
                                    break;
                                    }
                              }
                        }
                  }
                  break;

            case ElementType::CLEF:
                  {
                  Clef* clef = toClef(el);
                  Measure* m = clef->measure();
                  if (m->isMMRest()) {
                        // propagate to original measure
                        m = m->mmRestLast();
                        Segment* s = m->findSegment(SegmentType::Clef, clef->segment()->tick());
                        if (s && s->element(clef->track())) {
                              Clef* c = toClef(s->element(clef->track()));
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
                                    Segment* s = m->findSegment(SegmentType::Clef, tick);
                                    if (s && s->element(clef->track()))
                                          clef = toClef(s->element(clef->track()));
                                    }
                              }
                        undoRemoveElement(clef);
                        }
                  }
                  break;

            case ElementType::REHEARSAL_MARK:
            case ElementType::TEMPO_TEXT:
                  {
                  Segment* s = toSegment(el->parent());
                  Measure* m = s->measure();
                  if (m->isMMRest()) {
                        // propagate to original measure/element
                        m = m->mmRestFirst();
                        Segment* ns = m->findSegment(SegmentType::ChordRest, s->tick());
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

            case ElementType::OTTAVA_SEGMENT:
            case ElementType::HAIRPIN_SEGMENT:
            case ElementType::TRILL_SEGMENT:
            case ElementType::TEXTLINE_SEGMENT:
            case ElementType::VOLTA_SEGMENT:
            case ElementType::SLUR_SEGMENT:
            case ElementType::TIE_SEGMENT:
            case ElementType::PEDAL_SEGMENT:
            case ElementType::GLISSANDO_SEGMENT:
                  {
                  el = static_cast<SpannerSegment*>(el)->spanner();
                  undoRemoveElement(el);
                  }
                  break;

            case ElementType::STEM_SLASH:           // cannot delete this elements
            case ElementType::HOOK:
                  qDebug("cannot remove %s", el->name());
                  break;

            case ElementType::TEXT:
                  if (el->parent()->isTBox())
                        undoChangeProperty(el, P_ID::TEXT, QString());
                  else
                        undoRemoveElement(el);
                  break;

            default:
                  undoRemoveElement(el);
                  break;
            }
      }

//---------------------------------------------------------
//   deleteMeasures
//---------------------------------------------------------

void Score::deleteMeasures(MeasureBase* is, MeasureBase* ie)
      {
// qDebug("deleteMeasures %p %p", is, ie);

#if 0
      if (!selection().isRange())
            return;

      MeasureBase* is = selection().startSegment()->measure();
      if (is->isMeasure() && toMeasure(is)->isMMRest())
            is = toMeasure(is)->mmRestFirst();
      Segment* seg    = selection().endSegment();
      MeasureBase* ie;

      // choose the correct last measure based on the end segment
      // this depends on whether a whole measure is selected or only a few notes within it
      if (seg)
            ie = seg->prev() ? seg->measure() : seg->measure()->prev();
      else
            ie = lastMeasure();
#endif

      select(0, SelectType::SINGLE, 0);

      // createEndBar if last measure is deleted
      bool createEndBar = false;
      if (ie->isMeasure()) {
            Measure* iem = toMeasure(ie);
            if (iem->isMMRest())
                  ie = iem = iem->mmRestLast();
//TODO            createEndBar = (iem == lastMeasureMM()) && (iem->endBarLineType() == BarLineType::END);
            createEndBar = false;
            }


      // get the last deleted timesig & keysig in order to restore after deletion
      KeySigEvent lastDeletedKeySigEvent;
      TimeSig* lastDeletedSig   = 0;
      KeySig* lastDeletedKeySig = 0;
      bool transposeKeySigEvent = false;

      for (MeasureBase* mb = ie;; mb = mb->prev()) {
            if (mb->isMeasure()) {
                  Measure* m = toMeasure(mb);
                  Segment* sts = m->findSegment(SegmentType::TimeSig, m->tick());
                  if (sts && !lastDeletedSig)
                        lastDeletedSig = toTimeSig(sts->element(0));
                  sts = m->findSegment(SegmentType::KeySig, m->tick());
                  if (sts && !lastDeletedKeySig) {
                        lastDeletedKeySig = toKeySig(sts->element(0));
                        if (lastDeletedKeySig) {
                              lastDeletedKeySigEvent = lastDeletedKeySig->keySigEvent();
                              if (!styleB(StyleIdx::concertPitch) && !lastDeletedKeySigEvent.isAtonal() && !lastDeletedKeySigEvent.custom()) {
                                    // convert to concert pitch
                                    transposeKeySigEvent = true;
                                    Interval v = staff(0)->part()->instrument(m->tick())->transpose();
                                    if (!v.isZero())
                                          lastDeletedKeySigEvent.setKey(transposeKey(lastDeletedKeySigEvent.key(), v));
                                    }
                              }
                        }
                  if (lastDeletedSig && lastDeletedKeySig)
                        break;
                  }
            if (mb == is)
                  break;
            }
      int startTick = is->tick();
      int endTick   = ie->tick();

      undoInsertTime(is->tick(), -(ie->endTick() - is->tick()));
      for (Score* score : scoreList()) {
            Measure* is = score->tick2measure(startTick);
            Measure* ie = score->tick2measure(endTick);

            score->undoRemoveMeasures(is, ie);

            // adjust views
            Measure* focusOn = is->prevMeasure() ? is->prevMeasure() : score->firstMeasure();
            for (MuseScoreView* v : score->viewer)
                  v->adjustCanvasPosition(focusOn, false);

            if (createEndBar) {
//                  Measure* lastMeasure = score->lastMeasure();
//TODO                  if (lastMeasure && lastMeasure->endBarLineType() == BarLineType::NORMAL)
//                        score->undoChangeEndBarLineType(lastMeasure, BarLineType::END);
                  }

            // insert correct timesig after deletion
            Measure* mBeforeSel = is->prevMeasure();
            Measure* mAfterSel  = mBeforeSel ? mBeforeSel->nextMeasure() : score->firstMeasure();
            if (mAfterSel && lastDeletedSig) {
                  bool changed = true;
                  if (mBeforeSel) {
                        if (mBeforeSel->timesig() == mAfterSel->timesig()) {
                              changed = false;
                              }
                        }
                  Segment* s = mAfterSel->findSegment(SegmentType::TimeSig, mAfterSel->tick());
                  if (!s && changed) {
                        Segment* ns = mAfterSel->undoGetSegment(SegmentType::TimeSig, mAfterSel->tick());
                        for (int staffIdx = 0; staffIdx < score->nstaves(); staffIdx++) {
                              TimeSig* nts = new TimeSig(score);
                              nts->setTrack(staffIdx * VOICES);
                              nts->setParent(ns);
                              nts->setSig(lastDeletedSig->sig(), lastDeletedSig->timeSigType());
                              score->undoAddElement(nts);
                              }
                        }
                  }
            // insert correct keysig if necessary
            if (mAfterSel && !mBeforeSel && lastDeletedKeySig) {
                  Segment* s = mAfterSel->findSegment(SegmentType::KeySig, mAfterSel->tick());
                  if (!s) {
                        Segment* ns = mAfterSel->undoGetSegment(SegmentType::KeySig, mAfterSel->tick());
                        for (int staffIdx = 0; staffIdx < score->nstaves(); staffIdx++) {
                              KeySigEvent nkse = lastDeletedKeySigEvent;
                              if (transposeKeySigEvent) {
                                    Interval v = score->staff(staffIdx)->part()->instrument(0)->transpose();
                                    v.flip();
                                    nkse.setKey(transposeKey(nkse.key(), v));
                                    }
                              KeySig* nks = new KeySig(score);
                              nks->setTrack(staffIdx * VOICES);
                              nks->setParent(ns);
                              nks->setKeySigEvent(nkse);
                              score->undoAddElement(nks);
                              }
                        }
                  }
            }

      _is.setSegment(0);        // invalidate position
      }

//---------------------------------------------------------
//   cmdDeleteSelection
//---------------------------------------------------------

void Score::cmdDeleteSelection()
      {
      ChordRest* cr = nullptr;      // select something after deleting notes

      if (selection().isRange()) {
            Segment* s1 = selection().startSegment();
            Segment* s2 = selection().endSegment();

            // delete content from measures underlying mmrests
            if (s1 && s1->measure() && s1->measure()->isMMRest())
                  s1 = s1->measure()->mmRestFirst()->first();
            if (s2 && s2->measure() && s2->measure()->isMMRest())
                  s2 = s2->measure()->mmRestLast()->last();

            int stick1 = selection().tickStart();
            int stick2 = selection().tickEnd();

            Segment* ss1 = s1;
            if (!ss1->isChordRestType())
                  ss1 = ss1->next1(SegmentType::ChordRest);
            bool fullMeasure = ss1 && (ss1->measure()->first(SegmentType::ChordRest) == ss1)
                  && (s2 == 0 || s2->isEndBarLineType());

            int tick2   = s2 ? s2->tick() : INT_MAX;
            int track1  = selection().staffStart() * VOICES;
            int track2  = selection().staffEnd() * VOICES;
            auto spanners = _spanner.findOverlapping(stick1, stick2 - 1);
            for (auto i : spanners) {
                  Spanner* sp = i.value;
                  if (sp->isVolta())
                        continue;
                  if (sp->track() >= track1 && sp->track() < track2) {
                        if (sp->tick() >= stick1 && sp->tick() < stick2
                            && sp->tick2() >= stick1 && sp->tick2() < stick2) {
                              undoRemoveElement(sp);
                              }
                        else if (sp->isSlur() && ((sp->tick() >= stick1 && sp->tick() < stick2)
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
                        if (s->element(track) && s->isBreathType()) {
                              deleteItem(s->element(track));
                              continue;
                              }
                        foreach (Element* annotation, s->annotations()) {
                              // skip if not included in selection (eg, filter)
                              if (!selectionFilter().canSelect(annotation))
                                    continue;
                              if (!annotation->systemFlag() && annotation->track() == track)
                                    undoRemoveElement(annotation);
                              }

                        Element* e = s->element(track);
                        if (!e)
                              continue;
                        if (!s->isChordRestType()) {
                              // do not delete TimeSig/KeySig,
                              // it doesn't make sense to do it, except on full system
                              if (s->segmentType() != SegmentType::TimeSig && s->segmentType() != SegmentType::KeySig)
                                    undoRemoveElement(e);
                              continue;
                              }
                        ChordRest* cr = toChordRest(e);
                        if (tick == -1) {
                              // first ChordRest found:
                              int offset = cr->rtick();
                              if (cr->measure()->tick() >= s1->tick() && offset) {
                                    f = Fraction::fromTicks(offset);
                                    tick = s->measure()->tick();
                                    }
                              else {
                                    tick   = s->tick();
                                    f      = Fraction();
                                    }
                              tuplet = cr->tuplet();
                              if (tuplet && (tuplet->tick() == tick) && ((tuplet->tick() + tuplet->actualTicks()) <= tick2) ) {
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

                        if (fullMeasure) {
                              // handle this as special case to be able to
                              // fix broken measures:
                              for (Measure* m = s1->measure(); m; m = m->nextMeasure()) {
                                    Staff* staff = Score::staff(track / VOICES);
                                    int tick = m->tick();
                                    TimeSig* ts = staff->timeSig(tick);
                                    if (ts) {
                                          Fraction f = ts->sig();
                                          Rest* r = setRest(tick, track, f, false, 0);
                                          if (!cr)
                                                cr = r;
                                          if (s2 && (m == s2->measure()))
                                                break;
                                          }
                                    }
                              }
                        else {
                              Rest* r = setRest(tick, track, f, false, tuplet);
                              if (!cr)
                                    cr = r;
                              }
                        }
                  }
            s1 = tick2segment(stick1);
            s2 = tick2segment(stick2, true);
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
            QList<Element*> el = selection().elements();

            // keep track of linked elements that are deleted implicitly
            // so we don't try to delete them twice if they are also in selection
            QList<ScoreElement*> deletedElements;
            // Similarly, deleting one spanner segment, will delete all of them
            // so we don't try to delete them twice if they are also in selection
            QList<Spanner*> deletedSpanners;

            for (Element* e : el) {
                  // these are the linked elements we are about to delete
                  QList<ScoreElement*> links;
                  if (e->links())
                        links = *e->links();

                  // find location of element to select after deleting notes
                  int tick  = -1;
                  int track = -1;
                  if (!cr) {
                        if (e->isNote())
                              tick = toNote(e)->chord()->tick();
                        else if (e->isRest())
                              tick = toRest(e)->tick();
                        //else tick < 0
                        track = e->track();
                        }

                  // delete element if we have not done so already
                  if (!deletedElements.contains(e)) {
                        // do not delete two spanner segments from the same spanner
                        if (e->isSpannerSegment()) {
                              Spanner* spanner = static_cast<SpannerSegment*>(e)->spanner();
                              if (deletedSpanners.contains(spanner))
                                    continue;
                              else {
                                    QList<ScoreElement*> linkedSpanners;
                                    if (spanner->links())
                                          linkedSpanners = *spanner->links();
                                    else
                                          linkedSpanners.append(spanner);
                                    for (ScoreElement* se : linkedSpanners)
                                          deletedSpanners.append(static_cast<Spanner*>(se));
                                    }
                              }
                        deleteItem(e);
                        }

                  // find element to select
                  if (!cr && tick >= 0 && track >= 0)
                        cr = findCR(tick, track);

                  // add these linked elements to list of already-deleted elements
                  for (ScoreElement* se : links)
                        deletedElements.append(se);
                  }

            }

      deselectAll();
      // make new selection if appropriate
      if (noteEntryMode())
            cr = _is.cr();
      if (cr) {
            if (cr->isChord())
                  select(toChord(cr)->upNote(), SelectType::SINGLE);
            else
                  select(cr, SelectType::SINGLE);
            }
      }

//---------------------------------------------------------
//   cmdFullMeasureRest
//---------------------------------------------------------

void Score::cmdFullMeasureRest()
      {
      Segment* s1 = nullptr;
      Segment* s2 = nullptr;
      int stick1 = -1;
      int stick2 = -1;
      int track1 = -1;
      int track2 = -1;
      Rest* r = nullptr;

      if (noteEntryMode()) {
            s1 = inputState().segment();
            if (!s1 || s1->rtick() != 0)
                  return;
            Measure* m = s1->measure();
            s2 = m->last();
            stick1 = s1->tick();
            stick2 = s2->tick();
            track1 = inputState().track();
            track2 = track1 + 1;
            }
      else if (selection().isRange()) {
            s1 = selection().startSegment();
            s2 = selection().endSegment();
            if (styleB(StyleIdx::createMultiMeasureRests)) {
                   // use underlying measures
                   if (s1 && s1->measure()->isMMRest())
                         s1 = tick2segment(stick1);
                   if (s2 && s2->measure()->isMMRest())
                         s2 = tick2segment(stick2, true);
                   }
            stick1 = selection().tickStart();
            stick2 = selection().tickEnd();
            Segment* ss1 = s1;
            if (ss1 && ss1->segmentType() != SegmentType::ChordRest)
                  ss1 = ss1->next1(SegmentType::ChordRest);
            bool fullMeasure = ss1 && (ss1->measure()->first(SegmentType::ChordRest) == ss1)
                  && (s2 == 0 || (s2->segmentType() == SegmentType::EndBarLine)
                        || (s2->segmentType() == SegmentType::TimeSigAnnounce)
                        || (s2->segmentType() == SegmentType::KeySigAnnounce));
            if (!fullMeasure) {
                  return;
                  }
            track1 = selection().staffStart() * VOICES;
            track2 = selection().staffEnd() * VOICES;
            }
      else if (selection().cr()) {
            ChordRest* cr = selection().cr();
            if (!cr || cr->rtick() != 0)
                  return;
            Measure* m = cr->measure();
            s1 = m->first();
            s2 = m->last();
            stick1 = s1->tick();
            stick2 = s2->tick();
            track1 = selection().cr()->track();
            track2 = track1 + 1;
            }
      else {
            return;
            }

      for (int track = track1; track < track2; ++track) {
            if (selection().isRange() && !selectionFilter().canSelectVoice(track))
                  continue;
            // first pass - remove non-initial rests from empty measures/voices
            for (Segment* s = s1; s != s2; s = s->next1()) {
                  if (!(s->measure()->isOnlyRests(track))) // Don't remove anything from measures that contain notes
                        continue;
                  if (s->segmentType() != SegmentType::ChordRest || !s->element(track))
                        continue;
                  ChordRest* cr = toChordRest(s->element(track));
                  // keep first rest of measure as placeholder (replaced in second pass)
                  // but delete all others
                  if (s->rtick())
                        removeChordRest(cr, true);
                  }
            // second pass - replace placeholders with full measure rests
            for (Measure* m = s1->measure(); m; m = m->nextMeasure()) {
                  if (m->isOnlyRests(track)) {
                        ChordRest* cr = m->findChordRest(m->tick(), track);
                        if (cr) {
                              removeChordRest(cr, true);
                              r = addRest(m->tick(), track, TDuration(TDuration::DurationType::V_MEASURE), 0);
                              }
                        else if (noteEntryMode()) {
                              // might be no cr at input position
                              r = addRest(m->tick(), track, TDuration(TDuration::DurationType::V_MEASURE), 0);
                              }
                        }
                  if (s2 && (m == s2->measure()))
                        break;
                  }
            }

      // selected range is probably empty now and possibly subsumed by an mmrest
      // so updating selection requires forcing mmrests to be updated first
//TODO-ws      if (styleB(StyleIdx::createMultiMeasureRests))
//            createMMRests();
      s1 = tick2segmentMM(stick1);
      s2 = tick2segmentMM(stick2, true);
      if (selection().isRange() && s1 && s2) {
            _selection.setStartSegment(s1);
            _selection.setEndSegment(s2);
            _selection.updateSelectedElements();
            }
      else if (r) {
            // note entry mode
            select(r, SelectType::SINGLE);
            }
      else {
            deselectAll();
            }
      }

//---------------------------------------------------------
//   addLyrics
//    called from Keyboard Accelerator & menu
//---------------------------------------------------------

Lyrics* Score::addLyrics()
      {
      Element* el = selection().element();
      if (el == 0 || (!el->isNote() && !el->isLyrics())) {
            MScore::setError(NO_LYRICS_SELECTED);
            return 0;
            }
      ChordRest* cr;
      if (el->isNote()) {
            cr = toNote(el)->chord();
            if(cr->isGrace())
                  cr = toChordRest(cr->parent());
            }
      else if (el->isLyrics())
            cr = toLyrics(el)->chordRest();
      else
            return 0;

      int no = cr->lyrics().size();
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

Hairpin* Score::addHairpin(HairpinType t, int tickStart, int tickEnd, int track)
      {
      Hairpin* pin = new Hairpin(this);
      pin->setHairpinType(t);
      if (t == HairpinType::CRESC_LINE) {
            pin->setBeginText("cresc.");
            pin->setContinueText("(cresc.)");
            }
      else if (t == HairpinType::DECRESC_LINE) {
            pin->setBeginText("dim.");
            pin->setContinueText("(dim.)");
            }
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
      if (ocr->isChord()) {
            cr = new Chord(this);
            foreach (Note* oldNote, toChord(ocr)->notes()) {
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
                  addRefresh(e->abbox());
                  if (e->isBarLine()) {
//                        Element* ep = e->parent();
//                        if (ep->isSegment() && toSegment(ep)->isEndBarLineType()) {
//                              Measure* m = toSegment(ep)->measure();
//                              BarLine* bl = toBarLine(e);
//                              m->setEndBarLineType(bl->barLineType(), false, e->visible(), e->color());
//                              }
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
            MScore::setError(NO_STAFF_SELECTED);
            return;
            }
      int t1 = selection().tickStart();
      int t2 = selection().tickEnd();

      Measure* m1 = tick2measure(t1);
      Measure* m2 = tick2measure(t2);

      if (selection().score()->excerpt())
            return;

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
      setNoteRest(_is.segment(), track, nval, d.fraction(), Direction::AUTO);
      _is.moveToNextInputPos();
      if (!noteEntryMode() || usingNoteEntryMethod(NoteEntryMethod::STEPTIME))
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
      for (ScoreElement* e : cr->linkList()) {
            undo(new RemoveElement(static_cast<Element*>(e)));
            if (clearSegment) {
                  Segment* s = cr->segment();
                  if (!segments.contains(s))
                        segments.append(s);
                  }
            }
      for (Segment* s : segments) {
            if (s->empty())
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
      foreach(DurationElement* de, tuplet->elements()) {
            if (de->isChordRest())
                  removeChordRest(toChordRest(de), true);
            else {
                  Q_ASSERT(de->isTuplet());
                  cmdDeleteTuplet(toTuplet(de), false);
                  }
            }
      if (replaceWithRest)
            setRest(tuplet->tick(), tuplet->track(), tuplet->duration(), true, tuplet->tuplet());
      }

//---------------------------------------------------------
//   nextInputPos
//---------------------------------------------------------

void Score::nextInputPos(ChordRest* cr, bool doSelect)
      {
      ChordRest* ncr = nextChordRest(cr);
      if ((ncr == 0) && (_is.track() % VOICES)) {
            Segment* s = tick2segment(cr->tick() + cr->actualTicks(), false, SegmentType::ChordRest);
            int track = (cr->track() / VOICES) * VOICES;
            ncr = s ? toChordRest(s->element(track)) : 0;
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
//   insertMeasure
//    Create a new MeasureBase of type type and insert
//    before measure.
//    If measure is zero, append new MeasureBase.
//---------------------------------------------------------

void Score::insertMeasure(ElementType type, MeasureBase* measure, bool createEmptyMeasures)
      {
      int tick;
      if (measure) {
            if (measure->isMeasure() && toMeasure(measure)->isMMRest()) {
                  measure = toMeasure(measure)->prev();
                  measure = measure ? measure->next() : firstMeasure();
                  deselectAll();
                  }
            tick = measure->tick();
            }
      else
            tick = last() ? last()->endTick() : 0;

      Fraction f       = sigmap()->timesig(tick).nominal();       // use nominal time signature of current measure
      Measure* om      = 0;                                       // measure base in "this" score
      MeasureBase* rmb = 0;                                       // measure base in root score (for linking)
      int ticks        = 0;

      for (Score* score : scoreList()) {
            MeasureBase* im = 0;
            if (measure) {
                  if (measure->isMeasure())
                        im = score->tick2measure(tick);
                  else {
                        if (!measure->links()) {
                              if (measure->score() == score)
                                    im = measure;
                              else
                                    qDebug("no links");
                              }
                        else {
                              for (ScoreElement* m : *measure->links()) {
                                    if (measure->score() == score) {
                                          im = toMeasureBase(m);
                                          break;
                                          }
                                    }
                              }
                        }
                  if (!im)
                        qDebug("measure not found");
                  }
            MeasureBase* mb = toMeasureBase(Element::create(type, score));
            mb->setTick(tick);

            mb->setNext(im);
            mb->setPrev(im ? im->prev() : score->last());
            undo(new InsertMeasures(mb, mb));

            if (type == ElementType::MEASURE) {
                  Measure* m  = toMeasure(mb);  // new measure
                  ticks       = m->ticks();
                  Measure* mi = toMeasure(im);  // insert before

                  if (score->isMaster())
                        om = m;

                  m->setTimesig(f);
                  m->setLen(f);

                  QList<TimeSig*> tsl;
                  QList<KeySig*>  ksl;
                  QList<Clef*>    cl;
                  QList<Clef*>    pcl;

                  //
                  // remove clef, time and key signatures
                  //
                  if (mi) {
                        for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                              Measure* pm = mi->prevMeasure();
                              if (pm) {
                                    Segment* ps = pm->findSegment(SegmentType::Clef, tick);
                                    if (ps) {
                                          Element* pc = ps->element(staffIdx * VOICES);
                                          if (pc) {
                                                pcl.push_back(toClef(pc));
                                                undo(new RemoveElement(pc));
                                                if (ps->empty())
                                                      undoRemoveElement(ps);
                                                }
                                          }
                                    }
                              for (Segment* s = mi->first(); s && s->rtick() == 0; s = s->next()) {
                                    if (s->isHeaderClefType())
                                          continue;
                                    Element* e = s->element(staffIdx * VOICES);
                                    if (!e)
                                          continue;
                                    Element* ee = 0;
                                    if (e->isKeySig()) {
                                          KeySig* ks = toKeySig(e);
                                          ksl.push_back(ks);
                                          ee = e;
                                          }
                                    else if (e->isTimeSig()) {
                                          TimeSig* ts = toTimeSig(e);
                                          tsl.push_back(ts);
                                          ee = e;
                                          }
                                    if (tick == 0 && e->isClef()) {
                                          Clef* clef = toClef(e);
                                          cl.push_back(clef);
                                          ee = e;
                                          }
                                    if (ee) {
                                          undo(new RemoveElement(ee));
                                          if (s->empty())
                                                undoRemoveElement(s);
                                          }
                                    }
                              }
                        }


                  //
                  // move clef, time, key signatrues
                  //
                  for (TimeSig* ts : tsl) {
                        TimeSig* nts = new TimeSig(*ts);
                        Segment* s   = m->undoGetSegmentR(SegmentType::TimeSig, 0);
                        nts->setParent(s);
                        undoAddElement(nts);
                        }
                  for (KeySig* ks : ksl) {
                        KeySig* nks = new KeySig(*ks);
                        Segment* s  = m->undoGetSegmentR(SegmentType::KeySig, 0);
                        nks->setParent(s);
                        undoAddElement(nks);
                        }
                  for (Clef* clef : cl) {
                        Clef* nClef = new Clef(*clef);
                        Segment* s  = m->undoGetSegmentR(SegmentType::Clef, 0);
                        nClef->setParent(s);
                        undoAddElement(nClef);
                        }
                  Measure* pm = m->prevMeasure();
                  for (Clef* clef : pcl) {
                        Clef* nClef = new Clef(*clef);
                        Segment* s  = pm->undoGetSegment(SegmentType::Clef, tick);
                        nClef->setParent(s);
                        undoAddElement(nClef);
                        }
                  }
            else {
                  // a frame, not a measure
                  if (isMaster())
                        rmb = mb;
                  else if (rmb && mb != rmb) {
                        mb->linkTo(rmb);
                        if (rmb->isTBox())
                              toTBox(mb)->text()->linkTo(toTBox(rmb)->text());
                        }
                  }
            }

      undoInsertTime(tick, ticks);

      if (om && !createEmptyMeasures) {
            //
            // fill measure with rest
            //
            Score* score = om->score();

            // add rest to all staves and to all the staves linked to it
            for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                  int track = staffIdx * VOICES;
                  Rest* rest = new Rest(score, TDuration(TDuration::DurationType::V_MEASURE));
                  Fraction timeStretch(score->staff(staffIdx)->timeStretch(om->tick()));
                  rest->setDuration(om->len() * timeStretch);
                  rest->setTrack(track);
                  score->undoAddCR(rest, om, tick);
                  }
            }
      deselectAll();
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
      QList<Spanner*> sl;     // spanners to remove
      QList<Spanner*> sl2;    // spanners to shorten
      auto spanners = _spanner.findOverlapping(startTick, endTick);
// printf("checkSpanner %d %d\n", startTick, endTick);
//      for (auto i = spanners.begin(); i < spanners.end(); i++) {

      // DEBUG: check all spanner
      //        there may be spanners outside of score bc. some measures were deleted

      int lastTick = lastMeasure()->endTick();

      for (auto i : _spanner.map()) {
            Spanner* s = i.second;

            if (s->isSlur()) {
                  Segment* seg = tick2segmentMM(s->tick(), false, SegmentType::ChordRest);
                  if (!seg || !seg->element(s->track())) {
                        qDebug("checkSpanner::remove (1) tick %d seg %p", s->tick(), seg);
                        sl.append(s);
                        }
                  else {
                        seg = tick2segmentMM(s->tick2(), false, SegmentType::ChordRest);
                        if (!seg || !seg->element(s->track2())) {
                              qDebug("checkSpanner::remove (2) %d - tick %d track %d",
                                 s->tick(), s->tick2(), s->track2());
                              sl.append(s);
                              }
                        }
                  }
            else {
                  // remove spanner if there is no start element
                  s->computeStartElement();
                  if (!s->startElement()) {
                        sl.append(s);
                        qDebug("checkSpanner::remove (3)");
                        }
                  else {
                        if (s->tick2() > lastTick)
                              sl2.append(s);    //s->undoChangeProperty(P_ID::SPANNER_TICKS, lastTick - s->tick());
                        else
                              s->computeEndElement();
                        }
                  }
            }
      for (auto s : sl)       // actually remove scheduled spanners
            undo(new RemoveElement(s));
      for (auto s : sl2) {    // shorten spanners that extended past end of score
            undo(new ChangeProperty(s, P_ID::SPANNER_TICKS, lastTick - s->tick()));
            s->computeEndElement();
            }
      }

static constexpr SegmentType CR_TYPE = SegmentType::ChordRest;

//---------------------------------------------------------
//   globalTimeDelete
//---------------------------------------------------------

void Score::globalTimeDelete()
      {
      qDebug("not implemented");
      }

//---------------------------------------------------------
//   localTimeDelete
//---------------------------------------------------------

void Score::localTimeDelete()
      {
      Segment* startSegment;
      Segment* endSegment;

      if (selection().state() != SelState::RANGE) {
            Element* el = selection().element();
            if (!el)
                  return;
            ChordRest* cr = 0;
            if (el->isNote())
                  cr = toNote(el)->chord();
            else if (el->isChordRest())
                  cr = toChordRest(el);
            else
                  return;
            startSegment = cr->segment();
            int endTick  = startSegment->tick() + cr->duration().ticks();
            endSegment   = cr->measure()->findSegment(CR_TYPE, endTick);
            if (endSegment == 0) {
                  qDebug("no segment at %d", endTick);
                  return;
                  }
            }
      else {
            startSegment = selection().startSegment();
            endSegment   = selection().endSegment();
            }

      MeasureBase* is = startSegment->measure();
      if (is->isMeasure() && toMeasure(is)->isMMRest())
            is = toMeasure(is)->mmRestFirst();
      MeasureBase* ie;

      if (endSegment)
            ie = endSegment->prev() ? endSegment->measure() : endSegment->measure()->prev();
      else
            ie = lastMeasure();

      for (;;) {
            if (is->tick() != startSegment->tick()) {
                  int tick = startSegment->tick();
                  int len;
                  if (ie == is)
                        len = endSegment->tick() - tick;
                  else
                        len = is->endTick() - tick;
                  timeDelete(toMeasure(is), startSegment, Fraction::fromTicks(len));
                  if (is == ie)
                        break;
                  is = is->next();
                  }
            int endTick = endSegment ? endSegment->tick() : ie->endTick();
            if (ie->endTick() != endTick) {
                  int len  = endSegment->tick() - ie->tick();
                  timeDelete(toMeasure(ie), toMeasure(ie)->first(), Fraction::fromTicks(len));
                  if (is == ie)
                        break;
                  ie = ie->prev();
                  }
            deleteMeasures(is, ie);
            break;
            };

      deselectAll();
      }

//---------------------------------------------------------
//   timeDelete
//---------------------------------------------------------

void Score::timeDelete(Measure* m, Segment* startSegment, const Fraction& f)
      {
      int tick  = startSegment->rtick();
      int len   = f.ticks();
      int etick = tick + len;

      Segment* fs = m->first(CR_TYPE);

      for (int track = 0; track < _staves.size() * VOICES; ++track) {
            if (m->hasVoice(track)) {
                  for (Segment* s = fs; s; s = s->next(CR_TYPE)) {
                        if (s->element(track)) {
                              ChordRest* cr = toChordRest(s->element(track));
                              int cetick    = s->rtick() + cr->duration().ticks();

                              if (cetick <= tick) {
                                    continue;
                                    }
                              if (s->rtick() >= etick) {
                                    break;
                                    }

                              if (cr->isFullMeasureRest()) {
                                    // do nothing
                                    }
                              // inside deleted area
                              else if (s->rtick() >= tick && cetick <= etick) {
                                    // inside
                                    undoRemoveElement(cr);
                                    }
                              else if (s->rtick() >= tick) {
                                    // running out
                                    Fraction ff = cr->duration() - Fraction::fromTicks(cetick - etick);
                                    undoRemoveElement(cr);
                                    createCRSequence(ff, cr, tick + len);
                                    }
                              else if (s->rtick() < tick && cetick <= etick) {
                                    // running in
                                    Fraction f1 = Fraction::fromTicks(tick - s->tick());
                                    changeCRlen(cr, f1, false);
                                    }
                              else {
                                    // running in/out
                                    Fraction f1 = cr->duration() - f;
                                    changeCRlen(cr, f1, false);
                                    }
                              }
                        }
                  }
            }
      undoInsertTime(tick, -len);
      undo(new InsertTime(this, tick, -len));
      for (Segment* s = startSegment->next(); s; s = s->next())
            s->undoChangeProperty(P_ID::TICK, s->rtick() - len);
      undo(new ChangeMeasureLen(m, m->len() - f));
      }

//---------------------------------------------------------
//   cloneVoice
//---------------------------------------------------------

void Score::cloneVoice(int strack, int dtrack, Segment* sf, int lTick, bool link, bool spanner)
      {
      int start = sf->tick();
      TieMap  tieMap;
      TupletMap tupletMap;    // tuplets cannot cross measure boundaries
      Score* score = sf->score();
      Tremolo* tremolo = 0;

      for (Segment* oseg = sf; oseg && oseg->tick() < lTick; oseg = oseg->next1()) {
            Segment* ns = 0;        //create segment later, on demand
            Measure* dm = tick2measure(oseg->tick());

            Element* oe = oseg->element(strack);

            if (oe && !oe->generated() && oe->isChordRest()) {
                  Element* ne;
                  //does a linked clone to create just this element
                  //otherwise element will be add in every linked stave
                  if (link)
                        ne = oe->linkedClone();
                  else
                        ne = oe->clone();
                  ne->setTrack(dtrack);

                  //Don't clone gaps to a first voice
                  if (!(ne->track() % VOICES) && ne->isRest())
                        toRest(ne)->setGap(false);

                  ne->setScore(this);
                  ChordRest* ocr = toChordRest(oe);
                  ChordRest* ncr = toChordRest(ne);

                  //Handle beams
                  if (ocr->beam() && !ocr->beam()->empty() && ocr->beam()->elements().front() == ocr) {
                        Beam* nb = ocr->beam()->clone();
                        nb->clear();
                        nb->setTrack(dtrack);
                        nb->setScore(this);
                        nb->add(ncr);
                        ncr->setBeam(nb);
                        }

                  // clone Tuplets
                  Tuplet* ot = ocr->tuplet();
                  if (ot) {
                        ot->setTrack(strack);
                        Tuplet* nt = tupletMap.findNew(ot);
                        if (nt == 0) {
                              if (link)
                                    nt = toTuplet(ot->linkedClone());
                              else
                                    nt = toTuplet(ot->clone());
                              nt->setTrack(dtrack);
                              nt->setParent(dm);
                              tupletMap.add(ot, nt);

                              Tuplet* nt1 = nt;
                              while (ot->tuplet()) {
                                    Tuplet* nt = tupletMap.findNew(ot->tuplet());
                                    if (nt == 0) {
                                          if (link)
                                                nt = toTuplet(ot->tuplet()->linkedClone());
                                          else
                                                nt = toTuplet(ot->tuplet()->clone());
                                          nt->setTrack(dtrack);
                                          nt->setParent(dm);
                                          tupletMap.add(ot->tuplet(), nt);
                                          }
                                    nt->add(nt1);
                                    nt1->setTuplet(nt);
                                    ot = ot->tuplet();
                                    nt1 = nt;
                                    }
                              }
                        nt->add(ncr);
                        ncr->setTuplet(nt);
                        }

                  // clone additional settings
                  if (oe->isChordRest()) {
                        if (oe->isRest()) {
                              Rest* ore = toRest(ocr);
                              // If we would clone a full measure rest just don't clone this rest
                              if (ore->isFullMeasureRest() && (dtrack % VOICES)) {
                                    continue;
                                    }
                              }

                        if (oe->isChord()) {
                              Chord* och = toChord(ocr);
                              Chord* nch = toChord(ncr);

                              int n = och->notes().size();
                              for (int i = 0; i < n; ++i) {
                                    Note* on = och->notes().at(i);
                                    Note* nn = nch->notes().at(i);
                                    Interval v = staff(dtrack) ? staff(dtrack)->part()->instrument(dtrack)->transpose() : Interval();
                                    nn->setTpc1(on->tpc1());
                                    if (v.isZero())
                                          nn->setTpc2(on->tpc1());
                                    else {
                                          v.flip();
                                          nn->setTpc2(Ms::transposeTpc(nn->tpc1(), v, true));
                                          }

                                    if (on->tieFor()) {
                                          Tie* tie;
                                          if (link)
                                                tie = toTie(on->tieFor()->linkedClone());
                                          else
                                                tie = toTie(on->tieFor()->clone());
                                          tie->setScore(this);
                                          nn->setTieFor(tie);
                                          tie->setStartNote(nn);
                                          tie->setTrack(nn->track());
                                          tie->setEndNote(nn);
                                          tieMap.add(on->tieFor(), tie);
                                          }
                                    if (on->tieBack()) {
                                          Tie* tie = tieMap.findNew(on->tieBack());
                                          if (tie) {
                                                nn->setTieBack(tie);
                                                tie->setEndNote(nn);
                                                }
                                          else {
                                                qDebug("cloneVoices: cannot find tie");
                                                }
                                          }
                                    // add back spanners (going back from end to start spanner element
                                    // makes sure the 'other' spanner anchor element is already set up)
                                    // 'on' is the old spanner end note and 'nn' is the new spanner end note
                                    for (Spanner* oldSp : on->spannerBack()) {
                                          Note* newStart = Spanner::startElementFromSpanner(oldSp, nn);
                                          if (newStart) {
                                                Spanner* newSp;
                                                if (link)
                                                      newSp = static_cast<Spanner*>(oldSp->linkedClone());
                                                else
                                                      newSp = static_cast<Spanner*>(oldSp->clone());
                                                newSp->setNoteSpan(newStart, nn);
                                                addElement(newSp);
                                                }
                                          else {
                                                qDebug("cloneVoices: cannot find spanner start note");
                                                }
                                          }
                                    }
                              // two note tremolo
                              if (och->tremolo() && och->tremolo()->twoNotes()) {
                                   if (och == och->tremolo()->chord1()) {
                                          if (tremolo)
                                                qDebug("unconnected two note tremolo");
                                          if (link)
                                                tremolo = toTremolo(och->tremolo()->linkedClone());
                                          else
                                                tremolo = toTremolo(och->tremolo()->clone());
                                          tremolo->setScore(nch->score());
                                          tremolo->setParent(nch);
                                          tremolo->setTrack(nch->track());
                                          tremolo->setChords(nch, 0);
                                          nch->setTremolo(tremolo);
                                          }
                                    else if (och == och->tremolo()->chord2()) {
                                          if (!tremolo)
                                                qDebug("first note for two note tremolo missing");
                                          else {
                                                tremolo->setChords(tremolo->chord1(), nch);
                                                nch->setTremolo(tremolo);
                                                }
                                          }
                                    else
                                          qDebug("inconsistent two note tremolo");
                                    }
                              }

                        // Add element (link -> just in this measure)
                        if (link) {
                              if (!ns)
                                    ns = dm->getSegment(oseg->segmentType(), oseg->tick());
                              ns->add(ne);
                              }
                        else {
                              undoAddCR(toChordRest(ne), dm, oseg->tick());
                              }
                        }
                  }
            Segment* tst = dm->segments().firstCRSegment();
            if (strack % VOICES && !(dtrack % VOICES) && (!tst || (!tst->element(dtrack)))) {
                  Rest* rest = new Rest(this);
                  rest->setDuration(dm->len());
                  rest->setDurationType(TDuration::DurationType::V_MEASURE);
                  rest->setTrack(dtrack);
                  if (link) {
                        Segment* segment = dm->getSegment(SegmentType::ChordRest, dm->tick());
                        segment->add(rest);
                        }
                  else
                        undoAddCR(toChordRest(rest), dm, dm->tick());
                  }
            }

      if (spanner) {
            // Find and add corresponding slurs
            auto spanners = score->spannerMap().findOverlapping(start, lTick);
            for (auto i = spanners.begin(); i < spanners.end(); i++) {
                  Spanner* sp = i->value;
                  int spStart = sp->tick();
                  int track   = sp->track();
                  int track2  = sp->track2();
                  int spEnd = spStart + sp->ticks();
                  qDebug("Start %d End %d Diff %d \n Measure Start %d End %d", spStart, spEnd, spEnd-spStart, start, lTick);
                  if (sp->isSlur() && (spStart >= start && spEnd < lTick)) {
                        if (track == strack && track2 == strack){
                              Spanner* ns;
                              if (link)
                                    ns =  static_cast<Spanner*>(sp->linkedClone());
                              else
                                    ns = static_cast<Spanner*>(sp->clone());

                              ns->setScore(this);
                              ns->setParent(0);
                              ns->setTrack(dtrack);
                              ns->setTrack2(dtrack);

                              // set start/end element for slur
                              ChordRest* cr1 = sp->startCR();
                              ChordRest* cr2 = sp->endCR();

                              ns->setStartElement(0);
                              ns->setEndElement(0);
                              if (cr1 && cr1->links()) {
                                    for (ScoreElement* e : *cr1->links()) {
                                          ChordRest* cr = static_cast<ChordRest*>(e);
                                          if (cr == cr1)
                                                continue;
                                          if ((cr->score() == this) && (cr->tick() == ns->tick()) && cr->track() == dtrack) {
                                                ns->setStartElement(cr);
                                                break;
                                                }
                                          }
                                    }
                              if (cr2 && cr2->links()) {
                                    for (ScoreElement* e : *cr2->links()) {
                                          ChordRest* cr = static_cast<ChordRest*>(e);
                                          if (cr == cr2)
                                                continue;
                                          if ((cr->score() == this) && (cr->tick() == ns->tick2()) && cr->track() == dtrack) {
                                                ns->setEndElement(cr);
                                                break;
                                                }
                                          }
                                    }
                              undo(new AddElement(ns));
                              }
                        }
                  }
            }

      //Layout
//TODO ??      doLayoutRange(start, lTick);
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Score::undoChangeProperty(ScoreElement* e, P_ID t, const QVariant& st, PropertyFlags ps)
      {
      if (propertyLink(t)) {
            for (ScoreElement* ee : e->linkList()) {
                  if (ee->getProperty(t) != st || ee->propertyFlags(t) != ps)
                        undo(new ChangeProperty(ee, t, st, ps));
                  }
            }
      else {
            if (e->getProperty(t) != st || e->propertyFlags(t) != ps)
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
                              undoStack()->push(new ChangeProperty(ee, t, e->getProperty(t)), 0);
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
//   undoChangeStyleVal
//---------------------------------------------------------

void Score::undoChangeStyleVal(StyleIdx idx, const QVariant& v)
      {
      undo(new ChangeStyleVal(this, idx, v));
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
            undoStack()->push(new ChangePitch(n, pitch, tpc1, tpc2), 0);
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
                  undo(new ChangeFretting(n, pitch, string, fret, tpc1, tpc2));
                  }
            }
      else
            undo(new ChangeFretting(note, pitch, string, fret, tpc1, tpc2));
      }

//---------------------------------------------------------
//   undoChangeKeySig
//---------------------------------------------------------

void Score::undoChangeKeySig(Staff* ostaff, int tick, KeySigEvent key)
      {
      KeySig* lks = 0;

      for (Staff* staff : ostaff->staffList()) {
            if (staff->isDrumStaff(tick))
                  continue;

            Score* score = staff->score();
            Measure* measure = score->tick2measure(tick);
            if (!measure) {
                  qWarning("measure for tick %d not found!", tick);
                  continue;
                  }
            Segment* s   = measure->undoGetSegment(SegmentType::KeySig, tick);
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

void Score::undoChangeClef(Staff* ostaff, Segment* seg, ClefType ct)
      {
      SegmentType st;
      if (seg->isHeaderClefType())
            st = SegmentType::HeaderClef;
      else if (seg->isClefType())
            st = SegmentType::Clef;
      else if (seg->rtick() == 0)
            st = SegmentType::HeaderClef;
      else
            st = SegmentType::Clef;

      bool moveClef = (st == SegmentType::HeaderClef) && seg->measure()->prevMeasure();
      bool small = !seg->header() || moveClef;

      Clef* gclef = 0;
      int tick = seg->tick();
      int rtick = seg->rtick();
      for (Staff* staff : ostaff->staffList()) {
            if (staff->staffType(tick)->group() != ClefInfo::staffGroup(ct))
                  continue;

            Score* score     = staff->score();
            Measure* measure = score->tick2measure(tick);

            if (!measure) {
                  qWarning("measure for tick %d not found!", tick);
                  continue;
                  }

            Segment* destSeg;
            int rt;
            if (moveClef) {            // if at start of measure and there is a previous measure
                  measure = measure->prevMeasure();
                  rt      = measure->ticks();
                  }
            else
                  rt = rtick;
            destSeg = measure->undoGetSegmentR(st, rt);

            int staffIdx = staff->idx();
            int track    = staffIdx * VOICES;
            Clef* clef   = toClef(destSeg->element(track));

            if (clef) {
                  //
                  // for transposing instruments, differentiate
                  // clef type for concertPitch
                  //
                  Instrument* i = staff->part()->instrument(tick);
                  ClefType cp, tp;
                  if (i->transpose().isZero()) {
                        cp = ct;
                        tp = ct;
                        }
                  else {
                        bool concertPitch = clef->concertPitch();
                        if (concertPitch) {
                              cp = ct;
                              tp = clef->transposingClef();
                              }
                        else {
                              cp = clef->concertClef();
                              tp = ct;
                              }
                        }
                  clef->setGenerated(false);
                  score->undo(new ChangeClefType(clef, cp, tp));
                  // change the clef in the mmRest if any
                  if (measure->hasMMRest()) {
                        Measure* mmMeasure = measure->mmRest();
                        Segment* mmDestSeg = mmMeasure->findSegment(SegmentType::Clef, tick);
                        if (mmDestSeg) {
                              Clef* mmClef = toClef(mmDestSeg->element(clef->track()));
                              if (mmClef)
                                    score->undo(new ChangeClefType(mmClef, cp, tp));
                              }
                        }
                  }
            else {
                  if (gclef) {
                        clef = toClef(gclef->linkedClone());
                        clef->setScore(score);
                        }
                  else {
                        clef = new Clef(score);
                        gclef = clef;
                        }
                  clef->setTrack(track);
                  clef->setClefType(ct);
                  clef->setParent(destSeg);
                  score->undo(new AddElement(clef));
                  clef->layout();
                  }
            clef->setSmall(small);
            }
      }

//---------------------------------------------------------
//   findLinkedVoiceElement
//---------------------------------------------------------

static Element* findLinkedVoiceElement(Element* e, Staff* nstaff)
      {
      Excerpt* se = e->score()->excerpt();
      Excerpt* de = nstaff->score()->excerpt();
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
      Segment* segment = toSegment(e->parent());
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
      Excerpt* de = nstaff->score()->excerpt();
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
      if (!ne->isChord())
            return 0;
      Chord* nc = toChord(ne);
      if (c->isGrace()) {
            Chord* pc = toChord(c->parent());
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
                  ncr = findLinkedChord(toChord(cr), staff);
            else
                  ncr = toChordRest(findLinkedVoiceElement(cr, staff));
            if (!ncr)
                  continue;
            ncr->undoChangeProperty(P_ID::DURATION_TYPE, QVariant::fromValue(d));
            ncr->undoChangeProperty(P_ID::DURATION, QVariant::fromValue(d.fraction()));
            }
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
                  Excerpt* ex = st->score()->excerpt();
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
                  for (Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                        ChordRest* cr = toChordRest(s->element(track));
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
            if (ostaff->score()->excerpt() && strack > -1)
                  strack = ostaff->score()->excerpt()->tracks().key(strack, -1);
            else
                  strack = ostaff->idx() * VOICES + element->track() % VOICES;
            }

      ElementType et = element->type();

      //
      // some elements are replicated for all parts regardless of
      // linking:
      //

      if ((et == ElementType::REHEARSAL_MARK)
         || (et == ElementType::SYSTEM_TEXT)
         || (et == ElementType::JUMP)
         || (et == ElementType::MARKER)
         || (et == ElementType::TEMPO_TEXT)
         || (et == ElementType::VOLTA)
         ) {
            for (Score* s : scoreList())
                  staffList.append(s->staff(0));

            for (Staff* staff : staffList) {
                  Score* score  = staff->score();
                  int staffIdx  = staff->idx();
                  int ntrack    = staffIdx * VOICES;
                  Element* ne;

                  if (staff->score() == ostaff->score())
                        ne = element;
                  else {
                        // only create linked volta for first staff
                        if (et == ElementType::VOLTA && element->track() != 0)
                              continue;
                        ne = element->linkedClone();
                        ne->setScore(score);
                        ne->setSelected(false);
                        ne->setTrack(staffIdx * VOICES + element->voice());
                        }

                  if (et == ElementType::VOLTA) {
                        Spanner* nsp = static_cast<Spanner*>(ne);
                        Spanner* sp = static_cast<Spanner*>(element);
                        int staffIdx1 = sp->track() / VOICES;
                        int staffIdx2 = sp->track2() / VOICES;
                        int diff = staffIdx2 - staffIdx1;
                        nsp->setTrack2((staffIdx + diff) * VOICES + (sp->track2() % VOICES));
                        undo(new AddElement(nsp));
                        }
                  else if (et == ElementType::MARKER || et == ElementType::JUMP) {
                        Measure* om = toMeasure(element->parent());
                        Measure* m  = score->tick2measure(om->tick());
                        ne->setTrack(element->track());
                        ne->setParent(m);
                        undo(new AddElement(ne));
                        }
                  else {
                        Segment* segment  = toSegment(element->parent());
                        int tick          = segment->tick();
                        Measure* m        = score->tick2measure(tick);
                        Segment* seg      = m->undoGetSegment(SegmentType::ChordRest, tick);
                        ne->setTrack(ntrack);
                        ne->setParent(seg);
                        undo(new AddElement(ne));
                        }
                  }
            return;
            }

      if (et == ElementType::FINGERING
         || (et == ElementType::IMAGE  && !element->parent()->isSegment())
         || (et == ElementType::SYMBOL && !element->parent()->isSegment())
         || et == ElementType::NOTE
         || et == ElementType::TEXT
         || et == ElementType::GLISSANDO
         || et == ElementType::BEND
         || (et == ElementType::CHORD && toChord(element)->isGrace())
            ) {
            Element* parent = element->parent();
            const LinkedElements* links = parent->links();
            // don't link part name
            if (et == ElementType::TEXT) {
                  Text* t = toText(element);
                  if (t->subStyle() == SubStyle::INSTRUMENT_EXCERPT)
                        links = 0;
                  }
            if (links == 0) {
                  undo(new AddElement(element));
#ifndef QT_NO_DEBUG
                  if (element->isChord()) {
                        for (Note* n : toChord(element)->notes()) {
                        //      if(n->tpc() == Tpc::TPC_INVALID)
                        //            n->setTpcFromPitch();
                              Q_ASSERT(n->tpc() != Tpc::TPC_INVALID);
                              }
                        }
#endif
                  return;
                  }
            for (ScoreElement* ee : *links) {
                  Element* e = static_cast<Element*>(ee);
                  Element* ne;
                  if (e == parent)
                        ne = element;
                  else {
                        if (element->isGlissando()) {    // and other spanners with Anchor::NOTE
                              Note* newEnd = Spanner::endElementFromSpanner(toGlissando(element), e);
                              if (newEnd) {
                                    ne = element->linkedClone();
                                    static_cast<Spanner*>(ne)->setNoteSpan(toNote(e), newEnd);
                                    }
                              else              //couldn't find suitable start note
                                    continue;
                              }
                        else if (element->isFingering()) {
                              bool tabFingering = e->staff()->staffType(e->tick())->showTabFingering();
                              if (e->staff()->isTabStaff(e->tick()) && !tabFingering)
                                    continue;
                              ne = element->linkedClone();
                              }
                        else
                              ne = element->linkedClone();
                        }
                  ne->setScore(e->score());
                  ne->setSelected(false);
                  ne->setParent(e);
                  undo(new AddElement(ne));
#ifndef QT_NO_DEBUG
                  if (ne->isChord()) {
                        for (Note* n : toChord(ne)->notes()) {
                              Q_ASSERT(n->tpc() != Tpc::TPC_INVALID);
                        //      n->setTpcFromPitch();
                              }
                        }
#endif
                  }
            return;
            }

      if (et == ElementType::LAYOUT_BREAK) {
            LayoutBreak* lb = toLayoutBreak(element);
            if (lb->layoutBreakType() == LayoutBreak::Type::SECTION) {
                  Measure* m = lb->measure();
                  for (Score* s : scoreList()) {
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
         et    != ElementType::ARTICULATION
         && et != ElementType::CHORDLINE
         && et != ElementType::LYRICS
         && et != ElementType::SLUR
         && et != ElementType::TIE
         && et != ElementType::NOTE
         && et != ElementType::INSTRUMENT_CHANGE
         && et != ElementType::HAIRPIN
         && et != ElementType::OTTAVA
         && et != ElementType::TRILL
         && et != ElementType::TEXTLINE
         && et != ElementType::PEDAL
         && et != ElementType::BREATH
         && et != ElementType::DYNAMIC
         && et != ElementType::STAFF_TEXT
         && et != ElementType::SYSTEM_TEXT
         && et != ElementType::TREMOLO
         && et != ElementType::ARPEGGIO
         && et != ElementType::SYMBOL
         && et != ElementType::TREMOLOBAR
         && et != ElementType::FRET_DIAGRAM
         && et != ElementType::HARMONY)
            ) {
            undo(new AddElement(element));
            return;
            }

      foreach (Staff* staff, ostaff->staffList()) {
            Score* score = staff->score();
            int staffIdx = staff->idx();

            QList<int> tr;
            if (staff->score()->excerpt() && strack > -1)
                  tr = staff->score()->excerpt()->tracks().values(strack);
            else
                  tr.append(strack);

            // Some elements in voice 1 of a staff should be copied to every track which has a linked voice in this staff
            if (tr.isEmpty() && (element->isSymbol()
                || element->isImage()
                || element->isTremoloBar()
                || element->isDynamic()
                || element->isStaffText()
                || element->isFretDiagram()
                || element->isHarmony()
                || element->isHairpin()
                || element->isOttava()
                || element->isTrill()
                || element->isTextLine()
                || element->isPedal())) {
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
                                    case ElementType::STAFF_TEXT:
                                    case ElementType::SYSTEM_TEXT:
                                    case ElementType::FRET_DIAGRAM:
                                    case ElementType::HARMONY:
                                    case ElementType::FIGURED_BASS:
                                    case ElementType::DYNAMIC:
                                    case ElementType::LYRICS:   // not normally segment-attached
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

                  if (element->isArticulation()) {
                        Articulation* a  = toArticulation(element);
                        Segment* segment;
                        SegmentType st;
                        Measure* m;
                        int tick;
                        if (a->parent()->isChordRest()) {
                              ChordRest* cr = a->chordRest();
                              segment       = cr->segment();
                              st            = SegmentType::ChordRest;
                              tick          = segment->tick();
                              m             = score->tick2measure(tick);
                              }
                        else {
                              segment  = toSegment(a->parent()->parent());
                              st       = SegmentType::EndBarLine;
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
                        Articulation* na = toArticulation(ne);
                        na->setTrack(ntrack);
                        if (a->parent()->isChordRest()) {
                              ChordRest* cr = a->chordRest();
                              ChordRest* ncr;
                              if (cr->isGrace())
                                    ncr = findLinkedChord(toChord(cr), score->staff(staffIdx));
                              else
                                    ncr = toChordRest(seg->element(ntrack));
                              na->setParent(ncr);
                              }
                        else {
                              BarLine* bl = toBarLine(seg->element(ntrack));
                              na->setParent(bl);
                              }
                        undo(new AddElement(na));
                        }
                  else if (element->isChordLine() || element->isLyrics()) {
                        ChordRest* cr    = toChordRest(element->parent());
                        Segment* segment = cr->segment();
                        int tick         = segment->tick();
                        Measure* m       = score->tick2measure(tick);
                        Segment* seg     = m->findSegment(SegmentType::ChordRest, tick);
                        if (seg == 0) {
                              qWarning("undoAddSegment: segment not found");
                              break;
                              }
                        ne->setTrack(ntrack);
                        ChordRest* ncr = toChordRest(seg->element(ntrack));
                        ne->setParent(ncr);
                        undo(new AddElement(ne));
                        }
                  //
                  // elements with Segment as parent
                  //
                  else if (element->isSymbol()
                     || element->isImage()
                     || element->isTremoloBar()
                     || element->isDynamic()
                     || element->isStaffText()
                     || element->isFretDiagram()
                     || element->isHarmony()) {
                        Segment* segment = toSegment(element->parent());
                        int tick         = segment->tick();
                        Measure* m       = score->tick2measure(tick);
                        Segment* seg     = m->undoGetSegment(SegmentType::ChordRest, tick);
                        ne->setTrack(ntrack);
                        ne->setParent(seg);
                        undo(new AddElement(ne));
                        // transpose harmony if necessary
                        if (element->isHarmony() && ne != element) {
                              Harmony* h = toHarmony(ne);
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
                  else if (element->isSlur()
                     || element->isHairpin()
                     || element->isOttava()
                     || element->isTrill()
                     || element->isTextLine()
                     || element->isPedal()) {
                        Spanner* sp   = static_cast<Spanner*>(element);
                        Spanner* nsp  = static_cast<Spanner*>(ne);
                        int staffIdx1 = sp->track() / VOICES;
                        int staffIdx2 = sp->track2() / VOICES;
                        int diff      = staffIdx2 - staffIdx1;
                        nsp->setTrack2((staffIdx + diff) * VOICES + (sp->track2() % VOICES));
                        nsp->setTrack(ntrack);

                        QList<int> tl2;
                        if (staff->score()->excerpt() && element->isSlur()) {
                              nsp->setTrack(ntrack);
                                    tl2 = staff->score()->excerpt()->tracks().values(sp->track2());
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
                        if (element->isSlur() && sp != nsp) {
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
                  else if (et == ElementType::GLISSANDO)
                        undo(new AddElement(static_cast<Spanner*>(ne)));
                  else if (element->isTremolo() && toTremolo(element)->twoNotes()) {
                        Tremolo* tremolo = toTremolo(element);
                        ChordRest* cr1 = toChordRest(tremolo->chord1());
                        ChordRest* cr2 = toChordRest(tremolo->chord2());
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
                  else if (element->isTremolo() && !toTremolo(element)->twoNotes()) {
                        Chord* cr = toChord(element->parent());
                        Chord* c1 = findLinkedChord(cr, score->staff(staffIdx));
                        ne->setParent(c1);
                        undo(new AddElement(ne));
                        }
                  else if (element->isArpeggio()) {
                        ChordRest* cr = toChordRest(element->parent());
                        Segment* s    = cr->segment();
                        Measure* m    = s->measure();
                        Measure* nm   = score->tick2measure(m->tick());
                        Segment* ns   = nm->findSegment(s->segmentType(), s->tick());
                        Chord* c1     = static_cast<Chord*>(ns->element(staffIdx * VOICES + cr->voice()));
                        ne->setParent(c1);
                        undo(new AddElement(ne));
                        }
                  else if (element->isTie()) {
                        Tie* tie       = toTie(element);
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
                  else if (element->isInstrumentChange()) {
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
                  else if (element->isBreath()) {
                        Breath* breath   = static_cast<Breath*>(element);
                        int tick         = breath->segment()->tick();
                        Measure* m       = score->tick2measure(tick);
                        // breath appears before barline
                        if (m->tick() == tick)
                              m = m->prevMeasure();
                        Segment* seg     = m->undoGetSegment(SegmentType::Breath, tick);
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
      Q_ASSERT(cr->type() != ElementType::CHORD || !(static_cast<Chord*>(cr)->notes()).empty());
      Q_ASSERT(cr->isChordRest());

      Staff* ostaff = cr->staff();
      int strack = ostaff->idx() * VOICES + cr->voice();

      if (ostaff->score()->excerpt() && !ostaff->score()->excerpt()->tracks().isEmpty())
            strack = ostaff->score()->excerpt()->tracks().key(strack, -1);

      SegmentType segmentType = SegmentType::ChordRest;

      Tuplet* t = cr->tuplet();

      foreach (Staff* staff, ostaff->staffList()) {
            QList<int> tracks;
            if (staff->score()->excerpt() && !staff->score()->excerpt()->tracks().isEmpty())
                  tracks = staff->score()->excerpt()->tracks().values(strack);
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
                  if (newcr->isChord()) {
                        Chord* chord = toChord(newcr);
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
            if (e->parent() && (e->parent()->isSegment())) {
                  Segment* s = toSegment(e->parent());
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
//   undoChangeTpc
//    TODO-TPC: check
//---------------------------------------------------------

void Score::undoChangeTpc(Note* note, int v)
      {
      note->undoChangeProperty(P_ID::TPC1, v);
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
      undo(new RemoveBracket(b->staff(), b->column(), b->bracketType(), b->span()));
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
            if (!s->isChordRestType())
                  continue;
            for (int track = 0; track < ntracks(); ++track) {
                  Element* e = s->element(track);
                  if (!e || !e->isChord())
                        continue;
                  Chord* c = toChord(e);
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
//   undoChangeBarLine
//---------------------------------------------------------

void Score::undoChangeBarLine(Measure* measure, BarLineType barType, SegmentType segmentType)
      {
      int tick = measure->tick();

      for (Score* s : scoreList()) {
            Measure* m  = s->tick2measure(tick);
//            Measure* nm = m->nextMeasure();

            switch (barType) {
                  case BarLineType::END:
                  case BarLineType::NORMAL:
                  case BarLineType::DOUBLE:
                  case BarLineType::BROKEN:
                  case BarLineType::DOTTED: {
                        Segment* segment;
                        if (segmentType == SegmentType::EndBarLine) {
                              s->undoChangeProperty(m, P_ID::REPEAT_END, false);
                              // if (nm)
                              //      s->undoChangeProperty(nm, P_ID::REPEAT_START, false);
                              segment = m->findSegment(SegmentType::EndBarLine, m->endTick());
                              if (segment) {
                                    for (Element* e : segment->elist()) {
                                          if (e) {
                                                e->undoChangeProperty(P_ID::BARLINE_TYPE, QVariant::fromValue(barType));
                                                e->undoChangeProperty(P_ID::GENERATED, false);
                                                }
                                          }
                                    }
                              }
                        else if (segmentType == SegmentType::BeginBarLine) {
                              segment = m->undoGetSegment(SegmentType::BeginBarLine, tick);
                              for (Element* e : segment->elist()) {
                                    if (e) {
                                          e->undoChangeProperty(P_ID::BARLINE_TYPE, QVariant::fromValue(barType));
                                          e->undoChangeProperty(P_ID::GENERATED, false);
                                          }
                                    else {
                                          BarLine* bl = new BarLine(s);
                                          bl->setBarLineType(barType);
                                          bl->setParent(segment);
                                          bl->setTrack(0);
                                          bl->setSpanStaff(s->nstaves());
                                          undo(new AddElement(bl));
                                          }
                                    }
                              }
                        else if (segmentType == SegmentType::StartRepeatBarLine) {
                              s->undoChangeProperty(m, P_ID::REPEAT_START, false);
                              }
                        }
                        break;
                  case BarLineType::START_REPEAT:
                        s->undoChangeProperty(m, P_ID::REPEAT_START, true);
                        break;
                  case BarLineType::END_REPEAT:
                        s->undoChangeProperty(m, P_ID::REPEAT_END, true);
                        break;
                  }
            }
      }


}
