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
#include "textframe.h"
#include "accidental.h"
#include "instrchange.h"

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
//   getSelectedChordRests
//---------------------------------------------------------

QSet<ChordRest*> Score::getSelectedChordRests() const
      {
      QSet<ChordRest*> set;
      for (Element* e : selection().elements()) {
            if (e->type() == Element::Type::NOTE)
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
                  f = tuplet->baseLen().fraction() * tuplet->ratio().numerator();
                  for (DurationElement* de : tuplet->elements()) {
                        if (de->tick() >= tick)
                              break;
                        f -= de->duration();
                        }
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
                  QList<TDuration> dList = toDurationList(f, useDots);
                  if (dList.isEmpty())
                        return 0;

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
      _playChord = true;
      select(note, SelectType::SINGLE, 0);
      if (!chord->staff()->isTabStaff()) {
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
                  for (Segment* s = m->first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
                        for (int track = strack; track < etrack; ++track) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(track));
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

            int tick1 = m1->tick();
            int tick2 = m2->endTick();
            auto spanners = s->spannerMap().findContained(tick1, tick2);
            for (auto i : spanners)
                  undo(new RemoveElement(i.value));
            s->undoRemoveMeasures(m1, m2);

            Measure* nfm = 0;
            Measure* nlm = 0;
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
      if (!fill.isZero())
            undoInsertTime(lm->endTick(), fill.ticks());

      if (!range.write(rootScore(), fm->tick()))
            qFatal("Cannot write measures");
      connectTies(true);

      if (noteEntryMode()) {
            // set input cursor to possibly re-written segment
            int icTick = inputPos();
            Segment* icSegment = tick2segment(icTick, false, Segment::Type::ChordRest);
            if (!icSegment) {
                  // this can happen if cursor was on a rest
                  // and in the rewriting it got subsumed into a full measure rest
                  Measure* icMeasure = tick2measure(icTick);
                  if (!icMeasure)                     // shouldn't happen, but just in case
                        icMeasure = firstMeasure();
                  icSegment = icMeasure->first(Segment::Type::ChordRest);
                  }
            inputState().setSegment(icSegment);
            }

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
//   warnLocalTimeSig
//---------------------------------------------------------

static void warnLocalTimeSig()
      {
      if (!MScore::noGui) {
            const char* tt = QT_TRANSLATE_NOOP("addRemoveTimeSig", "MuseScore");
            const char* mt = QT_TRANSLATE_NOOP("addRemoveTimeSig", "Cannot change local time signature:\n"
               "Measure is not empty");

            QMessageBox::warning(0, qApp->translate("addRemoveTimeSig", tt),
               qApp->translate("addRemoveTimeSig", mt));
            }
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
      if (staffIdx != -1 && rootScore()->excerpts().size() > 0) {
            warnLocalTimeSig();
            return false;
            }

      //
      // split into Measure segments fm-lm
      //
      for (MeasureBase* m = fm; ; m = m->next()) {

            if (!m || !m->isMeasure() || lm->sectionBreak()
              || (static_cast<Measure*>(m)->first(Segment::Type::TimeSig) && m != fm))
                  {

                  // save section break to reinstate after rewrite
                  if (lm->sectionBreak())
                        sectionBreak = new LayoutBreak(*lm->sectionBreak());

                  if (!rewriteMeasures(fm1, lm, ns, staffIdx)) {
                        if (staffIdx >= 0) {
                              warnLocalTimeSig();
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
                              warnTupletCrossing();
                              }
                        for (Measure* m = fm1; m; m = m->nextMeasure()) {
                              if (m->first(Segment::Type::TimeSig))
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
                              sectionBreak = m->sectionBreak();
                              break;
                              }
                        m = m->next();
                        if (!m)
                              break;
                        }
                  // stop rewriting if we encountered a section break on a frame
                  // or if there is a time signature on first measure after the frame
                  if (sectionBreak || (m && static_cast<Measure*>(m)->first(Segment::Type::TimeSig)))
                        break;

                  // set up for next range to rewrite
                  fm1 = static_cast<Measure*>(m);
                  if (fm1 == 0)
                        break;
                  }

            // if we didn't break the loop already,
            // we must have an ordinary measure
            // add measure to range to rewrite
            lm = static_cast<Measure*>(m);
            }

      // if any staves don't have time signatures at the point where we stopped,
      // we need to reinstate their previous time signatures
      if (!nm)
            return true;
      Segment* s = nm->undoGetSegment(Segment::Type::TimeSig, nm->tick());
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
      Segment* seg = fm->undoGetSegment(Segment::Type::TimeSig, tick);
      TimeSig* ots = static_cast<TimeSig*>(seg->element(track));

      if (ots && (*ots == *ts)) {
            //
            //  ignore if there is already a timesig
            //  with same values
            //
            delete ts;
            return;
            }

      if (ots && ots->sig().identical(ns) && ots->stretch() == ts->stretch()) {
            ots->undoChangeProperty(P_ID::TIMESIG, QVariant::fromValue(ns));
            ots->undoChangeProperty(P_ID::GROUPS,  QVariant::fromValue(ts->groups()));
            if (ts->hasCustomText()) {
                  ots->undoChangeProperty(P_ID::NUMERATOR_STRING, ts->numeratorString());
                  ots->undoChangeProperty(P_ID::DENOMINATOR_STRING, ts->denominatorString());
                  }
            foreach (Score* score, scoreList()) {
                  Measure* fm = score->tick2measure(tick);
                  for (Measure* m = fm; m; m = m->nextMeasure()) {
                        if ((m != fm) && m->first(Segment::Type::TimeSig))
                              break;
                        bool changeActual = m->len() == m->timesig();
                        undoChangeProperty(m, P_ID::TIMESIG_NOMINAL, QVariant::fromValue(ns));
                        if (changeActual)
                              undoChangeProperty(m, P_ID::TIMESIG_ACTUAL,  QVariant::fromValue(ns));
                        // undoChangeProperty(ots, P_ID::GROUPS,  QVariant::fromValue(ts->groups()));
                        }
                  }
            int n = nstaves();
            for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
                  TimeSig* nsig = static_cast<TimeSig*>(seg->element(staffIdx * VOICES));
                  undoChangeProperty(nsig, P_ID::TIMESIG_TYPE, int(ts->timeSigType()));
                  if (ts->hasCustomText()) {
                        nsig->undoChangeProperty(P_ID::NUMERATOR_STRING, ts->numeratorString());
                        nsig->undoChangeProperty(P_ID::DENOMINATOR_STRING, ts->denominatorString());
                        }
                  }
            }
      else {
            Score* score = rootScore();
            Measure* fm  = score->tick2measure(tick);

            //
            // rewrite all measures up to the next time signature
            //
            if (fm == score->firstMeasure() && fm->nextMeasure() && (fm->len() != fm->timesig())) {
                  // handle upbeat
                  undoChangeProperty(fm, P_ID::TIMESIG_NOMINAL, QVariant::fromValue(ns));
                  Measure* m = fm->nextMeasure();
                  Segment* s = m->findSegment(Segment::Type::TimeSig, m->tick());
                  fm = s ? 0 : fm->nextMeasure();
                  }
            else {
                  if (sigmap()->timesig(seg->tick()).nominal().identical(ns)) {
                        // no change to global time signature,
                        // but we need to rewrite any staves with local time signatures
                        for (int i = 0; i < nstaves(); ++i) {
                              if (staff(i)->timeSig(tick) && staff(i)->timeSig(tick)->isLocal()) {
                                    if (!score->rewriteMeasures(fm, ns, i)) {
                                          undo()->current()->unwind();
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
                        undo()->current()->unwind();
                        return;
                        }
                  }

            // add the time signatures
            foreach (Score* score, scoreList()) {
                  Measure* nfm = score->tick2measure(tick);
                  seg   = nfm->undoGetSegment(Segment::Type::TimeSig, nfm->tick());
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
                        TimeSig* nsig = static_cast<TimeSig*>(seg->element(staffIdx * VOICES));
                        if (nsig == 0) {
                              nsig = new TimeSig(*ts);
                              nsig->setScore(score);
                              nsig->setTrack(staffIdx * VOICES);
                              nsig->setParent(seg);
                              nsig->setNeedLayout(true);
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
      if (ts->isLocal() && rootScore()->excerpts().size() > 0) {
            warnLocalTimeSig();
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
      Score* rScore = rootScore();
      Measure* rm = rScore->tick2measure(m->tick());
      Segment* rs = rm->findSegment(Segment::Type::TimeSig, s->tick());
      rScore->undoRemoveElement(rs);

      Measure* pm = m->prevMeasure();
      Fraction ns(pm ? pm->timesig() : Fraction(4,4));

      if (!rScore->rewriteMeasures(rm, ns, -1)) {
            undo()->current()->unwind();
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
                              if (cr && cr->type() == Element::Type::REST && cr->durationType() == TDuration::DurationType::V_MEASURE)
                                    cr->undoChangeProperty(P_ID::DURATION, QVariant::fromValue(nm->stretchedLen(staff(i))));
                                    //cr->setDuration(nm->stretchedLen(staff(i)));
                              }
                        }
                  }
            }
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
      if (addFlag) {
            Element* el = selection().element();
            if (el && el->type() == Element::Type::NOTE) {
                  Note* selectedNote = static_cast<Note*>(el);
                  pos.segment   = selectedNote->chord()->segment();
                  pos.staffIdx  = selectedNote->track() / VOICES;
                  ClefType clef = staff(pos.staffIdx)->clef(pos.segment->tick());
                  pos.line      = relStep(step, clef);
                  Chord* chord = static_cast<Note*>(el)->chord();
                  bool error;
                  NoteVal nval = noteValForPosition(pos, error);
                  if (error)
                        return;
                  addNote(chord, nval);
                  endCmd();
                  return;
                  }
            }
      
      pos.segment   = inputState().segment();
      pos.staffIdx  = inputState().track() / VOICES;
      ClefType clef = staff(pos.staffIdx)->clef(pos.segment->tick());
      pos.line      = relStep(step, clef);

      if (inputState().usingNoteEntryMethod(NoteEntryMethod::REPITCH))
            repitchNote(pos, !addFlag);
      else
            putNote(pos, !addFlag);
      }

//---------------------------------------------------------
//   noteValForPosition
//---------------------------------------------------------

NoteVal Score::noteValForPosition(Position pos, bool &error)
      {
      error = false;
      Segment* s = pos.segment;
      int line        = pos.line;
      int tick        = s->tick();
      int staffIdx    = pos.staffIdx;
      Staff* st       = staff(staffIdx);
      ClefType clef   = st->clef(tick);
      const Instrument* instr = st->part()->instrument(s->tick());
      NoteVal nval;
      const StringData* stringData = 0;

      switch (st->staffType()->group()) {
            case StaffGroup::PERCUSSION: {
                  if (_is.rest())
                        break;
                  const Drumset* ds = instr->drumset();
                  nval.pitch        = _is.drumNote();
                  if (nval.pitch < 0) {
                        error = true;
                        return nval;
                        }
                  nval.headGroup = ds->noteHead(nval.pitch);
                  if (nval.headGroup == NoteHead::Group::HEAD_INVALID) {
                        error = true;
                        return nval;
                        }
                  break;
                  }
            case StaffGroup::TAB: {
                  if (_is.rest()) {
                        error = true;
                        return nval;
                        }
                  stringData = instr->stringData();
                  if (line < 0 || line >= stringData->strings()) {
                        error = true;
                        return nval;
                        }
                  // build a default NoteVal for that string
                  nval.string = line;
                  if (pos.fret != FRET_NONE)          // if a fret is given, use it
                        nval.fret = pos.fret;
                  else {                              // if no fret, use 0 as default
                        _is.setString(line);
                        nval.fret = 0;
                        }
                  // reduce within fret limit
                  if (nval.fret > stringData->frets())
                        nval.fret = stringData->frets();
                  // for open strings, only accepts fret 0 (strings in StringData are from bottom to top)
                  int   strgDataIdx = stringData->strings() - line - 1;
                  if (nval.fret > 0 && stringData->stringList().at(strgDataIdx).open == true)
                        nval.fret = 0;
                  nval.pitch = stringData->getPitch(line, nval.fret, st, tick);
                  break;
                  }

            case StaffGroup::STANDARD: {
                  AccidentalVal acci = s->measure()->findAccidental(s, staffIdx, line, error);
                  if (error)
                        return nval;
                  int step           = absStep(line, clef);
                  int octave         = step/7;
                  nval.pitch         = step2pitch(step) + octave * 12 + int(acci);
                  if (styleB(StyleIdx::concertPitch))
                        nval.tpc1 = step2tpc(step % 7, acci);
                  else {
                        nval.pitch += instr->transpose().chromatic;
                        nval.tpc2 = step2tpc(step % 7, acci);
                        Interval v = st->part()->instrument(tick)->transpose();
                        if (v.isZero())
                              nval.tpc1 = nval.tpc2;
                        else
                              nval.tpc1 = Ms::transposeTpc(nval.tpc2, v, true);
                        }
                  }
                  break;
            }
      return nval;
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
//   addPitch
//---------------------------------------------------------

Note* Score::addPitch(NoteVal& nval, bool addFlag)
      {
      if (addFlag) {
            Element* el = _is.lastSegment()->element(_is.track());
            if (el == 0 || el->type() != Element::Type::CHORD) {
                  qDebug("Score::addPitch: cr %s", el ? el->name() : "zero");
                  return 0;
                  }
            Chord* c = static_cast<Chord*>(el);
            Note* note = addNote(c, nval);
            if (_is.lastSegment() == _is.segment()) {
                  NoteEntryMethod entryMethod = _is.noteEntryMethod();
                  if (entryMethod != NoteEntryMethod::REALTIME_AUTO && entryMethod != NoteEntryMethod::REALTIME_MANUAL)
                        _is.moveToNextInputPos();
                  }
            return note;
            }
      expandVoice();

      // insert note
      MScore::Direction stemDirection = MScore::Direction::AUTO;
      int track               = _is.track();
      if (_is.drumNote() != -1) {
            nval.pitch        = _is.drumNote();
            const Drumset* ds = _is.drumset();
            nval.headGroup    = ds->noteHead(nval.pitch);
            stemDirection     = ds->stemDirection(nval.pitch);
            track             = ds->voice(nval.pitch) + (_is.track() / VOICES) * VOICES;
            _is.setTrack(track);
            expandVoice();
            }
      if (!_is.cr())
            return 0;
      Fraction duration;
      if (_is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
            duration = _is.cr()->duration();
            }
      else if (_is.usingNoteEntryMethod(NoteEntryMethod::REALTIME_AUTO) || _is.usingNoteEntryMethod(NoteEntryMethod::REALTIME_MANUAL)) {
            int ticks2measureEnd = _is.segment()->measure()->ticks() - _is.segment()->rtick();
            duration = _is.duration().ticks() > ticks2measureEnd ? Fraction::fromTicks(ticks2measureEnd) : _is.duration().fraction();
            }
      else {
            duration = _is.duration().fraction();
            }
      Note* note = 0;
      Note* firstTiedNote = 0;
      Note* lastTiedNote = 0;
      if (_is.usingNoteEntryMethod(NoteEntryMethod::REPITCH) && _is.cr()->isChord()) {
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
            _playNote = true;
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
      else if (!_is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
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
      if (_is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
            // move cursor to next note, but skip tied notes (they were already repitched above)
            ChordRest* next = lastTiedNote ? nextChordRest(lastTiedNote->chord()) : nextChordRest(_is.cr());
            while (next && next->type() != Element::Type::CHORD)
                  next = nextChordRest(next);
            if (next)
                  _is.moveInputPos(next->segment());
            }
      else {
            NoteEntryMethod entryMethod = _is.noteEntryMethod();
            if (entryMethod != NoteEntryMethod::REALTIME_AUTO && entryMethod != NoteEntryMethod::REALTIME_MANUAL)
                  _is.moveToNextInputPos();
            }
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
      if (inputState().usingNoteEntryMethod(NoteEntryMethod::REPITCH))
            repitchNote(p, replace);
      else
            putNote(p, replace);
      }

void Score::putNote(const Position& p, bool replace)
      {
      int staffIdx    = p.staffIdx;
      Staff* st       = staff(staffIdx);
      Segment* s      = p.segment;

      _is.setTrack(staffIdx * VOICES + _is.voice());
      _is.setSegment(s);

      MScore::Direction stemDirection = MScore::Direction::AUTO;
      bool error;
      NoteVal nval = noteValForPosition(p, error);
      if (error)
            return;

      const StringData* stringData = 0;
      const Instrument* instr = st->part()->instrument(s->tick());
      switch (st->staffType()->group()) {
            case StaffGroup::PERCUSSION: {
                  const Drumset* ds = instr->drumset();
                  stemDirection     = ds->stemDirection(nval.pitch);
                  break;
                  }
            case StaffGroup::TAB: {
                  stringData = instr->stringData();
                  _is.setDrumNote(-1);
                  }
                  break;
            case StaffGroup::STANDARD: {
                  _is.setDrumNote(-1);
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
                                                nval.pitch = stringData->getPitch(nval.string, nval.fret, st, s->tick());
                                                }
                                          else
                                                qDebug("can't increase fret to %d", fret);
                                          }
                                    // set fret number (original or combined) in all linked notes
                                    int tpc1 = note->tpc1default(nval.pitch);
                                    int tpc2 = note->tpc2default(nval.pitch);
                                    undoChangeFretting(note, nval.pitch, nval.string, nval.fret, tpc1, tpc2);
                                    _playNote = true;
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
      bool error = false;
      AccidentalVal acci = s->measure()->findAccidental(s, p.staffIdx, p.line, error);
      if (error)
            return;
      int step   = absStep(p.line, clef);
      int octave = step / 7;
      nval.pitch = step2pitch(step) + octave * 12 + int(acci);

      if (styleB(StyleIdx::concertPitch))
            nval.tpc1 = step2tpc(step % 7, acci);
      else {
            nval.pitch += st->part()->instrument(s->tick())->transpose().chromatic;
            nval.tpc2 = step2tpc(step % 7, acci);
            }

      if (!_is.segment())
            return;

      Chord* chord;
      ChordRest* cr = _is.cr();
      if (!cr) {
            cr = _is.segment()->nextChordRest(_is.track());
            if (!cr)
                  return;
            }
      if (cr->type() == Element::Type::REST) { //skip rests
            ChordRest* next = nextChordRest(cr);
            while(next && next->type() != Element::Type::CHORD)
                  next = nextChordRest(next);
            if (next)
                  _is.moveInputPos(next->segment());
            return;
            }
      else {
            chord = static_cast<Chord*>(cr);
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
      _playNote = true;
      _playChord = true;
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
//   regroupNotesAndRests
//    * combine consecutive rests into fewer rests of longer duration.
//    * combine tied notes/chords into fewer notes of longer duration.
//    Only operates on one voice - protects manual layout adjustment, etc.
//---------------------------------------------------------

void Score::regroupNotesAndRests(int startTick, int endTick, int track)
      {
      Segment* inputSegment = _is.segment(); // store this so we can get back to it later.
      Segment* seg = tick2segment(startTick, true, Segment::Type::ChordRest);
      for (Measure* msr = seg->measure(); msr && msr->tick() < endTick; msr = msr->nextMeasure()) {
            int maxTick = endTick > msr->endTick() ? msr->endTick() : endTick;
            if (!seg || seg->measure() != msr)
                  seg = msr->first(Segment::Type::ChordRest);
            for (; seg; seg = seg->next(Segment::Type::ChordRest)) {
                  ChordRest* curr = seg->cr(track);
                  if (!curr)
                        continue; // this voice is empty here (CR overlaps with CR in other track)
                  if (seg->tick() + curr->actualTicks() > maxTick)
                        break; // outside range
                  if (curr->tuplet())
                        break; // do not use Regroup Rhythms inside a tuplet
                  if (curr->isRest()) {
                        // combine consecutive rests
                        ChordRest* lastRest = curr;
                        for (Segment* s = seg->next(Segment::Type::ChordRest); s ; s = s->next(Segment::Type::ChordRest)) {
                              ChordRest* cr = s->cr(track);
                              if (!cr)
                                    continue;  // this voice is empty here
                              if (!cr->isRest() || s->tick() + cr->actualTicks() > maxTick)
                                    break;
                              lastRest = cr;
                              }
                        int restTicks = lastRest->tick() + lastRest->duration().ticks() - curr->tick();
                        if (true)
                              seg = setNoteRest(seg, curr->track(), NoteVal(), Fraction::fromTicks(restTicks), MScore::Direction::AUTO, true);
                        }
                  else if (curr->isChord()) {
                        // combine tied chords
                        Chord* chord = static_cast<Chord*>(curr);
                        Chord* lastTiedChord = chord;
                        for (Chord* next = chord->nextTiedChord(); next && next->tick() + next->duration().ticks() <= maxTick; next = next->nextTiedChord()) {
                              lastTiedChord = next;
                              }
                        if (!lastTiedChord)
                              lastTiedChord = chord;
                        int noteTicks = lastTiedChord->tick() + lastTiedChord->duration().ticks() - chord->tick();
                        if (true) {
                              // store start/end note for backward/forward ties ending/starting on the group of notes being rewritten
                              int numNotes = chord->notes().size();
                              Note* tieBack[numNotes];
                              Note* tieFor[numNotes];
                              for (int i = 0; i < numNotes; i++) {
                                    Note* n = chord->notes()[i];
                                    Note* nn = lastTiedChord->notes()[i];
                                    if (n->tieBack())
                                          tieBack[i] = n->tieBack()->startNote();
                                    else
                                          tieBack[i] = 0;
                                    if (nn->tieFor())
                                          tieFor[i] = nn->tieFor()->endNote();
                                    else
                                          tieFor[i] = 0;
                                    }
                              int tick      = seg->tick();
                              int track     = chord->track();
                              Fraction sd   = Fraction::fromTicks(noteTicks);
                              Tie* tie      = 0;
                              Segment* segment = seg;
                              ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
                              Chord* nchord = static_cast<Chord*>(chord->clone());
                              for (int i = 0; i < numNotes; i++) { // strip ties from cloned chord
                                    Note* n = nchord->notes()[i];
                                    n->setTieFor(0);
                                    n->setTieBack(0);
                                    }
                              Chord* startChord = nchord;
                              Measure* measure = 0;
                              bool firstpart = true;
                              for (;;) {
                                    if (track % VOICES)
                                          expandVoice(segment, track);
                                    // the returned gap ends at the measure boundary or at tuplet end
                                    Fraction dd = makeGap(segment, track, sd, cr->tuplet());
                                    if (dd.isZero())
                                          break;
                                    measure = segment->measure();
                                    QList<TDuration> dl = toRhythmicDurationList(dd, false, segment->rtick(), sigmap()->timesig(tick).nominal(), measure, 1);
                                    int n = dl.size();
                                    for (int i = 0; i < n; ++i) {
                                          const TDuration& d = dl[i];
                                          Chord* nchord2 = static_cast<Chord*>(nchord->clone());
                                          if (!firstpart)
                                                nchord2->removeMarkings(true);
                                          nchord2->setDurationType(d);
                                          nchord2->setDuration(d.fraction());
                                          QList<Note*> nl1 = nchord->notes();
                                          QList<Note*> nl2 = nchord2->notes();
                                          if (!firstpart)
                                                for (int j = 0; j < nl1.size(); ++j) {
                                                      tie = new Tie(this);
                                                      tie->setStartNote(nl1[j]);
                                                      tie->setEndNote(nl2[j]);
                                                      tie->setTrack(track);
                                                      nl1[j]->setTieFor(tie);
                                                      nl2[j]->setTieBack(tie);
                                                      }
                                          undoAddCR(nchord2, measure, tick);
                                          segment = nchord2->segment();
                                          tick += nchord2->actualTicks();
                                          nchord = nchord2;
                                          firstpart = false;
                                          }
                                    sd -= dd;
                                    if (sd.isZero())
                                          break;
                                    Segment* nseg = tick2segment(tick, false, Segment::Type::ChordRest);
                                    if (nseg == 0)
                                          break;
                                    segment = nseg;
                                    cr = static_cast<ChordRest*>(segment->element(track));
                                    if (cr == 0) {
                                          if (track % VOICES)
                                                cr = addRest(segment, track, TDuration(TDuration::DurationType::V_MEASURE), 0);
                                          else
                                                break;
                                          }
                                    }
                              if (_is.slur()) {
                                    // extend slur
                                    _is.slur()->undoChangeProperty(P_ID::SPANNER_TICKS, nchord->tick() - _is.slur()->tick());
                                    for (ScoreElement* e : _is.slur()->linkList()) {
                                          Slur* slur = static_cast<Slur*>(e);
                                          for (ScoreElement* ee : nchord->linkList()) {
                                                Element* e = static_cast<Element*>(ee);
                                                if (e->score() == slur->score() && e->track() == slur->track2()) {
                                                      slur->score()->undo(new ChangeSpannerElements(slur, slur->startElement(), e));
                                                      break;
                                                      }
                                                }
                                          }
                                    }
                              // recreate previously stored pending ties
                              for (int i = 0; i < numNotes; i++) {
                                    Note* n = startChord->notes()[i];
                                    Note* nn = nchord->notes()[i];
                                    if (tieBack[i]) {
                                          tie = new Tie(this);
                                          tie->setStartNote(tieBack[i]);
                                          tie->setEndNote(n);
                                          tie->setTrack(track);
                                          n->setTieBack(tie);
                                          tieBack[i]->setTieFor(tie);
                                          undoAddElement(tie);
                                          }
                                    if (tieFor[i]) {
                                          tie = new Tie(this);
                                          tie->setStartNote(nn);
                                          tie->setEndNote(tieFor[i]);
                                          tie->setTrack(track);
                                          n->setTieFor(tie);
                                          tieFor[i]->setTieBack(tie);
                                          undoAddElement(tie);
                                          }
                                    }
                              if (tie) // at least one tie was created
                                    connectTies();
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
      QList<Note*> noteList;
      Element* el = selection().element();
      if (el && el->type() == Element::Type::NOTE) {
            Note* n = static_cast<Note*>(el);
            if (noteEntryMode())
                  noteList = n->chord()->notes();
            else
                  noteList.append(n);
            }
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
      Chord* lastAddedChord = nullptr;
      for (Note* note : noteList) {
            if (note->tieFor()) {
                  qDebug("cmdAddTie: note %p has already tie? noteFor: %p", note, note->tieFor());
                  continue;
                  }

            if (noteEntryMode()) {
                  ChordRest* cr = nullptr;
                  Chord* c = note->chord();

                  // set cursor at position after note
                  if (c->isGraceBefore()) {
                        // tie grace note before to main note
                        cr = static_cast<ChordRest*>(c->parent());
                        }
                  else {
                        // tie to next input position
                        _is.setSegment(note->chord()->segment());
                        _is.moveToNextInputPos();
                        _is.setLastSegment(_is.segment());

                        if (_is.cr() == 0)
                              expandVoice();
                        cr = _is.cr();
                        }
                  if (cr == 0)
                        break;

                  bool addFlag = lastAddedChord != nullptr;

                  // try to re-use existing note or chord
                  Note* n = nullptr;
                  if (cr->isChord()) {
                        Chord* c = static_cast<Chord*>(cr);
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
                              if (!addFlag || nnote->chord()->tick() >= lastAddedChord->tick() || nnote->chord()->isGrace()) {
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

                  for (Segment* seg = chord->segment()->next1(Segment::Type::ChordRest); seg; seg = seg->next1(Segment::Type::ChordRest)) {
                        bool noteFound = false;
                        for (int track = strack; track < etrack; ++track) {
                              Element* el = seg->element(track);
                              if (el == 0 || el->type() != Element::Type::CHORD)
                                    continue;
                              ChordRest* cr = static_cast<ChordRest*>(el);
                              int staffIdx = cr->staffIdx() + cr->staffMove();
                              if (staffIdx != chord->staffIdx() + chord->staffMove())
                                    continue;
                              foreach(Note* n, static_cast<Chord*>(cr)->notes()) {
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

void Score::cmdAddOttava(Ottava::Type type)
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
            if (cr) {
                  undoChangeProperty(cr, P_ID::BEAM_MODE, int(mode));
                  _layoutAll = true;
                  }
            }
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

      std::set<const Element*> alreadyFlippedElements;
      auto isNotFlippedElement = [&alreadyFlippedElements](const Element* element) {
          return alreadyFlippedElements.insert(element).second;
            };
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
                        if (!selection().isRange())
                              e = chord->beam();  // fall through
                        else
                              continue;
                  else {
                      if (isNotFlippedElement(chord)) {
                            MScore::Direction dir = chord->up() ? MScore::Direction::DOWN : MScore::Direction::UP;
                            undoChangeProperty(chord, P_ID::STEM_DIRECTION, int(dir));
                            }
                        }
                  }

            if (e->type() == Element::Type::BEAM) {
                  if (isNotFlippedElement(e)) {
                        Beam* beam = static_cast<Beam*>(e);
                        MScore::Direction dir = beam->up() ? MScore::Direction::DOWN : MScore::Direction::UP;
                        undoChangeProperty(beam, P_ID::STEM_DIRECTION, int(dir));
                        }
                  }
            else if (e->type() == Element::Type::SLUR_SEGMENT) {
                  if (isNotFlippedElement(e)) {
                        SlurTie* slur = static_cast<SlurSegment*>(e)->slurTie();
                        MScore::Direction dir = slur->up() ? MScore::Direction::DOWN : MScore::Direction::UP;
                        undoChangeProperty(slur, P_ID::SLUR_DIRECTION, int(dir));
                        }
                  }
            else if (e->type() == Element::Type::HAIRPIN_SEGMENT) {
                  Hairpin* h = static_cast<HairpinSegment*>(e)->hairpin();
                  Hairpin::Type st = h->hairpinType() == Hairpin::Type::CRESCENDO ? Hairpin::Type::CRESCENDO : Hairpin::Type::DECRESCENDO;
                  undoChangeProperty(h, P_ID::HAIRPIN_TYPE, int(st));
                  }
            else if (e->type() == Element::Type::ARTICULATION) {
                  if (isNotFlippedElement(e)) {
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
                        }
                  }
            else if (e->type() == Element::Type::TUPLET) {
                  if (isNotFlippedElement(e)) {
                        Tuplet* tuplet = static_cast<Tuplet*>(e);
                        MScore::Direction d = tuplet->isUp() ? MScore::Direction::DOWN : MScore::Direction::UP;
                        undoChangeProperty(tuplet, P_ID::DIRECTION, int(d));
                        }
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
      _layoutAll = true;      // must be set in undo/redo
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
                  Part* part = el->part();
                  InstrumentName* in = static_cast<InstrumentName*>(el);
                  if (in->instrumentNameType() == InstrumentNameType::LONG)
                        undo(new ChangeInstrumentLong(0, part, QList<StaffName>()));
                  else if (in->instrumentNameType() == InstrumentNameType::SHORT)
                        undo(new ChangeInstrumentShort(0, part, QList<StaffName>()));
                  }
                  break;

            case Element::Type::TIMESIG: {
                  // timesig might already be removed
                  TimeSig* ts = static_cast<TimeSig*>(el);
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

            case Element::Type::KEYSIG:
                  undoRemoveElement(el);
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
                  // fall through

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
                  rest->setDuration(rm->measure()->stretchedLen(rm->staff()));
                  rest->setTrack(rm->track());
                  rest->setParent(rm->parent());
                  Segment* segment = rm->segment();
                  undoAddCR(rest, segment->measure(), segment->tick());
                  }
                  // fall through

            case Element::Type::REST:
                  //
                  // only allow for voices != 0
                  //    e.g. voice 0 rests cannot be removed
                  //
                  {
                  Rest* rest = static_cast<Rest*>(el);
                  // delete empty tuplets
                  if (rest->tuplet() && rest->tuplet()->elements().empty())
                        undoRemoveElement(rest->tuplet());
                  // Voice > 1, do not delete rests in tuplet, turn them invisible instead
                  if (el->voice() != 0) {
                        if (rest->tuplet())
                              undoChangeProperty(el, P_ID::VISIBLE, false);
                        else {
                              undoRemoveElement(el);
                              if (noteEntryMode())
                                    _is.moveToNextInputPos();
                              }
                        }
                  else {
                        // voice = 1, do not delete rests, turn them invisible instead
                        undoChangeProperty(el, P_ID::VISIBLE, false);
                        }
                  }
                  break;

            case Element::Type::ACCIDENTAL:
                  if (el->parent()->type() == Element::Type::NOTE)
                        changeAccidental(static_cast<Note*>(el->parent()), AccidentalType::NONE);
                  else
                        undoRemoveElement(el);
                  break;

            case Element::Type::BAR_LINE:
                  {
                  BarLine* bl  = static_cast<BarLine*>(el);
                  // if system initial bar ine, route change to system first measure
                  if (bl->parent()->type() == Element::Type::SYSTEM) {
                        Measure* m = static_cast<System*>(bl->parent())->firstMeasure();
                        if (m && m->systemInitialBarLineType() != BarLineType::NORMAL)
                              m->undoChangeProperty(P_ID::SYSTEM_INITIAL_BARLINE_TYPE, int(BarLineType::NORMAL));
                        break;
                        }
                  // if regular measure bar line
                  Segment* seg   = static_cast<Segment*>(bl->parent());
                  bool normalBar = seg->measure()->endBarLineType() == BarLineType::NORMAL;
                  int tick       = seg->tick();
                  Segment::Type segType = seg->segmentType();

                  if (segType == Segment::Type::BarLine) {
                        undoRemoveElement(el);
                        break;
                        }

                  foreach(Score* score, scoreList()) {
                        Measure* m   = score->tick2measure(tick);
                        if (segType == Segment::Type::StartRepeatBarLine)
                              undoChangeProperty(m, P_ID::REPEAT_FLAGS, int(m->repeatFlags()) & ~int(Repeat::START));
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
                              // if bar line has custom span, reset to staff default
                              if (bl->customSpan() && bl->staff()) {
                                    Staff* stf = bl->staff();
                                    undoChangeProperty(bl, P_ID::BARLINE_SPAN,      stf->barLineSpan());
                                    undoChangeProperty(bl, P_ID::BARLINE_SPAN_FROM, stf->barLineFrom());
                                    undoChangeProperty(bl, P_ID::BARLINE_SPAN_TO,   stf->barLineTo());
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
                        foreach(Element* e, m->el()) {
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
//            case Element::Type::LYRICSLINE_SEGMENT:
            case Element::Type::GLISSANDO_SEGMENT:
                  {
                  el = static_cast<SpannerSegment*>(el)->spanner();
                  undoRemoveElement(el);
                  }
                  break;

            case Element::Type::STEM_SLASH:           // cannot delete this elements
            case Element::Type::HOOK:
                  qDebug("cannot remove %s", el->name());
                  break;

            case Element::Type::TEXT:
                  if (el->parent()->type() == Element::Type::TBOX)
                        undoChangeProperty(el, P_ID::TEXT, QString());
                  else
                        undoRemoveElement(el);
                  break;

            case Element::Type::INSTRUMENT_CHANGE:
                  {
                  InstrumentChange* ic = static_cast<InstrumentChange*>(el);
                  int tickStart = ic->segment()->tick();
                  Part* part = ic->part();
                  Interval oldV = part->instrument(tickStart)->transpose();
                  undoRemoveElement(el);
                  if (part->instrument(tickStart)->transpose() != oldV) {
                        auto i = part->instruments()->upper_bound(tickStart);
                        int tickEnd;
                        if (i == part->instruments()->end())
                              tickEnd = -1;
                        else
                              tickEnd = i->first;
                        transpositionChanged(part, oldV, tickStart, tickEnd);
                        }
                  }
                  break;

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
      if (is->isMeasure() && static_cast<Measure*>(is)->isMMRest())
            is = static_cast<Measure*>(is)->mmRestFirst();
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
      if (ie->isMeasure()) {
            Measure* iem = static_cast<Measure*>(ie);
            if (iem->isMMRest())
                  ie = iem = iem->mmRestLast();
            createEndBar = (iem == lastMeasureMM()) && (iem->endBarLineType() == BarLineType::END);
            }


      // get the last deleted timesig & keysig in order to restore after deletion
      TimeSig* lastDeletedSig = 0;
      KeySig* lastDeletedKeySig = 0;
      KeySigEvent lastDeletedKeySigEvent;
      bool transposeKeySigEvent = false;
      for (MeasureBase* mb = ie;; mb = mb->prev()) {
            if (mb->isMeasure()) {
                  Measure* m = static_cast<Measure*>(mb);
                  Segment* sts = m->findSegment(Segment::Type::TimeSig, m->tick());
                  if (sts && !lastDeletedSig)
                        lastDeletedSig = static_cast<TimeSig*>(sts->element(0));
                  sts = m->findSegment(Segment::Type::KeySig, m->tick());
                  if (sts && !lastDeletedKeySig) {
                        lastDeletedKeySig = static_cast<KeySig*>(sts->element(0));
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
      foreach (Score* score, scoreList()) {
            Measure* is = score->tick2measure(startTick);
            Measure* ie = score->tick2measure(endTick);

            score->undoRemoveMeasures(is, ie);

            // adjust views
            Measure* focusOn = is->prevMeasure() ? is->prevMeasure() : score->firstMeasure();
            for (MuseScoreView* v : score->viewer)
                  v->adjustCanvasPosition(focusOn, false);

            if (createEndBar) {
                  Measure* lastMeasure = score->lastMeasure();
                  if (lastMeasure && lastMeasure->endBarLineType() == BarLineType::NORMAL)
                        score->undoChangeEndBarLineType(lastMeasure, BarLineType::END);
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
            // insert correct keysig if necessary
            if (mAfterSel && !mBeforeSel && lastDeletedKeySig) {
                  Segment* s = mAfterSel->findSegment(Segment::Type::KeySig, mAfterSel->tick());
                  if (!s) {
                        Segment* ns = mAfterSel->undoGetSegment(Segment::Type::KeySig, mAfterSel->tick());
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

      select(0, SelectType::SINGLE, 0);
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
            if (ss1->segmentType() != Segment::Type::ChordRest)
                  ss1 = ss1->next1(Segment::Type::ChordRest);
            bool fullMeasure = ss1 && (ss1->measure()->first(Segment::Type::ChordRest) == ss1)
                  && (s2 == 0 || (s2->segmentType() == Segment::Type::EndBarLine));

            int tick2   = s2 ? s2->tick() : INT_MAX;
            int track1  = selection().staffStart() * VOICES;
            int track2  = selection().staffEnd() * VOICES;
            auto spanners = _spanner.findOverlapping(stick1, stick2 - 1);
            for (auto i : spanners) {
                  Spanner* sp = i.value;
                  if (sp->type() == Element::Type::VOLTA)
                        continue;
                  if (!selectionFilter().canSelectVoice(sp->track()))
                        continue;
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
                              // skip if not included in selection (eg, filter)
                              if (!selectionFilter().canSelect(annotation))
                                    continue;
                              if (!annotation->systemFlag() && annotation->track() == track)
                                    undoRemoveElement(annotation);
                              }

                        Element* e = s->element(track);
                        if (!e)
                              continue;
                        if (s->segmentType() != Segment::Type::ChordRest) {
                              // do not delete TimeSig/KeySig,
                              // it doesn't make sense to do it, except on full system
                              if (s->segmentType() != Segment::Type::TimeSig && s->segmentType() != Segment::Type::KeySig)
                                    undoRemoveElement(e);
                              continue;
                              }
                        ChordRest* cr = static_cast<ChordRest*>(e);
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
                                    Fraction ff = m->stretchedLen(staff);
                                    Rest* r = setRest(tick, track, ff, false, 0);
                                    if (!cr)
                                          cr = r;
                                    if (s2 && (m == s2->measure()))
                                          break;
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
            QList<Element*> el(selection().elements());
            if (el.isEmpty())
                  qDebug("...nothing selected");

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
                  int tick = -1;
                  int track = -1;
                  if (!cr) {
                        if (e->type() == Element::Type::NOTE)
                              tick = static_cast<Note*>(e)->chord()->tick();
                        else if (e->type() == Element::Type::REST)
                              tick = static_cast<Rest*>(e)->tick();
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
      if (_is.noteEntryMode())
            cr = _is.cr();
      if (cr) {
            if (cr->type() == Element::Type::CHORD)
                  select(static_cast<Chord*>(cr)->upNote(), SelectType::SINGLE);
            else
                  select(cr, SelectType::SINGLE);
            }

      _layoutAll = true;
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

      if (inputState().noteEntryMode()) {
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
            if (ss1 && ss1->segmentType() != Segment::Type::ChordRest)
                  ss1 = ss1->next1(Segment::Type::ChordRest);
            bool fullMeasure = ss1 && (ss1->measure()->first(Segment::Type::ChordRest) == ss1)
                  && (s2 == 0 || (s2->segmentType() == Segment::Type::EndBarLine)
                        || (s2->segmentType() == Segment::Type::TimeSigAnnounce)
                        || (s2->segmentType() == Segment::Type::KeySigAnnounce));
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
                  if (s->segmentType() != Segment::Type::ChordRest || !s->element(track))
                        continue;
                  ChordRest* cr = static_cast<ChordRest*>(s->element(track));
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
                        else if (inputState().noteEntryMode()) {
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
      if (styleB(StyleIdx::createMultiMeasureRests))
            createMMRests();
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
      pin->setBeginText(decrescendo ? "dim." : "cresc.");
      pin->setContinueText(decrescendo ? "(dim.)" : "(cresc.)");
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
      foreach(DurationElement* de, tuplet->elements()) {
            if (de->isChordRest())
                  removeChordRest(static_cast<ChordRest*>(de), true);
            else {
                  Q_ASSERT(de->type() == Element::Type::TUPLET);
                  cmdDeleteTuplet(static_cast<Tuplet*>(de), false);
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
      if (mb->isMeasure()) {
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  if (m->tick() == mb->tick())
                        return m;
                  }
            }
      else {
            if (!mb->links()) {
                  if (mb->score() == score)
                        return mb;
                  else
                        qDebug("searchMeasureBase: no links");
                  }
            else {
                  for (ScoreElement* m : *mb->links()) {
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

MeasureBase* Score::insertMeasure(Element::Type type, MeasureBase* measure, bool createEmptyMeasures, bool moveHeader)
      {
      int tick;
      int ticks = 0;
      if (measure) {
            if (measure->isMeasure() && static_cast<Measure*>(measure)->isMMRest()) {
                  measure = static_cast<Measure*>(measure)->prev();
                  measure = measure ? measure->next() : firstMeasure();
                  deselectAll();
                  }
            tick = measure->tick();
            }
      else
            tick = last() ? last()->endTick() : 0;

      // use nominal time signature of current measure
      Fraction f = sigmap()->timesig(tick).nominal();

      QList<pair<Score*, MeasureBase*>> ml;
      for (Score* score : scoreList())
            ml.append(pair<Score*, MeasureBase*>(score, searchMeasureBase(score, measure)));

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
                        else if (!lm)
                              createEndBar = true;
                        }

                  Measure* m = static_cast<Measure*>(mb);
                  Measure* mi = im ? score->tick2measure(im->tick()) : nullptr;
                  m->setTimesig(f);
                  m->setLen(f);
                  ticks = m->ticks();

                  QList<TimeSig*> tsl;
                  QList<KeySig*>  ksl;
                  QList<Clef*>    cl;
                  QList<Clef*>    pcl;

                  //
                  // remove clef, time and key signatures
                  //
                  if (mi && moveHeader) {
                        for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                              Measure* pm = mi->prevMeasure();
                              if (pm) {
                                    Segment* ps = pm->findSegment(Segment::Type::Clef, tick);
                                    if (ps) {
                                          Element* pc = ps->element(staffIdx * VOICES);
                                          if (pc) {
                                                pcl.push_back(static_cast<Clef*>(pc));
                                                undo(new RemoveElement(pc));
                                                if (ps->isEmpty())
                                                      undoRemoveElement(ps);
                                                }
                                          }
                                    }
                              for (Segment* s = mi->first(); s && s->tick() == tick; s = s->next()) {
                                    Element* e = s->element(staffIdx * VOICES);
                                    if (!e)
                                          continue;
                                    Element* ee = 0;
                                    if (e->type() == Element::Type::KEYSIG) {
                                          KeySig* ks = static_cast<KeySig*>(e);
                                          ksl.push_back(ks);
                                          ee = e;
                                          }
                                    else if (e->type() == Element::Type::TIMESIG) {
                                          TimeSig* ts = static_cast<TimeSig*>(e);
                                          tsl.push_back(ts);
                                          ee = e;
                                          }
                                    if (tick == 0) {
                                          if (e->type() == Element::Type::CLEF) {
                                                Clef* clef = static_cast<Clef*>(e);
                                                cl.push_back(clef);
                                                ee = e;
                                                }
                                          }
                                    if (ee) {
                                          undo(new RemoveElement(ee));
                                          if (s->isEmpty())
                                                undoRemoveElement(s);
                                          }
                                    }
                              }
                        }

                  m->setNext(im);
                  m->setPrev(im ? im->prev() : score->last());
                  undo(new InsertMeasures(m, m));

                  //
                  // move clef, time, key signatrues
                  //
                  for (TimeSig* ts : tsl) {
                        TimeSig* nts = new TimeSig(*ts);
                        Segment* s   = m->undoGetSegment(Segment::Type::TimeSig, tick);
                        nts->setParent(s);
                        undoAddElement(nts);
                        }
                  for (KeySig* ks : ksl) {
                        KeySig* nks = new KeySig(*ks);
                        Segment* s  = m->undoGetSegment(Segment::Type::KeySig, tick);
                        nks->setParent(s);
                        undoAddElement(nks);
                        }
                  for (Clef* clef : cl) {
                        Clef* nClef = new Clef(*clef);
                        Segment* s  = m->undoGetSegment(Segment::Type::Clef, tick);
                        nClef->setParent(s);
                        undoAddElement(nClef);
                        }
                  Measure* pm = m->prevMeasure();
                  for (Clef* clef : pcl) {
                        Clef* nClef = new Clef(*clef);
                        Segment* s  = pm->undoGetSegment(Segment::Type::Clef, tick);
                        nClef->setParent(s);
                        undoAddElement(nClef);
                        }
                  if (createEndBar)
                        m->setEndBarLineType(BarLineType::END, false);
                  }
            else {
                  // a frame, not a measure
                  if (score == rootScore())
                        rmb = mb;
                  else if (rmb && mb != rmb) {
                        mb->linkTo(rmb);
                        if (rmb->type() == Element::Type::TBOX)
                              static_cast<TBox*>(mb)->text()->linkTo(static_cast<TBox*>(rmb)->text());
                        }
                  mb->setNext(im);
                  mb->setPrev(im ? im->prev() : score->last());
                  undo(new InsertMeasures(mb, mb));
                  }
            }
      undoInsertTime(tick, ticks);

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
                        rest->setDuration(static_cast<Measure*>(omb)->len() * timeStretch);
                        rest->setTrack(track);
                        undoAddCR(rest, static_cast<Measure*>(omb), tick);
                        }
                  }
            }
      deselectAll();
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

            if (s->type() == Element::Type::SLUR) {
                  Segment* seg = tick2segmentMM(s->tick(), false, Segment::Type::ChordRest);
                  if (!seg || !seg->element(s->track())) {
                        qDebug("checkSpanner::remove (1) tick %d seg %p", s->tick(), seg);
                        sl.append(s);
                        }
                  else {
                        seg = tick2segmentMM(s->tick2(), false, Segment::Type::ChordRest);
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

}

