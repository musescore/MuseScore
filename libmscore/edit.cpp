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
#include "tablature.h"
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
            if (el->type() == Element::NOTE)
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
            if (el->type() == Element::NOTE)
                  return static_cast<Note*>(el)->chord();
            else if (el->type() == Element::REST || el->type() == Element::REPEAT_MEASURE)
                  return static_cast<Rest*>(el);
            else if (el->type() == Element::CHORD)
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
            if (e->type() == Element::NOTE)
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
                  case Element::NOTE:
                        el = el->parent();
                        // fall through
                  case Element::REPEAT_MEASURE:
                  case Element::REST:
                  case Element::CHORD:
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
      if (d.type() == TDuration::V_MEASURE)
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
      if (d.type() == TDuration::V_MEASURE)
            rest->setDuration(s->measure()->stretchedLen(staff(track/VOICES)));
      else
            rest->setDuration(d.fraction());
      rest->setTrack(track);
      rest->setParent(s);
      rest->setTuplet(tuplet);
      undoAddElement(rest);
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
            nn->setPitch(n->pitch(), n->tpc());
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

      updateAccidentals(measure, chord->staffIdx());
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
      if (cr->type() == Element::REPEAT_MEASURE)
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

Rest* Score::setRest(int tick, int track, Fraction l, bool useDots, Tuplet* tuplet)
      {
      Measure* measure = tick2measure(tick);
      Rest* r = 0;

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
               && (measure->timesig() == f)) {
                  Rest* rest = addRest(tick, track, TDuration(TDuration::V_MEASURE), tuplet);
                  tick += measure->timesig().ticks();
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
//   addNote from pitch
//---------------------------------------------------------

Note* Score::addNote(Chord* chord, int pitch)
      {
      Note* note = new Note(this);
      note->setParent(chord);
      note->setTrack(chord->track());
      note->setPitch(pitch);
      note->setTpcFromPitch();
      undoAddElement(note);
      _playNote = true;
      select(note, SELECT_SINGLE, 0);
      if (!chord->staff()->isTabStaff())
            moveToNextInputPos();
      return note;
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
      if (note->tpc() == INVALID_TPC)
            note->setTpcFromPitch();
      undoAddElement(note);
      _playNote = true;
      select(note, SELECT_SINGLE, 0);
      if (!chord->staff()->isTabStaff())
            moveToNextInputPos();
      return note;
      }

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures up to the next time signature
//    or section break
//---------------------------------------------------------

bool Score::rewriteMeasures(Measure* fm, Measure* lm, const Fraction& ns)
      {
      int measures = 1;
      for (Measure* m = fm; m != lm; m = m -> nextMeasure())
            ++measures;

      ScoreRange range;
      range.read(fm->first(), lm->last(), 0, nstaves() * VOICES);
      if (!range.canWrite(ns))
            return false;

      undoRemoveMeasures(fm, lm);
      //
      // calculate number of required measures = nm
      //
      Fraction k   = range.duration();
      k           /= ns;
      int nm       = (k.numerator() + k.denominator() - 1)/ k.denominator();

      Fraction nd(nm * ns.numerator(), ns.denominator()); // total new duration

      // evtl. we have to fill the last measure
      Fraction fill = nd - range.duration();
      if (!fill.isZero())
            range.fill(fill);

      Measure* nfm = 0;
      Measure* nlm = 0;

      // create destination measures
      int tick = 0;
      for (int i = 0; i < nm; ++i) {
            Measure* m = new Measure(this);
            m->setPrev(nlm);
            if (nlm)
                  nlm->setNext(m);
            m->setTimesig(ns);
            m->setLen(ns);
            m->setTick(tick);
            tick += m->ticks();
            nlm = m;
            if (nfm == 0)
                  nfm = m;
            }
      if (!range.write(0, nfm)) {
            qDebug("cannot write measures\n");
            abort();
            }
      nlm->setEndBarLineType(lm->endBarLineType(), lm->endBarLineGenerated(),
         lm->endBarLineVisible(), lm->endBarLineColor());

      //
      // insert new calculated measures
      //
      nfm->setPrev(fm->prev());
      nlm->setNext(lm->next());
      undo(new InsertMeasures(nfm, nlm));
      return true;
      }

//---------------------------------------------------------
//   warnTupletCrossing
//---------------------------------------------------------

static void warnTupletCrossing()
      {
      QMessageBox::warning(0,
         QT_TRANSLATE_NOOP("addRemoveTimeSig", "MuseScore"),
         QT_TRANSLATE_NOOP("addRemoveTimeSig", "cannot rewrite measures:\n"
         "tuplet would cross measure")
         );
      }

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures up to the next time signature
//---------------------------------------------------------

void Score::rewriteMeasures(Measure* fm, const Fraction& ns)
      {
      Measure* lm  = fm;
      Measure* fm1 = fm;
      //
      // split into Measure segments fm-lm
      //
      for (MeasureBase* m = fm; ; m = m->next()) {
            if (!m || (m->type() != Element::MEASURE)
              || (static_cast<Measure*>(m)->first(Segment::SegTimeSig) && m != fm))
                  {
                  if (!rewriteMeasures(fm1, lm, ns)) {
                        warnTupletCrossing();
                        for (Measure* m = fm1; m; m = m->nextMeasure()) {
                              if (m->first(Segment::SegTimeSig))
                                    break;
                              Fraction fr(ns);
                              undoChangeProperty(m, P_TIMESIG_NOMINAL, QVariant::fromValue(fr));
                              }
                        return;
                        }
                  if (!m || m->type() == Element::MEASURE)
                        break;
                  while (m->type() != Element::MEASURE) {
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
      Segment* seg = fm->undoGetSegment(Segment::SegTimeSig, tick);
      TimeSig* ots = static_cast<TimeSig*>(seg->element(track));
      if (ots) {
            //
            //  ignore if there is already a timesig
            //  with same values
            //
            if ((ots->timeSigType() == ts->timeSigType())
               && (ots->sig().identical(ts->sig()))
               && (ots->stretch() == ts->stretch())) {
                  delete ts;
                  return;
                  }
            }

      if (local) {
            ts->setParent(seg);
            ts->setTrack(track);
            ts->setStretch(ts->sig() / lsig);
            undoAddElement(ts);
            timesigStretchChanged(ts, fm, staffIdx);
            return;
            }

      foreach(Score* score, scoreList()) {
            Measure* fm = score->tick2measure(tick);
            Measure* nfm = fm;
            if (ots && ots->sig() == ts->sig() && ots->stretch() == ts->stretch()) {
                  //
                  // Set time signature of all measures up to next
                  // time signature. Do not touch measure contents.
                  //
                  for (Measure* m = fm; m; m = m->nextMeasure()) {
                        if ((m != fm) && m->first(Segment::SegTimeSig))
                              break;
                        bool changeActual = m->len() == m->timesig();
                        undoChangeProperty(m, P_TIMESIG_NOMINAL, QVariant::fromValue(ns));
                        if (changeActual)
                              undoChangeProperty(m, P_TIMESIG_ACTUAL,  QVariant::fromValue(ns));
                        }
                  }
            else {
                  //
                  // rewrite all measures up to the next time signature
                  //
                  if (fm == firstMeasure() && (fm->len() != fm->timesig())) {
                        // handle upbeat
                        undoChangeProperty(fm, P_TIMESIG_NOMINAL, QVariant::fromValue(ns));
                        Measure* m = fm->nextMeasure();
                        Segment* s = m->findSegment(Segment::SegTimeSig, m->tick());
                        if (!s) {
                              // there is something to rewrite
                              score->rewriteMeasures(fm->nextMeasure(), ns);
                              }
                        }
                  else {
                        score->rewriteMeasures(fm, ns);
                        nfm = fm->prev() ? fm->prev()->nextMeasure() : firstMeasure();
                        }
                  }

            seg   = nfm->undoGetSegment(Segment::SegTimeSig, nfm->tick());
            int n = score->nstaves();
            for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
                  TimeSig* nsig = static_cast<TimeSig*>(seg->element(staffIdx * VOICES));
                  if (nsig == 0) {
                        nsig = new TimeSig(this);
                        nsig->setTrack(staffIdx * VOICES);
                        nsig->setParent(seg);
                        nsig->setSig(ts->sig(), ts->timeSigType());
                        undoAddElement(nsig);
                        }
                  else {
                        undo(new ChangeTimesig(nsig, false, ts->sig(), ts->stretch(),
                              ts->numeratorString(), ts->denominatorString(), ts->timeSigType()));
                        nsig->setDropTarget(0);       // DEBUG
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
            if ((m != fm) && m->first(Segment::SegTimeSig))
                  break;
            int strack = staffIdx * VOICES;
            int etrack = strack + VOICES;
            for (Segment* s = m->first(Segment::SegChordRest); s; s = s->next(Segment::SegChordRest)) {
                  for (int track = strack; track < etrack; ++track) {
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (!cr)
                              continue;
                        if (cr->type() == Element::REST && cr->durationType() == TDuration::V_MEASURE)
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
      undoRemoveElement(ts->segment());

      Measure* fm = ts->measure();
      Measure* lm = fm;
      Measure* pm = fm->prevMeasure();
      Fraction ns(pm ? pm->timesig() : Fraction(4,4));
      for (Measure* m = lm; m; m = m->nextMeasure()) {
            if (m->first(Segment::SegTimeSig))
                  break;
            lm = m;
            }
      rewriteMeasures(fm, ns);
      }

//---------------------------------------------------------
//   putNote
//    mouse click in state NOTE_ENTRY
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

// qDebug("putNote at tick %d staff %d line %d clef %d currentAccidental %d",
//   tick, staffIdx, line, clef, acci);

      _is.setTrack(staffIdx * VOICES + _is.voice());
      _is.setSegment(s);

      const Instrument* instr = st->part()->instr();
      MScore::Direction stemDirection = MScore::AUTO;
      NoteVal nval;
      StringData* stringData = 0;
      StaffTypeTablature * tab = 0;

      switch(st->staffType()->group()) {
            case PERCUSSION_STAFF_GROUP: {
                  if (_is.rest)
                        break;
                  Drumset* ds   = instr->drumset();
                  nval.pitch    = _is.drumNote();
                  if (nval.pitch < 0)
                        return;
                  nval.headGroup = ds->noteHead(nval.pitch);
                  if (nval.headGroup < 0)
                        return;
                  stemDirection = ds->stemDirection(nval.pitch);
                  break;
                  }
            case TAB_STAFF_GROUP: {
                  if (_is.rest)
                        return;
                  stringData = instr->stringData();
                  tab = (StaffTypeTablature*)st->staffType();
                  int string = tab->VisualStringToPhys(line);
                  if (string < 0 || string >= stringData->strings())
                      return;
                  // build a default NoteVal for that line
                  nval.string = string;
                  if(p.fret != FRET_NONE)       // if a fret is given, use it
                        nval.fret = p.fret;
                  else {                        // if no fret, use 0 as default
                        _is.setString(line);
                        nval.fret = 0;
                        }
                  nval.pitch = stringData->getPitch(string, nval.fret);
                  break;
                  }

            case STANDARD_STAFF_GROUP: {
                  AccidentalVal acci = s->measure()->findAccidental(s, staffIdx, line);
                  int step   = absStep(line, clef);
                  int octave = step/7;
                  nval.pitch = step2pitch(step) + octave * 12 + acci;
                  nval.tpc   = step2tpc(step % 7, acci);
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
            // we need to add to current chord (otherwise, we will need to replace it or create a new onw)
            if (!replace
               && (d == _is.duration())
               && (cr->type() == Element::CHORD)
               && !_is.rest)
                  {
                  if (st->isTabStaff()) {      // TAB
                        // if a note on same string already exists, update to new pitch/fret
                        foreach(Note * note, static_cast<Chord*>(cr)->notes())
                              if(note->string() == nval.string) {       // if string is the same
                                    // if adding a new digit will keep fret number within fret limit,
                                    // add a digit to existing fret number
                                    if (stringData && tab->useNumbers() && note->fret() >= 1) {
                                          int fret = note->fret() * 10 + nval.fret;
                                          if (fret <= stringData->frets() ) {
                                                nval.fret = fret;
                                                nval.pitch = stringData->getPitch(nval.string, nval.fret);
                                                }
                                          else
                                                qDebug("can't increase fret to %d", fret);
                                          }
                                    // set fret number (orignal or combined) in all linked notes
                                    nval.tpc = pitch2tpc(nval.pitch, KEY_C, PREFER_NEAREST);
                                    foreach (Element* e, note->linkList()) {
                                          Note* linkedNote = static_cast<Note*>(e);
                                          Staff* staff = linkedNote->staff();
                                          if (staff->isTabStaff()) {
                                                (static_cast<Note*>(linkedNote))->undoChangeProperty(P_PITCH, nval.pitch);
                                                (static_cast<Note*>(linkedNote))->undoChangeProperty(P_TPC,   nval.tpc);
                                                (static_cast<Note*>(linkedNote))->undoChangeProperty(P_FRET,  nval.fret);
                                                (static_cast<Note*>(linkedNote))->undoChangeProperty(P_STRING,nval.string);
                                                }
                                          else if (staff->isPitchedStaff())
                                                undoChangePitch(linkedNote, nval.pitch, nval.tpc, linkedNote->line());
                                          }
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
      if (addToChord && cr->type() == Element::CHORD) {
            // if adding, add!
            addNote(static_cast<Chord*>(cr), nval);
            return;
            }
      else {
            // if not adding, replace current chord (or create a new one)

            if (_is.rest)
                  nval.pitch = -1;
            setNoteRest(_is.segment(), _is.track(), nval, _is.duration().fraction(), stemDirection);
            }
      if (!st->isTabStaff())
            moveToNextInputPos();
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
      nval.pitch = step2pitch(step) + octave * 12 + acci;
      nval.tpc   = step2tpc(step % 7, acci);

      Chord* chord;
      if (_is.cr()->type() == Element::REST) { //skip rests
            ChordRest* next = nextChordRest(_is.cr());
            while(next && next->type() != Element::CHORD)
                  next = nextChordRest(next);
            if(next)
                  moveInputPos(next->segment());
            return;
            }
      else {
            chord = static_cast<Chord*>(_is.cr());
            }
      Note* note = new Note(this);
      note->setNval(nval);
      note->setParent(chord);

      if (replace) {
            while (!chord->notes().isEmpty())
                  undoRemoveElement(chord->notes().first());
            }
      undoAddElement(note);
      // move to next Chord
      ChordRest* next = nextChordRest(_is.cr());
      while(next && next->type() != Element::CHORD)
            next = nextChordRest(next);
      if(next)
            moveInputPos(next->segment());
      }

//---------------------------------------------------------
//   cmdAddTie
//---------------------------------------------------------

void Score::cmdAddTie()
      {
      QList<Note*> noteList;
      Element* el = selection().element();
      if (el && el->type() == Element::NOTE)
            noteList.append(static_cast<Note*>(el));
      else if (el && el->type() == Element::STEM) {
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
            if(note->chord() &&  note->chord()->isGrace())
                  continue;
            if (note->tieFor()) {
                  qDebug("cmdAddTie: note %p has already tie? noteFor: %p", note, note->tieFor());
                  continue;
                  }
            Chord* chord  = note->chord();
            if (noteEntryMode()) {
                  if (note->chord() == _is.cr())      // if noteentry is started
                        break;

                  if (_is.cr() == 0) {
                        if (MScore::debugMode)
                              qDebug("cmdAddTie: no pos");
                        expandVoice();
                        }
                  if (_is.cr() == 0)
                        break;
                  bool addFlag = _is.cr()->type() == Element::CHORD;
                  Note* n = addPitch(note->pitch(), addFlag);
                  if (n) {
                        // n is not necessarly next note if duration span over measure
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
                        }
                  continue;
                  }
            Note* note2 = searchTieNote(note);
            Part* part = chord->staff()->part();
            int strack = part->staves()->front()->idx() * VOICES;
            int etrack = strack + part->staves()->size() * VOICES;

            for (Segment* seg = chord->segment()->next1(Segment::SegChordRest); seg; seg = seg->next1(Segment::SegChordRest)) {
                  bool noteFound = false;
                  for (int track = strack; track < etrack; ++track) {
                        ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
                        if (cr == 0 || cr->type() != Element::CHORD)
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
      endCmd();
      }

//---------------------------------------------------------
//   cmdAddHairpin
//    'H' typed on keyboard
//---------------------------------------------------------

void Score::cmdAddHairpin(bool decrescendo)
      {
      ChordRest* cr1;
      ChordRest* cr2;
      getSelectedChordRest2(&cr1, &cr2);
      if (!cr1)
            return;
      if (cr2 == 0)
            cr2 = nextChordRest(cr1);
      if (cr2 == 0)
            return;

      Hairpin* pin = new Hairpin(this);
      pin->setHairpinType(decrescendo ? Hairpin::DECRESCENDO : Hairpin::CRESCENDO);
      pin->setTrack(cr1->track());
      pin->setTick(cr1->segment()->tick());
      pin->setTick2(cr2->segment()->tick());
      undoAddElement(pin);
      if (!noteEntryMode())
            select(pin, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   cmdAddOttava
//---------------------------------------------------------

void Score::cmdAddOttava(OttavaType type)
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
      ottava->setTick(cr1->tick());
      ottava->setTick2(cr2->tick());
      undoAddElement(ottava);
      if (!noteEntryMode())
            select(ottava, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   cmdSetBeamMode
//---------------------------------------------------------

void Score::cmdSetBeamMode(BeamMode mode)
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
            if (e->type() == Element::NOTE || e->type() == Element::STEM || e->type() == Element::HOOK) {
                  Chord* chord;
                  if (e->type() == Element::NOTE)
                        chord = static_cast<Note*>(e)->chord();
                  else if (e->type() == Element::STEM)
                        chord = static_cast<Stem*>(e)->chord();
                  else if (e->type() == Element::HOOK)
                        chord = static_cast<Hook*>(e)->chord();
                  if (chord->beam())
                        e = chord->beam();  // fall trough
                  else {
                        MScore::Direction dir = chord->up() ? MScore::DOWN : MScore::UP;
                        undoChangeProperty(chord, P_STEM_DIRECTION, dir);
                        }
                  }

            if (e->type() == Element::BEAM) {
                  Beam* beam = static_cast<Beam*>(e);
                  MScore::Direction dir = beam->up() ? MScore::DOWN : MScore::UP;
                  undoChangeProperty(beam, P_STEM_DIRECTION, dir);
                  }
            else if (e->type() == Element::SLUR_SEGMENT) {
                  SlurTie* slur = static_cast<SlurSegment*>(e)->slurTie();
                  MScore::Direction dir = slur->up() ? MScore::DOWN : MScore::UP;
                  undoChangeProperty(slur, P_SLUR_DIRECTION, dir);
                  }
            else if (e->type() == Element::HAIRPIN_SEGMENT) {
                  int st = static_cast<Hairpin*>(e)->hairpinType() == 0 ? 1 : 0;
                  e->score()->undoChangeProperty(e, P_SUBTYPE, st);
                  }
            else if (e->type() == Element::ARTICULATION) {
                  Articulation* a = static_cast<Articulation*>(e);
                  if (a->articulationType() == Articulation_Staccato
                     || a->articulationType() == Articulation_Tenuto
                     || a->articulationType() == Articulation_Sforzatoaccent) {
                        ArticulationAnchor aa = a->anchor();
                        if (aa == A_TOP_CHORD)
                              aa = A_BOTTOM_CHORD;
                        else if (aa == A_BOTTOM_CHORD)
                              aa = A_TOP_CHORD;
                        else if (aa == A_CHORD)
                              aa = a->up() ? A_BOTTOM_CHORD : A_TOP_CHORD;
                        if (aa != a->anchor())
                              undoChangeProperty(a, P_ARTICULATION_ANCHOR, aa);
                        }
                  else {
                        MScore::Direction d = a->up() ? MScore::DOWN : MScore::UP;
                        undoChangeProperty(a, P_DIRECTION, d);
                        }
                  return;   // no layoutAll
                  }
            else if (e->type() == Element::TUPLET) {
                  Tuplet* tuplet = static_cast<Tuplet*>(e);
                  MScore::Direction d = tuplet->isUp() ? MScore::DOWN : MScore::UP;
                  undoChangeProperty(tuplet, P_DIRECTION, d);
                  }
            else if (e->type() == Element::NOTEDOT)
                  undo(new FlipNoteDotDirection(static_cast<Note*>(e->parent())));
            else if (
                 (e->type() == Element::TEMPO_TEXT)
               | (e->type() == Element::DYNAMIC)
               | (e->type() == Element::HAIRPIN)
               | (e->type() == Element::DYNAMIC)
               ) {
                  Element::Placement p = e->placement() == Element::ABOVE ? Element::BELOW : Element::ABOVE;
                  undoChangeProperty(e, P_PLACEMENT, p);
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
      switch(el->type()) {
            case Element::INSTRUMENT_NAME: {
                  Part* part = el->staff()->part();
                  InstrumentName* in = static_cast<InstrumentName*>(el);
                  if (in->instrumentNameType() == INSTRUMENT_NAME_LONG)
                        undo(new ChangeInstrumentLong(0, part, QList<StaffNameDoc>()));
                  else if (in->instrumentNameType() == INSTRUMENT_NAME_SHORT)
                        undo(new ChangeInstrumentShort(0, part, QList<StaffNameDoc>()));
                  }
                  break;

            case Element::TIMESIG:
                  cmdRemoveTimeSig(static_cast<TimeSig*>(el));
                  break;

            case Element::OTTAVA_SEGMENT:
            case Element::HAIRPIN_SEGMENT:
            case Element::TRILL_SEGMENT:
            case Element::TEXTLINE_SEGMENT:
            case Element::VOLTA_SEGMENT:
            case Element::SLUR_SEGMENT:
            case Element::PEDAL_SEGMENT:
                  undoRemoveElement(static_cast<SpannerSegment*>(el)->spanner());
                  break;

            case Element::NOTE:
                  {
                  Chord* chord = static_cast<Chord*>(el->parent());
                  if (chord->notes().size() > 1) {
                        undoRemoveElement(el);
                        select(chord->downNote(), SELECT_SINGLE, 0);
                        break;
                        }
                  // else fall through
                  el = chord;
                  }

            case Element::CHORD:
                  {
                  Chord* chord = static_cast<Chord*>(el);
                  removeChordRest(chord, false);

                  // replace with rest if voice 0 or if in tuplet
                  Tuplet* tuplet = chord->tuplet();
                  // if ((el->voice() == 0 || tuplet) && (chord->noteType() == NOTE_NORMAL)) {
                  if (chord->noteType() == NOTE_NORMAL) {
                        Rest* rest = new Rest(this, chord->durationType());
                        rest->setDurationType(chord->durationType());
                        rest->setDuration(chord->duration());

                        rest->setTrack(el->track());
                        rest->setParent(chord->parent());
                        Segment* segment = chord->segment();
                        undoAddCR(rest, segment->measure(), segment->tick());

                        // undoAddElement(rest);
                        if (tuplet) {
                              tuplet->add(rest);
                              rest->setTuplet(tuplet);
                              rest->setDurationType(chord->durationType());
                              }
                        select(rest, SELECT_SINGLE, 0);
                        }
                  else  {
                        // remove segment if empty
                        Segment* seg = chord->segment();
                        if (seg->isEmpty())
                              undoRemoveElement(seg);
                        }
                  }
                  break;

            case Element::REPEAT_MEASURE:
                  {
                  RepeatMeasure* rm = static_cast<RepeatMeasure*>(el);
                  removeChordRest(rm, false);
                  Rest* rest = new Rest(this);
                  rest->setDurationType(TDuration::V_MEASURE);
                  rest->setDuration(rm->measure()->len());
                  rest->setTrack(rm->track());
                  rest->setParent(rm->parent());
                  Segment* segment = rm->segment();
                  undoAddCR(rest, segment->measure(), segment->tick());
                  }

            case Element::REST:
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

            case Element::ACCIDENTAL:
                  if (el->parent()->type() == Element::NOTE)
                        changeAccidental(static_cast<Note*>(el->parent()), Accidental::ACC_NONE);
                  else
                        undoRemoveElement(el);
                  break;

            case Element::BAR_LINE:
                  {
                  BarLine* bl  = static_cast<BarLine*>(el);
                  if (bl->parent()->type() != Element::SEGMENT)
                        break;
                  Segment* seg   = static_cast<Segment*>(bl->parent());
                  bool normalBar = seg->measure()->endBarLineType() == NORMAL_BAR;
                  int tick       = seg->tick();
                  Segment::SegmentType segType = seg->segmentType();

                  foreach(Score* score, scoreList()) {
                        Measure* m   = score->tick2measure(tick);
                        if (segType == Segment::SegStartRepeatBarLine)
                              undoChangeProperty(m, P_REPEAT_FLAGS, m->repeatFlags() & ~RepeatStart);
                        else if (segType == Segment::SegBarLine)
                              undoRemoveElement(el);
                        else if (segType == Segment::SegEndBarLine) {
                              // if bar line has custom barLineType, change to barLineType of the whole measure
                              if (bl->customSubtype()) {
                                    undoChangeProperty(bl, P_SUBTYPE, seg->measure()->endBarLineType());
                                    }
                              // otherwise, if whole measure has special end bar line, change to normal
                              else if (!normalBar) {
                                    if (m->tick() >= tick)
                                          m = m->prevMeasure();
                                    undoChangeProperty(m, P_REPEAT_FLAGS, m->repeatFlags() & ~RepeatEnd);
                                    Measure* nm = m->nextMeasure();
                                    if (nm)
                                          undoChangeProperty(nm, P_REPEAT_FLAGS, nm->repeatFlags() & ~RepeatStart);
                                    undoChangeEndBarLineType(m, NORMAL_BAR);
                                    m->setEndBarLineGenerated(true);
                                    }
                              }
                        }
                  }
                  break;

            case Element::TUPLET:
                  cmdDeleteTuplet(static_cast<Tuplet*>(el), true);
                  break;

            case Element::MEASURE:
                  undoRemoveMeasures(static_cast<Measure*>(el), static_cast<Measure*>(el));
                  break;

            case Element::BRACKET:
                  undoRemoveBracket(static_cast<Bracket*>(el));
                  break;

            case Element::LAYOUT_BREAK:
                  {
                  undoRemoveElement(el);
                  LayoutBreak* lb = static_cast<LayoutBreak*>(el);
                  Measure* m = lb->measure();
                  if (m->isMMRest()) {
                        // propagate to original measure
                        m = static_cast<Measure*>(m->next()->prev());
                        foreach(Element* e, *m->el()) {
                              if (e->type() == Element::LAYOUT_BREAK) {
                                    undoRemoveElement(e);
                                    break;
                                    }
                              }
                        }
                  }
                  break;

            case Element::CLEF:
                  {
                  undoRemoveElement(el);
                  Clef* clef = static_cast<Clef*>(el);
                  Measure* m = clef->measure();
                  if (m->isMMRest()) {
                        // propagate to original measure
                        m = static_cast<Measure*>(m->next()->prev());
                        Segment* s = m->findSegment(Segment::SegClef, clef->segment()->tick());
                        if (s && s->element(clef->track()))
                              undoRemoveElement(s->element(clef->track()));
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
      if (selection().state() != SEL_RANGE)
            return;

      MeasureBase* is   = selection().startSegment()->measure();
      int startIdx      = measureIdx(is);
      Segment* seg      = selection().endSegment();
      MeasureBase* ie;
      // choose the correct last measure based on the end segment
      // this depends on whether a whole measure is selected or only a few notes within it
      if (seg)
            ie = !seg->prev() ? seg->measure()->prev() : seg->measure();
      else
            ie = lastMeasure();
      int endIdx        = measureIdx(ie);
      Measure* mBeforeSel = is->prevMeasure();

      // createEndBar if last measure is deleted
      bool createEndBar = false;
      if (ie->type() == Element::MEASURE) {
            Measure* iem = static_cast<Measure*>(ie);
            createEndBar = (iem == lastMeasure()) && (iem->endBarLineType() == END_BAR);
            }

      // get the last deleted timesig in order to restore after deletion
      TimeSig* lastDeletedSig = 0;
      for (MeasureBase* mb = ie;; mb = mb->prev()) {
            if (mb->type() == Element::MEASURE) {
                  Measure* m = static_cast<Measure*>(mb);
                  Segment* sts = m->findSegment(Segment::SegTimeSig, m->tick());
                  if (sts) {
                        lastDeletedSig = static_cast<TimeSig*>(sts->element(0));
                        break;
                        }
                  }

            if (mb == is)
                  break;
            }

      QList<Score*> scores = scoreList();
      int startTick = measure(startIdx)->tick();
      int endTick   = measure(endIdx)->tick();
      foreach (Score* score, scores) {
            Measure* is = score->tick2measure(startTick);
            Measure* ie = score->tick2measure(endTick);
            mBeforeSel = is->prevMeasure();

            int ticks = 0;
            for (Measure* m = is; m; m = m->nextMeasure()) {
                  ticks += m->ticks();
                  if (m == ie)
                        break;
                  }

#if 0
            // remove spanner
            std::list<Spanner*> sl;
            int tick2 = startTick + ticks;
            for (auto i : score->_spanner.map()) {
                  Spanner* s = i.second;
                  if (s->tick() >= startTick && s->tick() < tick2)
                        sl.push_back(s);
                  }
            for (Spanner* s : sl)
                  score->undoRemoveElement(s);
#endif
            undoRemoveMeasures(is, ie);

            if (createEndBar) {
                  Measure* lastMeasure = score->lastMeasure();
                  if (lastMeasure && lastMeasure->endBarLineType() == NORMAL_BAR)
                        undoChangeEndBarLineType(lastMeasure, END_BAR);
                  }

            // insert correct timesig after deletion
            Measure* mAfterSel = mBeforeSel ? mBeforeSel->nextMeasure() : firstMeasure();
            if (mAfterSel && lastDeletedSig) {
                  bool changed = true;
                  if (mBeforeSel) {
                        if (mBeforeSel->timesig() == mAfterSel->timesig()) {
                              changed = false;
                              }
                        }
                  Segment* s = mAfterSel->findSegment(Segment::SegTimeSig, mAfterSel->tick());
                  if (!s && changed) {
                        Segment* ns = mAfterSel->undoGetSegment(Segment::SegTimeSig, mAfterSel->tick());
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

      select(0, SELECT_SINGLE, 0);
      _is.setSegment(0);        // invalidate position
      }

//---------------------------------------------------------
//   cmdDeleteSelection
//---------------------------------------------------------

void Score::cmdDeleteSelection()
      {
      if (selection().state() == SEL_RANGE) {
            Segment* s1 = selection().startSegment();
            Segment* s2 = selection().endSegment();
            int stick1 = s1->tick();
            int stick2 = s2->tick();

            Segment* ss1 = s1;
            if (ss1->segmentType() != Segment::SegChordRest)
                  ss1 = ss1->next1(Segment::SegChordRest);
            bool fullMeasure = ss1 && (ss1->measure()->first(Segment::SegChordRest) == ss1)
                  && (s2 == 0 || (s2->segmentType() == Segment::SegEndBarLine));

            int tick2   = s2 ? s2->tick() : INT_MAX;
            int track1  = selection().staffStart() * VOICES;
            int track2  = selection().staffEnd() * VOICES;
            for (int track = track1; track < track2; ++track) {
                  Fraction f;
                  int tick  = -1;
                  Tuplet* tuplet = 0;
                  for (Segment* s = s1; s != s2; s = s->next1()) {
                        if (s->element(track) && s->segmentType() == Segment::SegBreath) {
                              deleteItem(s->element(track));
                              continue;
                              }
                        if (s->segmentType() != Segment::SegChordRest || !s->element(track))
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
                        if (fullMeasure) {
                              // handle this as special case to be able to
                              // fix broken measures:
                              for (Measure* m = s1->measure(); m; m = m->nextMeasure()) {
                                    setRest(m->tick(), track, Fraction(m->len()), false, 0);
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
            s2 = tick2segment(stick2);
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
            if (!noteEntryMode())   // in entry mode deleting note or chord add rest selected
                  deselectAll();
            }
      _layoutAll = true;
      }

//---------------------------------------------------------
//   addLyrics
//    called from Keyboard Accelerator & menue
//---------------------------------------------------------

Lyrics* Score::addLyrics()
      {
      Element* el = selection().element();
      if (el == 0 || (el->type() != Element::NOTE && el->type() != Element::LYRICS)) {
            QMessageBox::information(0,
               QMessageBox::tr("MuseScore:"),
               QMessageBox::tr("No note or lyrics selected:\n"
                  "Please select a single note or lyrics and retry operation\n"),
               QMessageBox::Ok, QMessageBox::NoButton);
            return 0;
            }
      ChordRest* cr;
      if (el->type() == Element::NOTE) {
            cr = static_cast<Note*>(el)->chord();
            if(cr->isGrace())
                  cr = static_cast<ChordRest*>(cr->parent());
            }
      else if (el->type() == Element::LYRICS)
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
      select(lyrics, SELECT_SINGLE, 0);
      return lyrics;
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
      if (ocr->type() == Element::CHORD) {
            cr = new Chord(this);
            foreach(Note* oldNote, static_cast<Chord*>(ocr)->notes()) {
                  Note* note = new Note(this);
                  note->setPitch(oldNote->pitch(), oldNote->tpc());
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
                  undoChangeProperty(e, P_COLOR, c);
                  e->setGenerated(false);
                  refresh |= e->abbox();
                  if (e->type() == Element::BAR_LINE) {
                        Element* ep = e->parent();
                        if (ep->type() == Element::SEGMENT && static_cast<Segment*>(ep)->segmentType() == Segment::SegEndBarLine) {
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
      if (selection().state() != SEL_RANGE) {
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
      setNoteRest(_is.segment(), track, nval, d.fraction(), MScore::AUTO);
      moveToNextInputPos();
      _is.rest = false;  // continue with normal note entry
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
      undoRemoveElement(cr);
      if (clearSegment) {
            Segment* seg = cr->segment();
            if (seg->isEmpty())
                  undoRemoveElement(seg);
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
                  Q_ASSERT(de->type() == Element::TUPLET);
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
            Segment* s = tick2segment(cr->tick() + cr->actualTicks());
            int track = (cr->track() / VOICES) * VOICES;
            ncr = s ? static_cast<ChordRest*>(s->element(track)) : 0;
            }
      _is.setSegment(ncr ? ncr->segment() : 0);
      if (doSelect)
            select(ncr, SELECT_SINGLE, 0);
//      if (ncr)
//            emit posChanged(ncr->tick());
      }

//---------------------------------------------------------
//   insertMeasure
//    Create a new MeasureBase of type type and insert
//    before measure.
//    If measure is zero, append new MeasureBase.
//---------------------------------------------------------

MeasureBase* Score::insertMeasure(Element::ElementType type, MeasureBase* measure,
   bool createEmptyMeasures)
      {
      int tick;
      int idx;
      if (measure) {
            if (measure->type() == Element::MEASURE && static_cast<Measure*>(measure)->isMMRest()) {
                  measure = static_cast<Measure*>(measure)->prev();
                  measure = measure ? measure->next() : firstMeasure();
                  deselectAll();
                  }
            tick = measure->tick();
            idx  = measureIdx(measure);
            }
      else {
            measure = last();
            if (measure)
                  tick = measure->tick() + measure->ticks();
            else
                  tick = 0;
            idx  = -1;
            }

      MeasureBase* omb = 0;
      QList<Score*> scorelist;
      if (type == Element::MEASURE)
            scorelist = scoreList();
      else
            scorelist.append(this);

      foreach(Score* score, scorelist) {
            MeasureBase* mb = static_cast<MeasureBase*>(Element::create(type, score));
            MeasureBase* im = idx != -1 ? score->measure(idx) : 0;
            // insert before im, append if im = 0
            measure = mb;
            mb->setTick(tick);
            if (score == this)
                  omb = mb;

            if (type == Element::MEASURE) {
                  bool createEndBar    = false;
                  bool endBarGenerated = false;
                  if (idx == -1) {
                        Measure* lm = score->lastMeasure();
                        if (lm && lm->endBarLineType() == END_BAR) {
                              createEndBar = true;
                              if (!lm->endBarLineGenerated()) {
                                    score->undoChangeEndBarLineType(lm, NORMAL_BAR);
                                    }
                              else {
                                    endBarGenerated = true;
                                    lm->setEndBarLineType(NORMAL_BAR, true);
                                    }
                              }
                        else if (lm == 0)
                              createEndBar = true;
                        }

                  // use nominal time signature of previous measure
                  Fraction f = score->sigmap()->timesig(tick - 1).nominal();

                  Measure* m = static_cast<Measure*>(mb);
                  m->setTimesig(f);
                  m->setLen(f);

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
                                    if (e == 0)
                                          continue;
                                    if (e->type() == Element::KEYSIG) {
                                          KeySig* ks = static_cast<KeySig*>(e);
                                          ksl.append(ks);
                                          undo(new RemoveElement(ks));
                                          if (ks->segment()->isEmpty())
                                                undoRemoveElement(ks->segment());
                                          }
                                    else if (e->type() == Element::TIMESIG) {
                                          TimeSig* ts = static_cast<TimeSig*>(e);
                                          tsl.append(ts);
                                          undo(new RemoveElement(ts));
                                          if (ts->segment()->isEmpty())
                                                undoRemoveElement(ts->segment());
                                          }
                                    else if (e->type() == Element::CLEF) {
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
                        Segment* s   = m->undoGetSegment(Segment::SegTimeSig, 0);
                        nts->setParent(s);
                        undoAddElement(nts);
                        }
                  foreach(KeySig* ks, ksl) {
                        KeySig* nks = new KeySig(*ks);
                        Segment* s  = m->undoGetSegment(Segment::SegKeySig, 0);
                        nks->setParent(s);
                        undoAddElement(nks);
                        }
                  foreach(Clef* clef, cl) {
                        Clef* nClef = new Clef(*clef);
                        Segment* s  = m->undoGetSegment(Segment::SegClef, 0);
                        nClef->setParent(s);
                        undoAddElement(nClef);
                        }
                  if (createEndBar) {
                        Measure* lm = score->lastMeasure();
                        if (lm)
                              lm->setEndBarLineType(END_BAR, endBarGenerated);
                        }
                  score->fixTicks();
                  }
            else {
                  undo(new InsertMeasure(mb, im));
                  }
            }
      insertTime(tick, measure->ticks());

      if (type == Element::MEASURE && !createEmptyMeasures) {
            //
            // fill measure with rest
            //
            Score*   root = rootScore();
            Measure* m = static_cast<Measure*>(omb);
            for (int staffIdx = 0; staffIdx < root->nstaves(); ++staffIdx) {
                  int track = staffIdx * VOICES;
                  Segment* s = m->findSegment(Segment::SegChordRest, m->tick());
                  if (s == 0 || s->element(track) == 0) {
                        // add rest to this staff and to all the staves linked to it
                        Rest* rest = new Rest(root, TDuration(TDuration::V_MEASURE));
                        Fraction timeStretch(root->staff(staffIdx)->timeStretch(m->tick()));
                        rest->setDuration(m->len() / timeStretch);
                        rest->setTrack(track);
                        undoAddCR(rest, m, m->tick());
                        }
                  }
            }
      return omb;
      }

}

