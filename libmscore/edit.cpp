//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: edit.cpp 5585 2012-04-28 09:11:33Z wschweer $
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
#include "slurmap.h"
#include "tiemap.h"
#include "stem.h"
#include "iname.h"
#include "range.h"
#include "hook.h"

//---------------------------------------------------------
//   getSelectedNote
//---------------------------------------------------------

Note* Score::getSelectedNote()
      {
      Element* el = selection().element();
      if (el) {
            if (el->type() == NOTE)
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
            if (el->type() == NOTE)
                  return static_cast<Note*>(el)->chord();
            else if (el->type() == REST || el->type() == REPEAT_MEASURE)
                  return static_cast<Rest*>(el);
            else if (el->type() == CHORD)
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
            if (e->type() == NOTE)
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
                  case NOTE:
                        el = el->parent();
                        // fall through
                  case REPEAT_MEASURE:
                  case REST:
                  case CHORD:
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
      if (cr->type() == REPEAT_MEASURE)
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

            qDebug("set rest %d/%d  -> measure %d/%d",
               f.numerator(), f.denominator(),
               measure->timesig().numerator(), measure->timesig().denominator()
               );

            if ((measure->timesig() == measure->len())   // not in pickup measure
               && (measure->tick() == tick)
               // && ((measure->timesig() / timeStretch) == f)
               && (measure->timesig() == f)) {
//  removed to have measures a breve long or more cleared to measure rest instead of actual value rest(s)
//               && (f < TDuration(TDuration::V_BREVE).fraction())
                  Rest* rest = addRest(tick, track, TDuration(TDuration::V_MEASURE), tuplet);
                  tick += measure->timesig().ticks();
                  if (r == 0)
                        r = rest;
                  }
            else {
                  //
                  // compute list of durations which will fit l
                  //

                  Fraction ff = f / staff(track/VOICES)->timeStretch(tick);
qDebug(" create duration list from %d/%d", ff.numerator(), ff.denominator());
                  QList<TDuration> dList = toDurationList(ff, useDots);
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
//   addNote
//---------------------------------------------------------

Note* Score::addNote(Chord* chord, int pitch)
      {
      Note* note = new Note(this);
      note->setParent(chord);
      note->setTrack(chord->track());
      note->setPitch(pitch);
      note->setTpcFromPitch();
      undoAddElement(note);
      select(note, SELECT_SINGLE, 0);
      return note;
      }

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures up to the next time signature
//    or section break
//---------------------------------------------------------

bool Score::rewriteMeasures(Measure* fm, Measure* lm, const Fraction& ns)
      {
      ScoreRange range;
      range.read(fm->first(), lm->last(), 0, nstaves() * VOICES);
      if (!range.canWrite(ns))
            return false;

      undo(new RemoveMeasures(fm, lm));
      //
      // calculate number of required measures = nm
      //
      Fraction k   = range.duration();
      k           /= ns;
      int nm       = (k.numerator() + k.denominator() - 1)/ k.denominator();

      Measure* nfm = 0;
      Measure* nlm = 0;

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
            if (!m || (m->type() != MEASURE)
              || (static_cast<Measure*>(m)->first(SegTimeSig) && m != fm))
                  {
                  if (!rewriteMeasures(fm1, lm, ns)) {
                        warnTupletCrossing();
                        for (Measure* m = fm1; m; m = m->nextMeasure()) {
                              if (m->first(SegTimeSig))
                                    break;
                              Fraction fr(ns);
                              undoChangeProperty(m, P_TIMESIG_NOMINAL, QVariant::fromValue(fr));
                              }
                        return;
                        }
                  if (!m || m->type() == MEASURE)
                        break;
                  while (m->type() != MEASURE)
                        m = m->next();
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

void Score::cmdAddTimeSig(Measure* fm, int staffIdx, TimeSig* ts)
      {
      Fraction ns  = ts->sig();
      int tick     = fm->tick();
      TimeSig* lts = staff(staffIdx)->timeSig(tick);
      Fraction stretch, lsig;
      if (lts) {
            stretch = lts->stretch();
            lsig    = lts->sig();
            }
      else {
            stretch.set(1,1);
            lsig.set(4,4);
            }

      int track    = staffIdx * VOICES;
      Segment* seg = fm->getSegment(SegTimeSig, tick);
      TimeSig* ots = static_cast<TimeSig*>(seg->element(track));
      if (ots) {
            //
            //  ignore if there is already a timesig
            //  with same values
            //
            if ((ots->subtype() == ts->subtype())
               && (ots->sig().identical(ts->sig()))
               && (ots->stretch() == ts->stretch())) {
                  delete ts;
                  return;
                  }
            }
      else {
            //
            //  check for local timesig (only staff value changes)
            //  or redundant time signature
            //
            if (lsig == ts->sig()) {
                  ts->setParent(seg);
                  ts->setTrack(track);
                  undoAddElement(ts);
//TODO                  timesigStretchChanged(ts, fm, staffIdx);
                  return;
                  }
            }
      Measure* nfm = fm;
      if (ots && ots->sig() == ts->sig() && ots->stretch() == ts->stretch()) {
            //
            // Set time signature of all measures up to next
            // time signature. Do not touch measure contents.
            //
            for (Measure* m = fm; m; m = m->nextMeasure()) {
                  if ((m != fm) && m->first(SegTimeSig))
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
                  rewriteMeasures(fm->nextMeasure(), ns);
                  }
            else {
                  rewriteMeasures(fm, ns);
                  nfm = fm->prev() ? fm->prev()->nextMeasure() : firstMeasure();
                  }
            }

      seg   = nfm->undoGetSegment(SegTimeSig, nfm->tick());
      int n = _staves.size();
      for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
            TimeSig* nsig = static_cast<TimeSig*>(seg->element(staffIdx * VOICES));
            if (nsig == 0) {
                  nsig = new TimeSig(this);
                  nsig->setTrack(staffIdx * VOICES);
                  nsig->setParent(seg);
                  nsig->setSubtype(ts->subtype());
                  nsig->setSig(ts->sig());
                  undoAddElement(nsig);
                  }
            else {
                  undo(new ChangeTimesig(nsig, false,
                     ts->sig(), nsig->stretch(), ts->subtype(),
                     QString(), QString()));
                  nsig->setDropTarget(0);       // DEBUG
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
            if ((m != fm) && m->first(SegTimeSig))
                  break;
            int strack = staffIdx * VOICES;
            int etrack = strack + VOICES;
            for (Segment* s = m->first(SegChordRest); s; s = s->next(SegChordRest)) {
                  for (int track = strack; track < etrack; ++track) {
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (!cr)
                              continue;
                        if (cr->type() == REST && cr->durationType() == TDuration::V_MEASURE) {
                              cr->setDuration(ts->actualSig());
                              }
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
            if (m->first(SegTimeSig))
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

      Segment* s      = p.segment;
      int tick        = s->tick();
      int staffIdx    = p.staffIdx;
      int line        = p.line;
      Staff* st       = staff(staffIdx);
      KeySigEvent key = st->keymap()->key(tick);
      int clef        = st->clef(tick);

qDebug("putNote at tick %d staff %d line %d key %d clef %d",
   tick, staffIdx, line, key.accidentalType(), clef);

      _is.setTrack(staffIdx * VOICES + _is.voice());
      _is.setSegment(s);
      const Instrument* instr = st->part()->instr();
      Direction stemDirection = AUTO;
      NoteVal nval;

      switch(st->staffType()->group()) {
            case PERCUSSION_STAFF: {
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
            case TAB_STAFF: {
                  Tablature* neck = instr->tablature();
                  StaffTypeTablature * tab = (StaffTypeTablature*)st->staffType();
                  // if tablature is upside down, 'flip' string number
                  int string = tab->upsideDown() ? (tab->lines() - line - 1) : line;
                  // check the chord does not already contains a note on the same string
                  ChordRest* cr = _is.cr();
                  if(cr != 0 && cr->type() == CHORD)
                        foreach(Note * note, static_cast<Chord*>(cr)->notes())
                              if(note->string() == string)  // if line is the same
                                    return;                 // do nothing
                  // build a default NoteVal for that line
                  nval.pitch     = neck->getPitch(string, 0);
                  nval.fret      = 0;
                  nval.string    = string;
                  break;
                  }

            case PITCHED_STAFF:
                  nval.pitch = line2pitch(line, clef, key.accidentalType());
                  break;
            }

      _is.pitch = nval.pitch;
      expandVoice();
      ChordRest* cr = _is.cr();
      bool addToChord = false;

      if (cr) {
            TDuration d = cr->durationType();
            Note* note = 0;
            if (cr->type() == CHORD) {
                  Fraction f = cr->duration();
                  note = static_cast<Chord*>(cr)->upNote();
                  if (note) {
                        Note* note2 = note;
                        while (note2->tieFor() && note2->tieFor()->endNote()) {
                              note2 = note2->tieFor()->endNote();
                              f += note2->chord()->duration();
                              }
                        TDuration dd(f);
                        if (dd.isValid())
                              d = dd;
                        }
                  else
                        qDebug("note not found: %d!", nval.pitch);
                  }
            if (!replace
               && (d == _is.duration())
               && (cr->type() == CHORD)
               && !_is.rest)
                  {
                  Chord* chord = static_cast<Chord*>(cr);
                  note = chord->findNote(nval.pitch);
                  if (note) {
                        // remove note from chord
                        if (chord->notes().size() > 1)
                              undoRemoveElement(note);
                        return;
                        }
                  addToChord = true;
                  }
            }
      if (addToChord && cr->type() == CHORD)
            addNote(static_cast<Chord*>(cr), nval.pitch);
      else {
            // replace chord
            if (_is.rest)
                  nval.pitch = -1;
            setNoteRest(_is.segment(), _is.track(), nval, _is.duration().fraction(), stemDirection);
            }
      moveToNextInputPos();
      }

//---------------------------------------------------------
//   cmdAddTie
//---------------------------------------------------------

void Score::cmdAddTie()
      {
      QList<Note*> noteList;
      Element* el = selection().element();
      if (el && el->type() == NOTE)
            noteList.append(static_cast<Note*>(el));
      else if (el && el->type() == STEM) {
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
            if (note->tieFor()) {
                  qDebug("cmdAddTie: has already tie? noteFor: %p", note->tieFor());
                  continue;
                  }
            Chord* chord  = note->chord();
            if (noteEntryMode()) {
                  if (_is.cr() == 0) {
                        if (MScore::debugMode)
                              qDebug("cmdAddTie: no pos");
                        expandVoice();
                        }
                  if (_is.cr() == 0)
                        break;
                  bool addFlag = _is.cr()->type() == CHORD;
                  Note* n = addPitch(note->pitch(), addFlag);
                  if (n) {
                        n->setLine(note->line());
                        n->setTpc(note->tpc());
                        Tie* tie = new Tie(this);
                        tie->setStartNote(note);
                        tie->setEndNote(n);
                        tie->setTrack(note->track());
                        note->setTieFor(tie);
                        n->setTieBack(tie);
                        undoAddElement(tie);
                        nextInputPos(n->chord(), false);
                        }
                  continue;
                  }
            Note* note2 = searchTieNote(note);
            Part* part = chord->staff()->part();
            int strack = part->staves()->front()->idx() * VOICES;
            int etrack = strack + part->staves()->size() * VOICES;

            for (Segment* seg = chord->segment()->next1(SegChordRest); seg; seg = seg->next1(SegChordRest)) {
                  bool noteFound = false;
                  for (int track = strack; track < etrack; ++track) {
                        ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
                        if (cr == 0 || cr->type() != CHORD)
                              continue;
                        int staffIdx = cr->staffIdx() + cr->staffMove();
                        if (staffIdx != chord->staffIdx())
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
            Tie* tie = new Tie(this);
            tie->setStartNote(note);
            tie->setEndNote(note2);
            tie->setTrack(note->track());
            undoAddElement(tie);
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
      pin->setSubtype(decrescendo ? 1 : 0);
      pin->setTrack(cr1->track());
      pin->setStartElement(cr1->segment());
      pin->setEndElement(cr2->segment());
      pin->setParent(cr1->segment());
      undoAddElement(pin);
      if (!noteEntryMode())
            select(pin, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   cmdSetBeamMode
//---------------------------------------------------------

void Score::cmdSetBeamMode(int mode)
      {
      ChordRest* cr = getSelectedChordRest();
      if (cr == 0)
            return;
      cr->setBeamMode(BeamMode(mode));
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
            if (e->type() == NOTE || e->type() == STEM || e->type() == HOOK) {
                  Chord* chord = static_cast<Note*>(e)->chord();
                  if (e->type() == STEM)
                        chord = static_cast<Stem*>(e)->chord();
                  else if (e->type() == HOOK)
                        chord = static_cast<Hook*>(e)->chord();
                  if (chord->beam())
                        e = chord->beam();  // fall trough
                  else {
                        Direction dir = chord->up() ? DOWN : UP;
                        undoChangeProperty(chord, P_STEM_DIRECTION, dir);
                        }
                  }
            if (e->type() == BEAM) {
                  Beam* beam = static_cast<Beam*>(e);
                  Direction dir = beam->up() ? DOWN : UP;
                  undoChangeProperty(beam, P_STEM_DIRECTION, dir);
                  }
            else if (e->type() == SLUR_SEGMENT) {
                  SlurTie* slur = static_cast<SlurSegment*>(e)->slurTie();
                  Direction dir = slur->up() ? DOWN : UP;
                  undoChangeProperty(slur, P_SLUR_DIRECTION, dir);
                  }
            else if (e->type() == HAIRPIN_SEGMENT) {
                  int st = static_cast<Hairpin*>(e)->subtype() == 0 ? 1 : 0;
                  e->score()->undoChangeProperty(e, P_SUBTYPE, st);
                  }
            else if (e->type() == ARTICULATION) {
                  Articulation* a = static_cast<Articulation*>(e);
                  if (a->subtype() == Articulation_Staccato
                     || a->subtype() == Articulation_Tenuto
                     || a->subtype() == Articulation_Sforzatoaccent) {
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
                        Direction d = a->up() ? DOWN : UP;
                        undoChangeProperty(a, P_DIRECTION, d);
                        }
                  return;   // no layoutAll
                  }
            else if (e->type() == TUPLET) {
                  Tuplet* tuplet = static_cast<Tuplet*>(e);
                  Direction d = tuplet->isUp() ? DOWN : UP;
                  undoChangeProperty(tuplet, P_DIRECTION, d);
                  }
            else if (e->type() == NOTEDOT)
                  undo(new FlipNoteDotDirection(static_cast<Note*>(e->parent())));
            }
      _layoutAll = true;      // must be set in und/redo
      }

//---------------------------------------------------------
//   cmdAddBSymbol
//    add Symbol or Image
//---------------------------------------------------------

void Score::cmdAddBSymbol(BSymbol* s, const QPointF& pos, const QPointF& off)
      {
      s->setSelected(false);
      bool foundPage = false;
      foreach (Page* page, pages()) {
            if (page->contains(pos)) {
                  const QList<System*>* sl = page->systems();
                  if (sl->isEmpty()) {
                        qDebug("addSymbol: cannot put symbol here: no system on page");
                        delete s;
                        return;
                        }
                  System* system = sl->front();
                  MeasureBase* m = system->measures().front();
                  if (m == 0) {
                        qDebug("addSymbol: cannot put symbol here: no measure in system");
                        delete s;
                        return;
                        }
                  s->setPos(0.0, 0.0);
                  s->setUserOff(pos - m->pagePos() - off);
                  s->setTrack(0);
                  s->setParent(m);
                  foundPage = true;
                  break;
                  }
            }
      if (!foundPage) {
            qDebug("addSymbol: cannot put symbol here: no page");
            delete s;
            return;
            }
      undoAddElement(s);
      addRefresh(s->abbox());
      select(s, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   deleteItem
//---------------------------------------------------------

void Score::deleteItem(Element* el)
      {
      switch(el->type()) {
            case INSTRUMENT_NAME: {
                  Part* part = el->staff()->part();
                  InstrumentName* in = static_cast<InstrumentName*>(el);
                  if (in->subtype() == INSTRUMENT_NAME_LONG)
                        undo(new ChangeInstrumentLong(0, part, QList<StaffNameDoc>()));
                  else if (in->subtype() == INSTRUMENT_NAME_SHORT)
                        undo(new ChangeInstrumentShort(0, part, QList<StaffNameDoc>()));
                  }
                  break;

            case TIMESIG:
                  cmdRemoveTimeSig(static_cast<TimeSig*>(el));
                  break;

            case OTTAVA_SEGMENT:
            case HAIRPIN_SEGMENT:
            case TRILL_SEGMENT:
            case TEXTLINE_SEGMENT:
            case VOLTA_SEGMENT:
            case SLUR_SEGMENT:
                  undoRemoveElement(static_cast<SpannerSegment*>(el)->spanner());
                  break;

            case NOTE:
                  {
                  Chord* chord = static_cast<Chord*>(el->parent());
                  if (chord->notes().size() > 1) {
                        undoRemoveElement(el);
                        break;
                        }
                  // else fall through
                  el = chord;
                  }

            case CHORD:
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

            case REST:
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

            case MEASURE:
                  {
                  Measure* measure = static_cast<Measure*>(el);
                  undo(new RemoveElement(measure));
                  cmdRemoveTime(measure->tick(), measure->ticks());
                  }
                  break;

            case ACCIDENTAL:
                  changeAccidental(static_cast<Note*>(el->parent()), ACC_NONE);
                  break;

            case BAR_LINE:
                  {
                  BarLine* bl  = static_cast<BarLine*>(el);
                  Segment* seg = bl->segment();
                  Measure* m   = seg->measure();
                  if (seg->subtype() == SegStartRepeatBarLine)
                        undoChangeRepeatFlags(m, m->repeatFlags() & ~RepeatStart);
                  else if (seg->subtype() == SegBarLine)
                        undoRemoveElement(el);
                  else if (seg->subtype() == SegEndBarLine) {
                        if (m->endBarLineType() != NORMAL_BAR) {
                              undoChangeRepeatFlags(m, m->repeatFlags() & ~RepeatEnd);
                              Measure* nm = m->nextMeasure();
                              if (nm)
                                    undoChangeRepeatFlags(nm, nm->repeatFlags() & ~RepeatStart);
                              undoChangeEndBarLineType(m, NORMAL_BAR);
                              m->setEndBarLineGenerated(true);
                              }
                        }
                  }
                  break;
            case TUPLET:
                  cmdDeleteTuplet(static_cast<Tuplet*>(el), true);
                  break;

            default:
                  undoRemoveElement(el);
                  break;
            }
      }

//---------------------------------------------------------
//   cmdRemoveTime
//---------------------------------------------------------

void Score::cmdRemoveTime(int tick, int len)
      {
      undoInsertTime(tick, -len);
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
      MeasureBase* ie   = seg ? seg->measure() : lastMeasure();
      int endIdx        = measureIdx(ie);
      if (ie != is)
            ie = ie->prev();

      // createEndBar if last measure is deleted
      bool createEndBar = true;
      for (MeasureBase* mb = ie->next(); mb; mb = mb->next()) {
            if (mb->type() == MEASURE) {
                  createEndBar = false;
                  break;
                  }
            }

      QList<Score*> scores = scoreList();
      foreach(Score* score, scores) {
            MeasureBase* is = score->measure(startIdx);
            MeasureBase* ie = score->measure(endIdx);
            for (;;) {
                  deleteItem(ie);
                  if (ie == is)
                        break;
                  ie = ie->prev();
                  if (ie == 0)
                        break;
                  }
            }

      if (createEndBar) {
            MeasureBase* mb = _measures.last();
            while (mb && mb->type() != MEASURE)
                  mb = mb->prev();
            if (mb) {
                  Measure* lastMeasure = static_cast<Measure*>(mb);
                  if (lastMeasure->endBarLineType() == NORMAL_BAR) {
                        undoChangeEndBarLineType(lastMeasure, END_BAR);
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

            Segment* ss1 = s1;
            if (ss1->subtype() != SegChordRest)
                  ss1 = ss1->next1(SegChordRest);
            bool fullMeasure = ss1 && (ss1->measure()->first(SegChordRest) == ss1)
                  && (s2 == 0 || (s2->subtype() == SegEndBarLine));

            int tick2   = s2 ? s2->tick() : INT_MAX;
            int track1  = selection().staffStart() * VOICES;
            int track2  = selection().staffEnd() * VOICES;
            for (int track = track1; track < track2; ++track) {
                  Fraction f;
                  int tick  = -1;
                  Tuplet* tuplet = 0;
                  for (Segment* s = s1; s != s2; s = s->next1()) {
                        if (s->element(track) &&
                           ((s->subtype() == SegBreath)
                           || (s->subtype() == SegGrace))) {
                              deleteItem(s->element(track));
                              continue;
                              }
                        if (s->subtype() != SegChordRest || !s->element(track))
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
      if (el == 0 || (el->type() != NOTE && el->type() != LYRICS)) {
            QMessageBox::information(0,
               QMessageBox::tr("MuseScore:"),
               QMessageBox::tr("No note or lyrics selected:\n"
                  "Please select a single note or lyrics and retry operation\n"),
               QMessageBox::Ok, QMessageBox::NoButton);
            return 0;
            }
      ChordRest* cr;
      if (el->type() == NOTE)
            cr = static_cast<Note*>(el)->chord();
      else if (el->type() == LYRICS)
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
qDebug("createTuplet at %d <%s> duration <%s> ratio <%s> baseLen <%s>",
  ocr->tick(), ocr->name(),
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
      if (ocr->type() == CHORD) {
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

      cr->setTuplet(tuplet);
      cr->setTrack(track);
      cr->setDurationType(tuplet->baseLen());
      cr->setDuration(tuplet->baseLen().fraction());

qDebug("tuplet note duration %s  actualNotes %d  ticks %d",
      qPrintable(tuplet->baseLen().name()), actualNotes, cr->actualTicks());

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
                  if (e->type() == BAR_LINE) {
                        Element* ep = e->parent();
                        if (ep->type() == SEGMENT && static_cast<Segment*>(ep)->subtype() == SegEndBarLine) {
                              Measure* m = static_cast<Segment*>(ep)->measure();
                              BarLine* bl = static_cast<BarLine*>(e);
                              m->setEndBarLineType(bl->subtype(), false, e->visible(), e->color());
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
      Segment* seg  = setNoteRest(_is.segment(), track, nval, d.fraction(), AUTO);
      if (seg) {
            ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
            if (cr)
                  nextInputPos(cr, false);
            }
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
                  Q_ASSERT(de->type() == TUPLET);
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

MeasureBase* Score::insertMeasure(ElementType type, MeasureBase* measure,
   bool createEmptyMeasures)
      {
      int tick;
      int idx;
      if (measure) {
            tick = measure->tick();
            idx  = measureIdx(measure);
            }
      else {
            measure = last();
            tick = measure->tick() + measure->ticks();
            idx  = -1;
            }

      MeasureBase* omb = 0;
      foreach(Score* score, scoreList()) {
            MeasureBase* mb = static_cast<MeasureBase*>(Element::create(type, score));
            MeasureBase* im = idx != -1 ? score->measure(idx) : 0;
            // insert before im, append if im = 0

            mb->setTick(tick);
            if (score == this)
                  omb = mb;

            if (type == MEASURE) {
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

                  Fraction f = score->sigmap()->timesig(tick).nominal();
      	      int ticks  = f.ticks();

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
                                    if (e->type() == KEYSIG) {
                                          KeySig* ks = static_cast<KeySig*>(e);
                                          ksl.append(ks);
                                          undo(new RemoveElement(ks));
                                          if (ks->segment()->isEmpty())
                                                undoRemoveElement(ks->segment());
                                          }
                                    else if (e->type() == TIMESIG) {
                                          TimeSig* ts = static_cast<TimeSig*>(e);
                                          tsl.append(ts);
                                          undo(new RemoveElement(ts));
                                          if (ts->segment()->isEmpty())
                                                undoRemoveElement(ts->segment());
                                          }
                                    else if (e->type() == CLEF) {
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
                  undoInsertTime(tick, ticks);

                  //
                  // if measure is inserted at tick zero,
                  // create key and time signature
                  //
                  foreach(TimeSig* ts, tsl) {
                        TimeSig* nts = new TimeSig(*ts);
                        Segment* s   = m->undoGetSegment(SegTimeSig, 0);
                        nts->setParent(s);
                        undoAddElement(nts);
                        }
                  foreach(KeySig* ks, ksl) {
                        KeySig* nks = new KeySig(*ks);
                        Segment* s  = m->undoGetSegment(SegKeySig, 0);
                        nks->setParent(s);
                        undoAddElement(nks);
                        }
                  foreach(Clef* clef, cl) {
                        Clef* nClef = new Clef(*clef);
                        Segment* s  = m->undoGetSegment(SegClef, 0);
                        nClef->setParent(s);
                        undoAddElement(nClef);
                        }
                  if (createEndBar) {
                        Measure* lm = score->lastMeasure();
                        if (lm)
                              lm->setEndBarLineType(END_BAR, endBarGenerated);
                        }
                  }
            else {
                  undo(new InsertMeasure(mb, im));
                  }
            }
      if (type == MEASURE && !createEmptyMeasures) {
            //
            // fill measure with rest
            //
            // TODO: this does not work if the part has unlinked staves!
            //
            Score* root = rootScore();
            int n = root->nstaves();
            for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
                  Rest* rest = new Rest(root, TDuration(TDuration::V_MEASURE));
                  Measure* m = static_cast<Measure*>(omb);
                  rest->setDuration(m->len());
                  rest->setTrack(staffIdx * VOICES);
                  undoAddCR(rest, m, m->tick());
            	}
            }
      return omb;
      }

