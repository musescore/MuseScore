//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: cmd.cpp 5644 2012-05-17 10:53:29Z lasconic $
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
 Handling of several GUI commands.
*/

#include <assert.h>

#include "score.h"
#include "utils.h"
#include "key.h"
#include "clef.h"
#include "navigate.h"
#include "slur.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "text.h"
#include "sig.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "page.h"
#include "barline.h"
#include "tuplet.h"
#include "xml.h"
#include "ottava.h"
#include "trill.h"
#include "pedal.h"
#include "hairpin.h"
#include "textline.h"
#include "keysig.h"
#include "volta.h"
#include "dynamic.h"
#include "box.h"
#include "harmony.h"
#include "system.h"
#include "stafftext.h"
#include "articulation.h"
#include "layoutbreak.h"
#include "drumset.h"
#include "beam.h"
#include "lyrics.h"
#include "pitchspelling.h"
#include "measure.h"
#include "tempo.h"
#include "sig.h"
#include "undo.h"
#include "timesig.h"
#include "repeat.h"
#include "tempotext.h"
#include "clef.h"
#include "noteevent.h"
#include "breath.h"
#include "tablature.h"
#include "stafftype.h"
#include "segment.h"
#include "chordlist.h"
#include "mscore.h"
#include "accidental.h"
#include "sequencer.h"

//---------------------------------------------------------
//   startCmd
///   Start a GUI command by clearing the redraw area
///   and starting a user-visble undo.
//---------------------------------------------------------

void Score::startCmd()
      {
      if (MScore::debugMode)
            qDebug("===startCmd()");
      _layoutAll = true;      ///< do a complete relayout
      _playNote = false;

      // Start collecting low-level undo operations for a
      // user-visible undo action.

      if (undo()->active()) {
            // if (MScore::debugMode)
            qDebug("Score::startCmd(): cmd already active");
            return;
            }
      undo()->beginMacro();
      undo(new SaveState(this));
      }

//---------------------------------------------------------
//   endCmd
///   End a GUI command by (if \a undo) ending a user-visble undo
///   and (always) updating the redraw area.
//---------------------------------------------------------

void Score::endCmd()
      {
      if (!undo()->active()) {
            // if (MScore::debugMode)
                  qDebug("Score::endCmd(): no cmd active");
            end();
            return;
            }

      foreach(Score* s, scoreList())
            s->end2();

      bool noUndo = undo()->current()->childCount() <= 1;
      if (!noUndo)
            setDirty(true);
      undo()->endMacro(noUndo);
      end();      // DEBUG
      }

//---------------------------------------------------------
//   end
///   Update the redraw area.
//---------------------------------------------------------

void Score::end()
      {
      foreach(Score* s, scoreList())
            s->end1();
      }

//---------------------------------------------------------
//   update
//    layout & update
//---------------------------------------------------------

void Score::update()
      {
      foreach(Score* s, scoreList()) {
            s->end2();
            s->end1();
            }
      }

//---------------------------------------------------------
//   end2
//---------------------------------------------------------

void Score::end2()
      {
      bool _needLayout = false;
      if (_layoutAll) {
            _updateAll  = true;
            _needLayout = true;
            startLayout = 0;
            }
      else if (startLayout) {
            _updateAll = true;
            _needLayout = true;
            }
      if (_needLayout)
            doLayout();
      _layoutAll   = false;
      startLayout = 0;
      }

//---------------------------------------------------------
//   end1
//---------------------------------------------------------

void Score::end1()
      {
      if (_updateAll) {
            foreach(MuseScoreView* v, viewer)
                  v->updateAll();
            }
      else {
            // update a little more:
            qreal d = spatium() * .5;
            refresh.adjust(-d, -d, 2 * d, 2 * d);
            foreach(MuseScoreView* v, viewer)
                  v->dataChanged(refresh);
            }
      refresh     = QRectF();
      _updateAll  = false;
      }

//---------------------------------------------------------
//   endUndoRedo
///   Common handling for ending undo or redo
//---------------------------------------------------------

void Score::endUndoRedo()
      {
      updateSelection();
      foreach(Score* score, scoreList()) {
            if (score->layoutAll()) {
                  score->setUndoRedo(true);
                  score->doLayout();
                  score->setUndoRedo(false);
                  score->setUpdateAll(true);
                  }
            score->setPlaylistDirty(true);
            }
      end();
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void Score::moveCursor()
      {
      foreach(MuseScoreView* v, viewer)
            v->moveCursor();
      }

//---------------------------------------------------------
//   cmdAddSpanner
//   drop VOLTA, OTTAVA, TRILL, PEDAL, DYNAMIC
//        HAIRPIN, and TEXTLINE
//---------------------------------------------------------

void Score::cmdAddSpanner(Spanner* spanner, const QPointF& pos, const QPointF& /*dragOffset*/)
      {
      int staffIdx;
      Segment* segment;
      MeasureBase* mb = pos2measure(pos, &staffIdx, 0, &segment, 0);
      if (mb == 0 || mb->type() != Element::MEASURE) {
            qDebug("cmdAddSpanner: cannot put object here");
            delete spanner;
            return;
            }

      int track = staffIdx == -1 ? -1 : staffIdx * VOICES;
      spanner->setTrack(track);

      if (spanner->anchor() == Spanner::ANCHOR_SEGMENT) {
            spanner->setStartElement(segment);
            spanner->setParent(segment);

            static const Segment::SegmentType st = Segment::SegChordRest;
            Segment* ns = 0;
            for (Segment* s = segment; s; s = s->next1(st)) {
                  ns = s;
                  if (s->measure() != segment->measure())
                        break;
                  }
            if (ns == segment) {
                  qDebug("cmdAddSpanner: cannot put object on last segment");
                  delete spanner;
                  return;
                  }
            spanner->setEndElement(ns);
            }
      else {      // ANCHOR_MEASURE
            Measure* m = static_cast<Measure*>(mb);
            QRectF b(m->canvasBoundingRect());

            if (pos.x() >= (b.x() + b.width() * .5))
                  m = m->nextMeasure();
            spanner->setStartElement(m);
            spanner->setEndElement(m);
            spanner->setParent(m);
            }

      undoAddElement(spanner);
      select(spanner, SELECT_SINGLE, 0);

      if (spanner->type() == Element::TRILL) {
            Element* e = segment->element(staffIdx * VOICES);
            if (e && e->type() == Element::CHORD) {
                  Chord* chord = static_cast<Chord*>(e);
                  Fraction l = chord->duration();
                  if (chord->notes().size() > 1) {
                        // trill do not work for chords
                        }
                  Note* note = chord->upNote();
                  while (note->tieFor()) {
                        note = note->tieFor()->endNote();
                        l += note->chord()->duration();
                        }
                  Segment* s = note->chord()->segment();
                  s = s->next1(Segment::SegChordRest);
                  while (s) {
                        Element* e = s->element(staffIdx * VOICES);
                        if (e)
                              break;
                        s = s->next1(Segment::SegChordRest);
                        }
                  if (s)
                        spanner->setEndElement(s);
                  Fraction d(1,32);
                  Fraction e = l / d;
                  int n = e.numerator() / e.denominator();
                  QList<NoteEvent*> events;
                  int pitch  = chord->upNote()->ppitch();
                  int key    = chord->staff()->key(segment->tick()).accidentalType();
                  int pitch2 = diatonicUpDown(key, pitch, 1);
                  int dpitch = pitch2 - pitch;
                  for (int i = 0; i < n; i += 2) {
                        events.append(new NoteEvent(0,      i * 1000 / n,    1000/n));
                        events.append(new NoteEvent(dpitch, (i+1) *1000 / n, 1000/n));
                        }
                  undo(new ChangeNoteEvents(chord, events));
                  }
            }
      }

//---------------------------------------------------------
//   expandVoice
//---------------------------------------------------------

void Score::expandVoice(Segment* s, int track)
      {
      if (s->element(track)) {
            ChordRest* cr = (ChordRest*)(s->element(track));
            qDebug("expand voice: found %s %s", cr->name(), qPrintable(cr->duration().print()));
            return;
            }

      Segment* ps;
      for (ps = s; ps; ps = ps->prev(Segment::SegChordRest)) {
            if (ps->element(track))
                  break;
            }
      if (ps) {
            ChordRest* cr = static_cast<ChordRest*>(ps->element(track));
            int tick = cr->tick() + cr->actualTicks();
            if (tick == s->tick())
                  return;
            if (tick > s->tick()) {
                  qDebug("expandVoice: cannot insert element here");
                  return;
                  }
            }
      //
      // fill upto s->tick() with rests
      //
      Measure* m = s->measure();
      int stick  = ps ?  ps->tick() : m->tick();
      int ticks  = s->tick() - stick;
      if (ticks)
            setRest(stick, track, Fraction::fromTicks(ticks), false, 0);

      //
      // fill from s->tick() until next chord/rest
      //
      Segment* ns;
      for (ns = s->next(Segment::SegChordRest); ns; ns = ns->next(Segment::SegChordRest)) {
            if (ns->element(track))
                  break;
            }
      ticks  = ns ? (ns->tick() - s->tick()) : (m->ticks() - s->rtick());
      if (ticks == m->ticks())
            addRest(s, track, TDuration(TDuration::V_MEASURE), 0);
      else
            setRest(s->tick(), track, Fraction::fromTicks(ticks), false, 0);
      }

void Score::expandVoice()
      {
      Segment* s = _is.segment();
      int track  = _is.track();
      expandVoice(s, track);
      }

//---------------------------------------------------------
//   addPitch
//---------------------------------------------------------

Note* Score::addPitch(int pitch, bool addFlag)
      {
qDebug("add pitch %d %d", pitch, addFlag);

      if (addFlag) {
            moveToPrevInputPos();
            if (_is.cr() == 0 || _is.cr()->type() != Element::CHORD)
                  return 0;
            Chord* chord = static_cast<Chord*>(_is.cr());
            Note* n = addNote(chord, pitch);
            setLayoutAll(false);
            setLayout(chord->measure());
            moveToNextInputPos();
            return n;
            }
      expandVoice();

      // insert note
      MScore::Direction stemDirection = MScore::AUTO;
      Note::NoteHeadGroup headGroup = Note::HEAD_NORMAL;
      int track               = _is.track();
      if (_is.drumNote() != -1) {
            pitch         = _is.drumNote();
            Drumset* ds   = _is.drumset();
            headGroup     = ds->noteHead(pitch);
            stemDirection = ds->stemDirection(pitch);
            track         = ds->voice(pitch) + (_is.track() / VOICES) * VOICES;
            _is.setTrack(track);
            expandVoice();
            }
      if (!_is.cr())
            return 0;
      NoteVal nval;
      nval.pitch     = pitch;
      nval.headGroup = headGroup;
      Fraction duration;
      if (_is.repitchMode())
            duration = _is.cr()->duration();
      else
            duration = _is.duration().fraction();
      Segment* seg   = setNoteRest(_is.segment(), track, nval, duration, stemDirection);
      Note* note     = 0;
      if (seg) {
            note = static_cast<Chord*>(seg->element(track))->upNote();
            setLayout(note->chord()->measure());
            }

      if (_is.slur) {
            //
            // extend slur
            //
            ChordRest* e = searchNote(_is.tick(), _is.track());
            if (e) {
                  int stick = 0;
                  Element* ee = _is.slur->startElement();
                  if (ee->isChordRest())
                        stick = static_cast<ChordRest*>(ee)->tick();
                  else if (ee->type() == Element::NOTE)
                        stick = static_cast<Note*>(ee)->chord()->tick();
                  if (stick == e->tick()) {
                        if (_is.slur->startElement())
                              static_cast<ChordRest*>(_is.slur->startElement())->removeSlurFor(_is.slur);
                        _is.slur->setStartElement(e);
                        static_cast<ChordRest*>(e)->addSlurFor(_is.slur);
                        }
                  else
                        _is.slur->setEndElement(e);
                  }
            else
                  qDebug("addPitch: cannot find slur note");
            setLayoutAll(true);
            }
      moveToNextInputPos();
      return note;
      }

//---------------------------------------------------------
//   cmdAddInterval
//---------------------------------------------------------

void Score::cmdAddInterval(int val, const QList<Note*>& nl)
      {
      startCmd();
      foreach(Note* on, nl) {
            Note* note = new Note(*on);
            Chord* chord = on->chord();
            note->setParent(chord);
            int valTmp = val < 0 ? val+1 : val-1;

            int npitch;
            int ntpc;
            if( abs(valTmp) != 7 ) {
                  int line = on->line() - valTmp;
                  int tick   = chord->tick();
                  Staff* estaff = staff(on->staffIdx() + chord->staffMove());
                  int clef   = estaff->clef(tick);
                  int key    = estaff->key(tick).accidentalType();
                  npitch = line2pitch(line, clef, key);
                  ntpc   = pitch2tpc(npitch, key);
                  }
            else { //special case for octave
                  Interval interval(7, 12);
                  if(val < 0) {
                        interval.flip();
                        }
                  transposeInterval(on->pitch(), on->tpc(), &npitch, &ntpc, interval, false);
                  }
            note->setPitch(npitch, ntpc);

            undoAddElement(note);
            _playNote = true;
            setLayout(on->chord()->measure());

            select(note, SELECT_SINGLE, 0);
            _is.pitch = note->pitch();
            }
      moveToNextInputPos();
      endCmd();
      }

//---------------------------------------------------------
//   setGraceNote
///   Create a grace note in front of a normal note.
///   \arg chord is the normal note
///   \arg pitch is the pitch of the grace note
///   \arg is the grace note type
///   \len is the visual duration of the grace note (1/16 or 1/32)
//---------------------------------------------------------

void Score::setGraceNote(Chord* ch, int pitch, NoteType type, bool behind, int len)
      {
      Note* note = new Note(this);
      note->setPitch(pitch);

      Chord* chord = new Chord(this);
      chord->setTrack(ch->track());
      chord->add(note);

      TDuration d;
      d.setVal(len);
      chord->setDurationType(d);
      chord->setDuration(d.fraction());
      chord->setNoteType(type);
      chord->setMag(ch->staff()->mag() * styleD(ST_graceNoteMag));

      undoAddGrace(chord, ch->segment(), behind);

      note->setTpcFromPitch();      // tick must be known
      select(note, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   setNoteRest
//    pitch == -1  -> set rest
//    return segment of last created note/rest
//---------------------------------------------------------

Segment* Score::setNoteRest(Segment* segment, int track, NoteVal nval, Fraction sd,
   MScore::Direction stemDirection)
      {
      if (segment->subtype() == Segment::SegGrace) {
            Chord* chord = static_cast<Chord*>(segment->element(track));
            if (chord->notes().size() == 1) {
                  Note* note = chord->upNote();
                  int tpc = pitch2tpc2(nval.pitch, true);
                  int line = note->line();
                  undoChangePitch(note, nval.pitch, tpc, line);
                  }
            return segment;
            }
      assert(segment->subtype() == Segment::SegChordRest);

      int tick      = segment->tick();
      Element* nr   = 0;
      Tie* tie      = 0;
      ChordRest* cr = static_cast<ChordRest*>(segment->element(track));

      Measure* measure = 0;
      for (;;) {
            if (track % VOICES)
                  expandVoice(segment, track);

            // the returned gap ends at the measure boundary or at tuplet end
            Fraction dd = makeGap(segment, track, sd, cr ? cr->tuplet() : 0);

            if (dd.isZero()) {
                  qDebug("cannot get gap at %d type: %d/%d", tick, sd.numerator(),
                     sd.denominator());
                  break;
                  }
            QList<TDuration> dl = toDurationList(dd, true);

            measure = segment->measure();
            int n = dl.size();
            for (int i = 0; i < n; ++i) {
                  TDuration d = dl[i];

                  ChordRest* ncr;
                  Note* note = 0;
                  if (nval.pitch == -1) {
                        nr = ncr = new Rest(this);
                        nr->setTrack(track);
                        ncr->setDurationType(d);
                        ncr->setDuration(d.fraction());
                        }
                  else {
                        nr = note = new Note(this);

                        if (tie) {
                              tie->setEndNote(note);
                              note->setTieBack(tie);
                              }
                        Chord* chord = new Chord(this);
                        chord->setTrack(track);
                        chord->setDurationType(d);
                        chord->setDuration(d.fraction());
                        chord->setStemDirection(stemDirection);
                        chord->add(note);
                        note->setNval(nval);
                        ncr = chord;
                        if (i+1 < n) {
                              tie = new Tie(this);
                              tie->setStartNote(note);
                              tie->setTrack(track);
                              note->setTieFor(tie);
                              }
                        }
                  ncr->setTuplet(cr ? cr->tuplet() : 0);
                  undoAddCR(ncr, measure, tick);
                  _playNote = true;
                  segment = ncr->segment();
                  tick += ncr->actualTicks();
                  }

            sd -= dd;
            if (sd.isZero())
                  break;

            Segment* nseg = tick2segment(tick, false, Segment::SegChordRest);
            if (nseg == 0) {
                  qDebug("reached end of score");
                  break;
                  }
            segment = nseg;

            cr = static_cast<ChordRest*>(segment->element(track));

            if (cr == 0) {
                  if (track % VOICES)
                        cr = addRest(segment, track, TDuration(TDuration::V_MEASURE), 0);
                  else {
                        qDebug("no rest in voice 0");
                        break;
                        }
                  }
            //
            //  Note does not fit on current measure, create Tie to
            //  next part of note
            if (nval.pitch != -1) {
                  tie = new Tie(this);
                  tie->setStartNote((Note*)nr);
                  tie->setTrack(nr->track());
                  ((Note*)nr)->setTieFor(tie);
                  }
            }
      if (tie)
            connectTies();
      if (nr)
            select(nr, SELECT_SINGLE, 0);
      return segment;
      }

//---------------------------------------------------------
//   makeGap
//    make time gap at tick by removing/shortening
//    chord/rest
//
//    gap does not exceed measure or scope of tuplet
//
//    return size of actual gap
//---------------------------------------------------------

Fraction Score::makeGap(Segment* segment, int track, const Fraction& _sd, Tuplet* tuplet)
      {
qDebug("makeGap %s at %d track %d", qPrintable(_sd.print()), segment->tick(), track);
      assert(_sd.numerator());

      Measure* measure = segment->measure();
      setLayout(measure);
      Fraction akkumulated;
      Fraction sd = _sd;

      //
      // remember first segment which should
      // not be deleted (it may contain other elements we want to preserve)
      //
      Segment* firstSegment = segment;
      int nextTick = segment->tick();

      for (Segment* seg = firstSegment; seg; seg = seg->next(Segment::SegChordRestGrace)) {
            if (seg->subtype() == Segment::SegGrace) {
                  if (seg->element(track)) {
                        undoRemoveElement(seg->element(track));
                        if (seg->isEmpty() && seg != firstSegment)
                              undoRemoveElement(seg);
                        }
                  continue;
                  }
            //
            // voices != 0 may have gaps:
            //
            ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
            if (!cr) {
                  if (seg->tick() < nextTick)
                        continue;
                  Segment* seg1 = seg->next(Segment::SegChordRest);
                  int tick2 = seg1 ? seg1->tick() : seg->measure()->tick() + seg->measure()->ticks();
                  Fraction td(Fraction::fromTicks(tick2 - seg->tick()));
                  segment = seg;
                  if (td > sd)
                        td = sd;
                  akkumulated += td;
                  sd -= td;
                  if (sd.isZero())
                        return akkumulated;
                  nextTick = seg1->tick();
                  continue;
                  }
            //
            // limit to tuplet level
            //
            if (tuplet) {
                  bool tupletEnd = true;
                  Tuplet* t = cr->tuplet();
                  while (t) {
                        if (cr->tuplet() == tuplet) {
                              tupletEnd = false;
                              break;
                              }
                        t = t->tuplet();
                        }
                  if (tupletEnd) {
//                        qDebug("makeGap: end of tuplet reached");
                        return akkumulated;
                        }
                  }
            Fraction td(cr->duration());
qDebug("remove %s %s at tick %d track %d",
   cr->name(), qPrintable(cr->duration().print()), seg->tick(), track);

            Tuplet* ltuplet = cr->tuplet();
            if (cr->tuplet() != tuplet) {
//                  qDebug("   remove tuplet %d", sd >= ltuplet->fraction());
                  //
                  // Current location points to the start of a (nested)tuplet.
                  // We have to remove the complete tuplet.

                  Tuplet* t = ltuplet;
                  while (t->elements().last()->type() == Element::TUPLET)
                        t = static_cast<Tuplet*>(t->elements().last());
                  seg = static_cast<ChordRest*>(t->elements().last())->segment();

                  td = ltuplet->duration();
                  cmdDeleteTuplet(ltuplet, false);
                  tuplet = 0;
                  }
            else {
qDebug("  makeGap: remove %d/%d at %d", td.numerator(), td.denominator(), cr->tick());
                  undoRemoveElement(cr);
                  if (seg->isEmpty() && seg != firstSegment)
                        undoRemoveElement(seg);
                  else if (seg != firstSegment) {     // keep _all_ annotations on first segment?
                        foreach(Element* e, seg->annotations()) {
                              if (e->track() == cr->track())
                                    undoRemoveElement(e);
                              }
                        }
                  }
            nextTick += td.ticks();
            if (sd < td) {
                  //
                  // we removed too much
                  //
                  akkumulated = _sd;
                  Fraction rd = td - sd;

qDebug("  makeGap: %d/%d removed %d/%d too much", sd.numerator(), sd.denominator(), rd.numerator(), rd.denominator());

                  QList<TDuration> dList = toDurationList(rd, false);
                  if (dList.isEmpty())
                        return akkumulated;
qDebug("   dList: %d\n", dList.size());

                  Fraction f(cr->staff()->timeStretch(cr->tick()) * sd);
                  for (Tuplet* t = tuplet; t; t = t->tuplet())
                        f /= t->ratio();
                  int tick  = cr->tick() + f.ticks();
qDebug("   gap at tick %d+%d", cr->tick(), f.ticks());

                  if ((tuplet == 0) && (((measure->tick() - tick) % dList[0].ticks()) == 0)) {
                        foreach(TDuration d, dList) {
                              qDebug("    addClone at %d, %d", tick, d.ticks());
                              tick += addClone(cr, tick, d)->actualTicks();
                              }
                        }
                  else {
                        for (int i = dList.size() - 1; i >= 0; --i)
                              tick += addClone(cr, tick, dList[i])->actualTicks();
                        }
// qDebug("  return %d/%d", akkumulated.numerator(), akkumulated.denominator());
                  return akkumulated;
                  }
            akkumulated += td;
            sd          -= td;
qDebug("  akkumulated %d/%d rest %d/%d (-%d/%d)",
   akkumulated.numerator(), akkumulated.denominator(), sd.numerator(), sd.denominator(),
   td.numerator(), td.denominator());
            if (sd.isZero())
                  return akkumulated;
            }
//      int ticks = measure->tick() + measure->ticks() - segment->tick();
//      Fraction td = Fraction::fromTicks(ticks);
// NEEDS REVIEW !!
// once the statement below is removed, these two lines do nothing
//      if (td > sd)
//            td = sd;
// ???  akkumulated should already contain the total value of the created gap: line 749, 811 or 838
//      this line creates a qreal-sized gap if the needed gap crosses a measure boundary
//      by adding again the duration already added in line 838
//      akkumulated += td;
      return akkumulated;
      }

//---------------------------------------------------------
//   makeGap1
//    make time gap at tick by removing/shortening
//    chord/rest
//    - cr is top level (not part of a tuplet)
//    - do not stop at measure end
//---------------------------------------------------------

bool Score::makeGap1(int tick, int staffIdx, Fraction len)
      {
      ChordRest* cr = 0;
      Segment* seg = tick2segment(tick, true, Segment::SegChordRestGrace);
      if (!seg) {
            qDebug("1:makeGap1: no segment at %d", tick);
            return false;
            }
      int track = staffIdx * VOICES;
      cr = static_cast<ChordRest*>(seg->element(track));
      if (!cr) {
            if (seg->subtype() & Segment::SegGrace) {
                  seg = seg->next1(Segment::SegChordRest);
                  if (!seg || !seg->element(track)) {
                        qDebug("makeGap1: no chord/rest at %d staff %d", tick, staffIdx);
                        return false;
                        }
                  cr = static_cast<ChordRest*>(seg->element(track));
                  }
            else {
                  // check if we are in the middle of a chord/rest
                  Segment* seg1 = 0;
                  for (;;) {
                        seg1 = seg->prev(Segment::SegChordRest);
                        if (seg1 == 0) {
                              qDebug("1:makeGap1: no segment at %d", tick);
                              return false;
                              }
                        if (seg1->element(track))
                              break;
                        }
                  ChordRest* cr1 = static_cast<ChordRest*>(seg1->element(track));
                  Fraction dstF = Fraction::fromTicks(tick - cr1->tick());
                  len -= cr1->duration() - dstF;
                  undoChangeChordRestLen(cr1, TDuration(dstF));
                  for (;;) {
                        seg = seg->next1(Segment::SegChordRestGrace);
                        if (seg == 0) {
                              qDebug("2:makeGap1: no segment");
                              return false;
                              }
                        if (seg->element(track)) {
                              tick = seg->tick();
                              cr = static_cast<ChordRest*>(seg->element(track));
                              break;
                              }
                        }
                  }
            }

      for (;;) {
            if (!cr) {
                  qDebug("makeGap1: cannot make gap");
                  return false;
                  }
            Fraction l = makeGap(cr->segment(), cr->track(), len, 0);
            if (l.isZero()) {
                  qDebug("makeGap1: makeGap returns zero gap");
                  return false;
                  }
            len -= l;
            if (len.isZero())
                  break;
            // go to next cr
            Measure* m = cr->measure()->nextMeasure();
            if (m == 0) {
                  qDebug("EOS reached");
                  insertMeasure(Element::MEASURE, 0, false);
                  m = cr->measure()->nextMeasure();
                  if (m == 0) {
                        qDebug("===EOS reached");
                        return true;
                        }
                  }
            Segment* s = m->firstCRSegment();
            int track  = cr->track();
            cr = static_cast<ChordRest*>(s->element(track));
            if (cr == 0) {
                  addRest(s, track, TDuration(TDuration::V_MEASURE), 0);
                  cr = static_cast<ChordRest*>(s->element(track));
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   splitGapToMeasureBoundaries
//    cr  - start of gap
//    gap - gap len
//---------------------------------------------------------

QList<Fraction> Score::splitGapToMeasureBoundaries(ChordRest* cr, Fraction gap)
      {
      QList<Fraction> flist;

      Tuplet* tuplet = cr->tuplet();
      if (tuplet) {
            Fraction rest(tuplet->duration());

            if (rest < gap)
                  qDebug("does not fit in tuplet");
            else
                  flist.append(gap);
            return flist;
            }

      Segment* s = cr->segment();
      while (gap > Fraction(0)) {
            Measure* m    = s->measure();
            Fraction rest = Fraction::fromTicks(m->ticks() - s->rtick());
            if (rest >= gap) {
                  flist.append(gap);
                  return flist;
                  }
            flist.append(rest);
            gap -= rest;
            m = m->nextMeasure();
            if (m == 0)
                  return flist;
            s = m->first(Segment::SegChordRest);
            }
      return flist;
      }

//---------------------------------------------------------
//   changeCRlen
//---------------------------------------------------------

void Score::changeCRlen(ChordRest* cr, const TDuration& d)
      {
      deselectAll();
      Fraction srcF(cr->duration());
      Fraction dstF;
      if (d.type() == TDuration::V_MEASURE)
            dstF = cr->measure()->stretchedLen(cr->staff());
      else
            dstF = d.fraction();

qDebug("changeCRlen: %d/%d -> %d/%d", srcF.numerator(), srcF.denominator(),
      dstF.numerator(), dstF.denominator());

      if (srcF == dstF)
            return;
      int track = cr->track();
      Tuplet* tuplet = cr->tuplet();
      if (srcF > dstF) {
            //
            // make shorter and fill with rest
            //
            if (cr->type() == Element::CHORD) {
                  //
                  // remove ties
                  //
                  Chord* c = static_cast<Chord*>(cr);
                  foreach(Note* n, c->notes()) {
                        if (n->tieFor())
                              undoRemoveElement(n->tieFor());
                        }
                  }
            undoChangeChordRestLen(cr, TDuration(dstF));
            setRest(cr->tick() + cr->actualTicks(), track, srcF - dstF, false, tuplet);
            select(cr, SELECT_SINGLE, 0);
            return;
            }

      //
      // make longer
      //
      // split required len into Measures
      QList<Fraction> flist = splitGapToMeasureBoundaries(cr, dstF);
      if (flist.isEmpty())
            return;

qDebug("ChangeCRLen::List:");
      foreach (Fraction f, flist)
            qDebug("  %d/%d", f.numerator(), f.denominator());

      int tick       = cr->tick();
      Fraction f     = dstF;
      ChordRest* cr1 = cr;
      Chord* oc      = 0;

      bool first = true;
      foreach (Fraction f2, flist) {
            f  -= f2;
            makeGap(cr1->segment(), cr1->track(), f2, tuplet);

            if (cr->type() == Element::REST) {
qDebug("  +ChangeCRLen::setRest %d/%d", f2.numerator(), f2.denominator());
                  Fraction timeStretch = cr1->staff()->timeStretch(cr1->tick());
                  Rest* r = setRest(tick, track, f2 * timeStretch, (d.dots() > 0), tuplet);
                  if (first) {
                        select(r, SELECT_SINGLE, 0);
                        first = false;
                        }
qDebug("  ChangeCRLen:: %d += %d(actual=%d)", tick, f2.ticks(), f2.ticks() * timeStretch.numerator() / timeStretch.denominator());
                  tick += f2.ticks() * timeStretch.numerator() / timeStretch.denominator();
                  }
            else {
                  QList<TDuration> dList = toDurationList(f2, true);
                  Measure* measure = tick2measure(tick);
                  int etick = measure->tick();
//                  if (measure->tick() != tick)
//                        etick += measure->ticks();
                  if (((tick - etick) % dList[0].ticks()) == 0) {
                        foreach(TDuration d, dList) {
                              bool genTie;
                              Chord* cc;
                              if (oc) {
                                    genTie = true;
                                    cc = oc;
                                    }
                              else {
                                    genTie = false;
                                    cc = static_cast<Chord*>(cr);
                                    }
                              oc = addChord(tick, d, cc, genTie, tuplet);
                              if (oc && first) {
                                    select(oc, SELECT_SINGLE, 0);
                                    first = false;
                                    }
                              if (oc)
                                    tick += oc->actualTicks();
                              }
                        }
                  else {
                        for (int i = dList.size() - 1; i >= 0; --i) {
                              bool genTie;
                              Chord* cc;
                              if (oc) {
                                    genTie = true;
                                    cc = oc;
                                    }
                              else {
                                    genTie = false;
                                    cc = static_cast<Chord*>(cr);
                                    }
                              oc = addChord(tick, dList[i], cc, genTie, tuplet);
                              if (first) {
                                    select(oc, SELECT_SINGLE, 0);
                                    first = false;
                                    }
                              tick += oc->actualTicks();
                              }
                        }
                  }
            Measure* m  = cr1->measure();
            Measure* m1 = m->nextMeasure();
            if (m1 == 0)
                  break;
            Segment* s = m1->firstCRSegment();
            expandVoice(s, track);
            cr1 = static_cast<ChordRest*>(s->element(track));
            }
      connectTies();
      }

//---------------------------------------------------------
//   upDown
///   Increment/decrement pitch of note by one or by an octave.
//---------------------------------------------------------

void Score::upDown(bool up, UpDownMode mode)
      {
      QList<Note*> el;
      int tick = -1;
      foreach (Note* note, selection().noteList()) {
            while (note->tieBack())
                  note = note->tieBack()->startNote();
            for (; note; note = note->tieFor() ? note->tieFor()->endNote() : 0) {
                  if (el.indexOf(note) == -1) {
                        el.append(note);
                        if (tick == -1)
                              tick = note->chord()->tick();
                        }
                  }
            }
      if (el.empty())
            return;

      foreach(Note* oNote, el) {
            Part* part  = oNote->staff()->part();
            int pitch   = oNote->pitch();
            int newTpc = 0;
            int newPitch = 0;
            int string = oNote->string();
            int fret   = oNote->fret();
            Tablature* tab;

            switch(oNote->staff()->staffType()->group()) {
                  case PERCUSSION_STAFF:
                        {
                        Drumset* ds = part->instr()->drumset();
                        newPitch    = up ? ds->prevPitch(pitch) : ds->nextPitch(pitch);
                        newTpc      = oNote->tpc();
                        }
                        break;
                  case TAB_STAFF:
                        {
                        tab = part->instr()->tablature();
                        switch(mode) {
                              case UP_DOWN_OCTAVE:          // move same note to next string, if possible
                                    {
                                    StaffTypeTablature * stt = static_cast<StaffTypeTablature*>(oNote->staff()->staffType());
                                    string = stt->physStringToVisual(string);
                                    string += (up ? -1 : 1);
                                    if(string < 0 || string >= tab->strings())
                                          return;           // no next string to move to
                                    string = stt->VisualStringToPhys(string);
                                    fret = tab->fret(pitch, string);
                                    if(fret == -1)          // can't have that note on that string
                                          return;
                                    newPitch = pitch;       // these didn't change
                                    newTpc   = oNote->tpc();
                                    }
                                    break;

                              case UP_DOWN_CHROMATIC:       // increase / decrease the pitch,
                                                            // letting the algorithm to choose fret & string
                                    newPitch = up ? pitch+1 : pitch-1;
                                    if (newPitch < 0)
                                          newPitch = 0;
                                    else if (newPitch > 127)
                                          newPitch = 127;
                                    newTpc = pitch2tpc2(newPitch, up);
                                    break;

                              case UP_DOWN_DIATONIC:        // increase / decrease the fret
                                    {                       // without changing the string
                                    fret += (up ? 1 : -1);
                                    if (fret < 0)
                                          fret = 0;
                                    else if (fret >= tab->frets())
                                          fret = tab->frets() - 1;
                                    newPitch    = tab->getPitch(string, fret);
                                    newTpc      = pitch2tpc2(newPitch, up);
                                    // store the fretting change before undoChangePitch() chooses
                                    // a fretting of its own liking!
                                    undoChangeProperty(oNote, P_FRET, fret);
                                    undoChangeProperty(oNote, P_STRING, string);
                                    }
                                    break;
                              }
                        }
                        break;
                  case PITCHED_STAFF:
                        switch(mode) {
                              case UP_DOWN_OCTAVE:
                                    newPitch = pitch + (up ? 12 : -12);
                                    if (newPitch < 0)
                                          newPitch = 0;
                                    else if (newPitch > 127)
                                          newPitch = 127;
                                    newTpc = oNote->tpc();
                                    break;

                              case UP_DOWN_CHROMATIC:
                                    newPitch = up ? pitch+1 : pitch-1;
                                    if (newPitch < 0)
                                          newPitch = 0;
                                    else if (newPitch > 127)
                                          newPitch = 127;
                                    newTpc = pitch2tpc2(newPitch, up);
                                    break;

                              case UP_DOWN_DIATONIC:
                                    {
                                    Chord* chord  = oNote->chord();
                                    Staff* estaff = staff(chord->staffIdx() + chord->staffMove());
                                    int clef      = estaff->clef(chord->tick());
                                    int line      = oNote->line() + (up ? -1 : 1);
                                    newPitch      = line2pitch(line, clef, 0);
                                    int step      = clefTable[clef].pitchOffset - line;
                                    while (step < 0)
                                          step += 7;
                                    step %= 7;
                                    newTpc = step2tpc(step, NATURAL);
                                    }
                                    break;
                              }
                        break;
                  }
            _is.pitch = newPitch;

            if ( (oNote->pitch() != newPitch) || (oNote->tpc() != newTpc) )
                  undoChangePitch(oNote, newPitch, newTpc, oNote->line()/*, fret, string*/);
            // store fret change only if undoChangePitch has not been called,
            // as undoChangePitch() already manages fret changes, if necessary
            else if( oNote->staff()->staffType()->group() == TAB_STAFF) {
                  bool refret = false;
                  if (oNote->string() != string) {
                        undoChangeProperty(oNote, P_STRING, string);
                        refret = true;
                        }
                  if (oNote->fret() != fret) {
                        undoChangeProperty(oNote, P_FRET, fret);
                        refret = true;
                        }
                  if (refret)
                        tab->fretChord(oNote->chord());
                  }

            // play new note with velocity 80 for 0.3 sec:
            _playNote = true;
            }
      _selection.updateState();     // accidentals may have changed
      }

//---------------------------------------------------------
//   addArticulation
///   Add attribute \a attr to all selected notes/rests.
///
///   Called from padToggle() to add note prefix/accent.
//---------------------------------------------------------

void Score::addArticulation(ArticulationType attr)
      {
      foreach(Element* el, selection().elements()) {
            if (el->type() == Element::NOTE || el->type() == Element::CHORD) {
                  Articulation* na = new Articulation(this);
                  na->setSubtype(attr);
                  addArticulation(el, na);
                  }
            }
      }

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \a idx for all selected
///   notes.
//---------------------------------------------------------

void Score::changeAccidental(Accidental::AccidentalType idx)
      {
      foreach(Note* note, selection().noteList())
            changeAccidental(note, idx);
      }

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \accidental for
///   note \a note.
//---------------------------------------------------------

void Score::changeAccidental(Note* note, Accidental::AccidentalType accidental)
      {
      QList<Staff*> staffList;
      Staff* ostaff = note->chord()->staff();
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);

      Chord* chord     = note->chord();
      Segment* segment = chord->segment();
      int voice        = chord->voice();
      Measure* measure = segment->measure();
      int tick         = segment->tick();
      int noteIndex    = chord->notes().indexOf(note);
      Staff* estaff    = staff(chord->staffIdx() + chord->staffMove());
      int clef         = estaff->clef(tick);
      int step         = clefTable[clef].pitchOffset - note->line();
      while (step < 0)
            step += 7;
      step %= 7;
      //
      // accidental change may result in pitch change
      //
      AccidentalVal acc    = Accidental::subtype2value(accidental);
      AccidentalVal acc2   = measure->findAccidental(note);
      Accidental::AccidentalType accType;

      int pitch, tpc;
      if (accidental == Accidental::ACC_NONE) {
            //
            //  delete accidentals
            //
            accType = Accidental::ACC_NONE;
            pitch   = line2pitch(note->line(), clef, 0) + acc2;
            tpc     = step2tpc(step, acc2);
            // check if there's accidentals left, previously set as
            // precautionary accidentals
            Accidental *a_rem = note->accidental();
            if (a_rem)
                  undoRemoveElement(note->accidental());
            }
      else {
            if (acc2 == acc) {
                  //
                  // this is a precautionary accidental
                  //
                  accType = accidental;
                  pitch = line2pitch(note->line(), clef, 0) + Accidental::subtype2value(accType);
                  tpc   = step2tpc(step, acc);

                  Accidental* a = new Accidental(this);
                  a->setParent(note);
                  a->setSubtype(accidental);
                  a->setRole(Accidental::ACC_USER);
                  undoAddElement(a);
                  }
            else {
                  accType = accidental;
                  pitch = line2pitch(note->line(), clef, 0) + Accidental::subtype2value(accType);
                  tpc   = step2tpc(step, acc);
                  }
            }

      foreach(Staff* st, staffList) {
            Score* score = st->score();
            Measure* m;
            Segment* s;
            if (score == this) {
                  m = measure;
                  s = segment;
                  }
            else {
                  m   = score->tick2measure(measure->tick());
                  s   = m->findSegment(segment->subtype(), segment->tick());
                  }
            int staffIdx  = score->staffIdx(st);
            Chord* chord  = static_cast<Chord*>(s->element(staffIdx * VOICES + voice));
            Note* n       = chord->notes().at(noteIndex);

            int fret   = n->fret();
            int string = n->string();
            if (st->isTabStaff()) {
                  if (pitch != n->pitch()) {
                        //
                        // as pitch has changed, calculate new
                        // string & fret
                        //
                        Tablature* tab = n->staff()->part()->instr()->tablature();
                        if (tab)
                              tab->convertPitch(pitch, &string, &fret);
                        }
                  }
            undo(new ChangePitch(n, pitch, tpc, n->line()));
            if (!st->isTabStaff()) {
                  //
                  // handle ties
                  //
                  if (n->tieBack()) {
                        undoRemoveElement(n->tieBack());
                        if (n->tieFor())
                              undoRemoveElement(n->tieFor());
                        }
                  else {
                        Note* nn = n;
                        while (nn->tieFor()) {
                              nn = nn->tieFor()->endNote();
                              undo(new ChangePitch(nn, pitch, tpc, nn->line()));
                              }
                        }
                  }
            //
            // recalculate needed accidentals for
            // whole measure
            //
            score->updateAccidentals(m, staffIdx);
            }
      }

//---------------------------------------------------------
//   addArticulation
//---------------------------------------------------------

void Score::addArticulation(Element* el, Articulation* atr)
      {
      ChordRest* cr;
      if (el->type() == Element::NOTE)
            cr = static_cast<ChordRest*>(((Note*)el)->chord());
      else if (el->type() == Element::REST)
            cr = static_cast<ChordRest*>(el);
      else if (el->type() == Element::CHORD)
            cr = static_cast<ChordRest*>(el);
      else {
            delete atr;
            return;
            }
      atr->setParent(cr);
      Articulation* oa = cr->hasArticulation(atr);
      if (oa) {
            delete atr;
            atr = 0;
            undoRemoveElement(oa);
            }
      else
            undoAddElement(atr);
      }

//---------------------------------------------------------
//   resetUserStretch
//---------------------------------------------------------

void Score::resetUserStretch()
      {
      Measure* m1;
      Measure* m2;
      // retrieve span of selection
      Segment* s1 = _selection.startSegment();
      Segment* s2 = _selection.endSegment();
      // if either segment is not returned by the selection
      // (for instance, no selection) fall back to first/last measure
      if(!s1)
            m1 = firstMeasure();
      else
            m1 = s1->measure();
      if(!s2)
            m2 = lastMeasure();
      else
            m2 = s2->measure();
      if(!m1 || !m2)                // should not happen!
            return;

      for (Measure* m = m1; m; m = m->nextMeasure()) {
            _undo->push(new ChangeStretch(m, 1.0));
            if (m == m2)
                  break;
            }
      _layoutAll = true;
      }

//---------------------------------------------------------
//   moveUp
//---------------------------------------------------------

void Score::moveUp(Chord* chord)
      {
      int rstaff    = chord->staff()->rstaff();
      int staffMove = chord->staffMove();

      if ((staffMove == -1) || (rstaff + staffMove <= 0))
            return;
      undo(new ChangeChordStaffMove(chord, staffMove - 1));
      }

//---------------------------------------------------------
//   moveDown
//---------------------------------------------------------

void Score::moveDown(Chord* chord)
      {
      Staff* staff  = chord->staff();
      Part* part    = staff->part();
      int rstaff    = staff->rstaff();
      int rstaves   = part->nstaves();
      int staffMove = chord->staffMove();

      if ((staffMove == 1) || (rstaff + staffMove >= rstaves - 1)) {
qDebug("moveDown staffMove==%d  rstaff %d rstaves %d", staffMove, rstaff, rstaves);
            return;
            }
      undo(new ChangeChordStaffMove(chord, staffMove + 1));
      _layoutAll = true;
      }

//---------------------------------------------------------
//   cmdAddStretch
//---------------------------------------------------------

void Score::cmdAddStretch(qreal val)
      {
      if (selection().state() != SEL_RANGE)
            return;
      int startTick = selection().tickStart();
      int endTick   = selection().tickEnd();
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            if (m->tick() < startTick)
                  continue;
            if (m->tick() >= endTick)
                  break;
            qreal stretch = m->userStretch();
            stretch += val;
            undo(new ChangeStretch(m, stretch));
            }
      _layoutAll = true;
      }

//---------------------------------------------------------
//   cmdInsertClef
//---------------------------------------------------------

void Score::cmdInsertClef(ClefType type)
      {
      if (!noteEntryMode())
            return;
      undoChangeClef(staff(inputTrack()/VOICES), inputState().segment(), type);
      }

//---------------------------------------------------------
//   cmdResetBeamMode
//---------------------------------------------------------

void Score::cmdResetBeamMode()
      {
      if (selection().state() != SEL_RANGE) {
            qDebug("no system or staff selected");
            return;
            }
      int startTick = selection().tickStart();
      int endTick   = selection().tickEnd();

      Segment::SegmentTypes st = Segment::SegChordRestGrace;
      for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            if (seg->tick() < startTick)
                  continue;
            if (seg->tick() >= endTick)
                  break;
            for (int track = 0; track < nstaves() * VOICES; ++track) {
                  ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
                  if (cr == 0)
                        continue;
                  if (cr->type() == Element::CHORD) {
                        if (cr->beamMode() != BEAM_AUTO)
                              undoChangeProperty(cr, P_BEAM_MODE, int(BEAM_AUTO));
                        }
                  else if (cr->type() == Element::REST) {
                        if (cr->beamMode() != BEAM_NO)
                              undoChangeProperty(cr, P_BEAM_MODE, int(BEAM_NO));
                        }
                  }
            }
      _layoutAll = true;
      }

//---------------------------------------------------------
//   processMidiInput
//---------------------------------------------------------

bool Score::processMidiInput()
      {
      if (MScore::debugMode)
          qDebug("processMidiInput");
      if (midiInputQueue.isEmpty())
            return false;

      bool cmdActive = false;
      Note* n = 0;
      while (!midiInputQueue.isEmpty()) {
            MidiInputEvent ev = midiInputQueue.dequeue();
            if (MScore::debugMode)
                  qDebug("<-- !noteentry dequeue %i", ev.pitch);
            if (!noteEntryMode()) {
                  int staffIdx = selection().staffStart();
                  Part* p;
                  if (staffIdx < 0 || staffIdx >= nstaves())
                        p = staff(0)->part();
                  else
                        p = staff(staffIdx)->part();
                  if (p)
                        MScore::seq->startNote(p->instr()->channel(0).channel, ev.pitch, 80,
                           MScore::defaultPlayDuration, 0.0);
                  }
            else  {
                  if (!cmdActive) {
                        startCmd();
                        cmdActive = true;
                        }
                  Note* n2 = addPitch(ev.pitch, ev.chord);
                  if (n2)
                        n = n2;
                  }
            }
      if (cmdActive) {
            _layoutAll = true;
            endCmd();
            //after relayout
            if (n) {
                  foreach(MuseScoreView* v, viewer)
                        v->adjustCanvasPosition(n, false);
                  }
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   moveInputPos
//---------------------------------------------------------

void Score::moveInputPos(Segment* s)
      {
      if (s == 0)
            return;
      _is.setSegment(s);
//      emit posChanged(s->tick());
#if 0
      Element* el;
      if (s->element(_is.track()))
            el = s->element(_is.track());
      else
            el = s->element(_is.track() / VOICES * VOICES);
      if (el->type() == CHORD)
            el = static_cast<Chord*>(el)->upNote();
      emit adjustCanvasPosition(el, false);
#endif
      }

//---------------------------------------------------------
//   moveToPrevInputPos
//   Derived from moveToNextInputPos()
//---------------------------------------------------------

void Score::moveToPrevInputPos()
      {
      Segment* s = _is.segment();
      Measure* m = s->measure();
      int track  = _is.track();
      for (s = s->prev1(Segment::SegChordRest); s; s = s->prev1(Segment::SegChordRest)) {
            if (s->element(track) || s->measure() != m)
                  break;
            }
      moveInputPos(s);
      }
//---------------------------------------------------------
//   moveToNextInputPos
//   TODO: special case: note is first note of tie: goto to last note of tie
//---------------------------------------------------------

void Score::moveToNextInputPos()
      {
      Segment* s = _is.segment();
      Measure* m = s->measure();
      int track  = _is.track();
      for (s = s->next1(Segment::SegChordRest); s; s = s->next1(Segment::SegChordRest)) {
            if (s->element(track) || s->measure() != m)
                  break;
            }
      moveInputPos(s);
      }

//---------------------------------------------------------
//   move
//    move current selection
//---------------------------------------------------------

Element* Score::move(const QString& cmd)
      {
      ChordRest* cr;
      if (selection().activeCR())
            cr = selection().activeCR();
      else
            cr = selection().lastChordRest();
      if (cr == 0 && inputState().noteEntryMode)
            cr = inputState().cr();

      // no chord/rest found? look for another type of element
      if (cr == 0) {
            Element* el  = 0;
            Element* trg = 0;
            // retrieve last element of section list
            if (!selection().elements().isEmpty())
                  el = selection().elements().last();
            if(!el)                             // no element, no party!
                  return 0;
            // get parent of element and process accordingly:
            // trg is the element to select on "next-chord" cmd
            // cr is the ChordRest to move from on other cmd's
            int track = el->track();            // keep note of element track
            el = el->parent();
            switch (el->type()) {
                  case Element::NOTE:           // a note is a valid target
                        trg = el;
                        cr  = static_cast<Note*>(el)->chord();
                        break;
                  case Element::CHORD:          // a chord or a rest are valid targets
                  case Element::REST:
                        trg = el;
                        cr  = static_cast<ChordRest*>(trg);
                        break;
                  case Element::SEGMENT: {      // from segment go to top chordrest in segment
                        Segment* seg  = static_cast<Segment*>(el);
                        // if segment is not chord/rest or grace, move to next chord/rest or grace segment
                        if (!seg->isChordRest() && !seg->isGrace()) {
                              seg = seg->next1(Segment::SegChordRestGrace);
                              if (seg == 0)     // if none found, reutrn failure
                                    return 0;
                              }
                        // segment for sure contains chords/rests,
                        int size = seg->elist().size();
                        // if segment has a chord/rest in original element track, use it
                        if(track > -1 && track < size && seg->element(track)) {
                              trg  = seg->element(track);
                              cr = static_cast<ChordRest*>(trg);
                              break;
                              }
                        // if not, get topmost chord/rest
                        for (int i = 0; i < size; i++)
                              if (seg->element(i)) {
                                    trg  = seg->element(i);
                                    cr = static_cast<ChordRest*>(trg);
                                    break;
                                    }
                        break;
                        }
                  default:                      // on anything else, return failure
                        return 0;
                  }

            // if something found and command is forward, the element found is the destination
            if (trg && cmd == "next-chord") {
                  // if chord, go to topmost note
                  if (trg->type() == Element::CHORD)
                        trg = static_cast<Chord*>(trg)->upNote();
                  _playNote = true;
                  select(trg, SELECT_SINGLE, 0);
                  return trg;
                  }
            // if no chordrest found, do nothing
            if(cr == 0)
                  return 0;
            // if some chordrest found, continue with default processing
            }

      Element* el = 0;
      if (cmd == "next-chord") {
            if (noteEntryMode())
                  moveToNextInputPos();
            el = nextChordRest(cr, false);
            }
      else if (cmd == "prev-chord") {
            if (noteEntryMode()) {
                  Segment* s = _is.segment()->prev1();
                  //
                  // if _is._segment is first chord/rest segment in measure
                  // make sure "m" points to previous measure
                  //
                  while (s && s->subtype() != Segment::SegChordRest)
                        s = s->prev1();
                  if (s == 0)
                        return 0;
                  Measure* m = s->measure();

                  int track  = _is.track();
                  for (; s; s = s->prev1()) {
                        if (s->subtype() != Segment::SegChordRest)
                              continue;
                        if (s->element(track) || s->measure() != m)
                              break;
                        }
                  if (s && !s->element(track))
                        s = m->firstCRSegment();
                  moveInputPos(s);
                  }
            el = prevChordRest(cr, false);
            }
      else if (cmd == "next-measure") {
            el = nextMeasure(cr);
            if (noteEntryMode() && el && (el->type() == Element::CHORD || el->type() == Element::REST)){
                  ChordRest* crc = static_cast<ChordRest*>(el);
                  moveInputPos(crc->segment());
                  }
            }
      else if (cmd == "prev-measure") {
            el = prevMeasure(cr);
            if (noteEntryMode() && el && (el->type() == Element::CHORD || el->type() == Element::REST)){
                  ChordRest* crc = static_cast<ChordRest*>(el);
                  moveInputPos(crc->segment());
                  }
            }
      if (el) {
            if (el->type() == Element::CHORD)
                  el = static_cast<Chord*>(el)->downNote();
            _playNote = true;
            select(el, SELECT_SINGLE, 0);
            }
      return el;
      }

//---------------------------------------------------------
//   selectMove
//---------------------------------------------------------

Element* Score::selectMove(const QString& cmd)
      {
      ChordRest* cr;
      if (selection().activeCR())
            cr = selection().activeCR();
      else
            cr = selection().lastChordRest();
      if (cr == 0 && inputState().noteEntryMode)
            cr = inputState().cr();
      if (cr == 0)
            return 0;

      ChordRest* el = 0;
      if (cmd == "select-next-chord")
            el = nextChordRest(cr);
      else if (cmd == "select-prev-chord")
            el = prevChordRest(cr);
      else if (cmd == "select-next-measure")
            el = nextMeasure(cr, true);
      else if (cmd == "select-prev-measure")
            el = prevMeasure(cr);
      else if (cmd == "select-begin-line") {
            Measure* measure = cr->segment()->measure()->system()->firstMeasure();
            if (!measure)
                  return 0;
            el = measure->first()->nextChordRest(cr->track());
            }
      else if (cmd == "select-end-line") {
            Measure* measure = cr->segment()->measure()->system()->lastMeasure();
            if (!measure)
                  return 0;
            el = measure->last()->nextChordRest(cr->track(), true);
            }
      else if (cmd == "select-begin-score") {
            Measure* measure = first()->system()->firstMeasure();
            if (!measure)
                  return 0;
            el = measure->first()->nextChordRest(cr->track());
            }
      else if (cmd == "select-end-score") {
            Measure* measure = last()->system()->lastMeasure();
            if (!measure)
                  return 0;
            el = measure->last()->nextChordRest(cr->track(), true);
            }
      else if (cmd == "select-staff-above")
            el = upStaff(cr);
      else if (cmd == "select-staff-below")
            el = downStaff(cr);
      if (el)
            select(el, SELECT_RANGE, el->staffIdx());
      return el;
      }

//---------------------------------------------------------
//   cmdMirrorNoteHead
//---------------------------------------------------------

void Score::cmdMirrorNoteHead()
      {
      const QList<Element*>& el = selection().elements();
      foreach(Element* e, el) {
            if (e->type() == Element::NOTE) {
                  Note* note = static_cast<Note*>(e);
                  if (note->staff() && note->staff()->isTabStaff())
                        note->score()->undoChangeProperty(e, P_GHOST, true);
                  else {
                        MScore::DirectionH d = note->userMirror();
                        if (d == MScore::DH_AUTO)
                              d = note->chord()->up() ? MScore::DH_RIGHT : MScore::DH_LEFT;
                        else
                              d = d == MScore::DH_LEFT ? MScore::DH_RIGHT : MScore::DH_LEFT;
                        undoChangeUserMirror(note, d);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   cmdHalfDuration
//---------------------------------------------------------

void Score::cmdHalfDuration()
      {
      Element* el = selection().element();
      if (el == 0)
            return;
      if (el->type() == Element::NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = static_cast<ChordRest*>(el);
      TDuration d = _is.duration().shift(1);
      if (!d.isValid() || (d.type() > TDuration::V_64TH))
            return;
      if (cr->type() == Element::CHORD && (static_cast<Chord*>(cr)->noteType() != NOTE_NORMAL)) {
            //
            // handle appoggiatura and acciaccatura
            //
            cr->setDurationType(d);
            }
      else
            changeCRlen(cr, d);
      _is.setDuration(d);
      nextInputPos(cr, false);
      }

//---------------------------------------------------------
//   cmdDoubleDuration
//---------------------------------------------------------

void Score::cmdDoubleDuration()
      {
      Element* el = selection().element();
      if (el == 0)
            return;
      if (el->type() == Element::NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = static_cast<ChordRest*>(el);
      TDuration d = _is.duration().shift(-1);
      if (!d.isValid() || (d.type() < TDuration::V_WHOLE))
            return;
      if (cr->type() == Element::CHORD && (static_cast<Chord*>(cr)->noteType() != NOTE_NORMAL)) {
            //
            // handle appoggiatura and acciaccatura
            //
            cr->setDurationType(d);
            }
      else
            changeCRlen(cr, d);
      _is.setDuration(d);
      nextInputPos(cr, false);
      }

//---------------------------------------------------------
//   cmdMoveRest
//---------------------------------------------------------

void Score::cmdMoveRest(Rest* rest, MScore::Direction dir)
      {
      QPointF pos(rest->userOff());
      if (dir == MScore::UP)
            pos.ry() -= spatium();
      else if (dir == MScore::DOWN)
            pos.ry() += spatium();
      undoChangeProperty(rest, P_USER_OFF, pos);
      setLayoutAll(false);
      }

//---------------------------------------------------------
//   cmdMoveLyrics
//---------------------------------------------------------

void Score::cmdMoveLyrics(Lyrics* lyrics, MScore::Direction dir)
      {
      ChordRest* cr      = lyrics->chordRest();
      QList<Lyrics*>& ll = cr->lyricsList();
      int no             = lyrics->no();
      if (dir == MScore::UP) {
            if (no) {
                  if (ll[no-1] == 0) {
                        ll[no-1] = ll[no];
                        ll[no] = 0;
                        lyrics->setNo(no-1);
                        }
                  }
            }
      else {
            if (no == ll.size()-1) {
                  ll.append(ll[no]);
                  ll[no] = 0;
                  lyrics->setNo(no+1);
                  }
            else if (ll[no + 1] == 0) {
                  ll[no+1] = ll[no];
                  ll[no] = 0;
                  lyrics->setNo(no+1);
                  }
            }
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void Score::cmd(const QAction* a)
      {
      QString cmd(a ? a->data().toString() : "");
      if (MScore::debugMode)
            qDebug("Score::cmd <%s>", qPrintable(cmd));

      //
      // Hack for moving articulations while selected
      //
      Element* el = selection().element();
      if (cmd == "pitch-up") {
            if (el && (el->type() == Element::ARTICULATION || el->type() == Element::FINGERING))
                  undoMove(el, el->userOff() + QPointF(0.0, -MScore::nudgeStep * el->spatium()));
            else if (el && el->type() == Element::REST)
                  cmdMoveRest(static_cast<Rest*>(el), MScore::UP);
            else if (el && el->type() == Element::LYRICS)
                  cmdMoveLyrics(static_cast<Lyrics*>(el), MScore::UP);
            else
                  upDown(true, UP_DOWN_CHROMATIC);
            }
      else if (cmd == "pitch-down") {
            if (el && (el->type() == Element::ARTICULATION || el->type() == Element::FINGERING))
                  undoMove(el, el->userOff() + QPointF(0.0, MScore::nudgeStep * el->spatium()));
            else if (el && el->type() == Element::REST)
                  cmdMoveRest(static_cast<Rest*>(el), MScore::DOWN);
            else if (el && el->type() == Element::LYRICS)
                  cmdMoveLyrics(static_cast<Lyrics*>(el), MScore::DOWN);
            else
                  upDown(false, UP_DOWN_CHROMATIC);
            }
	else if (cmd == "add-staccato")
            addArticulation(Articulation_Staccato);
	else if (cmd == "add-tenuto")
            addArticulation(Articulation_Tenuto);
      else if (cmd == "add-marcato")
            addArticulation(Articulation_Marcato);
	else if (cmd == "add-trill")
            addArticulation(Articulation_Trill);
      else if (cmd == "add-hairpin")
            cmdAddHairpin(false);
      else if (cmd == "add-hairpin-reverse")
            cmdAddHairpin(true);
      else if (cmd == "delete-measures")
            cmdDeleteSelectedMeasures();
      else if (cmd == "time-delete") {
            // TODO:
            // remove measures if stave-range is 0-nstaves()
            cmdDeleteSelectedMeasures();
            }
      else if (cmd == "pitch-up-octave")
            upDown(true, UP_DOWN_OCTAVE);
      else if (cmd == "pitch-down-octave")
            upDown(false, UP_DOWN_OCTAVE);
      else if (cmd == "pitch-up-diatonic")
            upDown(true, UP_DOWN_DIATONIC);
      else if (cmd == "pitch-down-diatonic")
            upDown(false, UP_DOWN_DIATONIC);
      else if (cmd == "move-up") {
            setLayoutAll(false);
            Element* el = selection().element(); // single selection
            if (el && el->type() == Element::NOTE) {
                  Note* note = static_cast<Note*>(el);
                  moveUp(note->chord());
                  }
            }
      else if (cmd == "move-down") {
            setLayoutAll(false);
            Element* el = selection().element(); // single selection
            if (el && el->type() == Element::NOTE) {
                  Note* note = static_cast<Note*>(el);
                  moveDown(note->chord());
                  }
            }
      else if (cmd == "up-chord") {
            Element* el = selection().element(); // single selection
            if (el && (el->type() == Element::NOTE || el->type() == Element::REST)) {
                  Element* e = upAlt(el);
                  if (e) {
                        if (e->type() == Element::NOTE) {
                              _is.pitch = static_cast<Note*>(e)->pitch();
                              _playNote = true;
                              }
                        select(e, SELECT_SINGLE, 0);
                        }
                  }
            setLayoutAll(false);
            }
      else if (cmd == "down-chord") {
            Element* el = selection().element(); // single selection
            if (el && (el->type() == Element::NOTE || el->type() == Element::REST)) {
                  Element* e = downAlt(el);
                  if (e) {
                        if (e->type() == Element::NOTE) {
                              _is.pitch = static_cast<Note*>(e)->pitch();
                              _playNote = true;
                              }
                        select(e, SELECT_SINGLE, 0);
                        }
                  }
            setLayoutAll(false);
            }
      else if (cmd == "top-chord" ) {
            Element* el = selection().element(); // single selection
            if (el && el->type() == Element::NOTE) {
                  Element* e = upAltCtrl(static_cast<Note*>(el));
                  if (e) {
                        if (e->type() == Element::NOTE) {
                              _is.pitch = static_cast<Note*>(e)->pitch();
                              _playNote = true;
                              }
                        select(e, SELECT_SINGLE, 0);
                        }
                  }
            setLayoutAll(false);
            }
      else if (cmd == "bottom-chord") {
            Element* el = selection().element(); // single selection
            if (el && el->type() == Element::NOTE) {
                  Element* e = downAltCtrl(static_cast<Note*>(el));
                  if (e) {
                        if (e->type() == Element::NOTE) {
                              _is.pitch = static_cast<Note*>(e)->pitch();
                              _playNote = true;
                              }
                        select(e, SELECT_SINGLE, 0);
                        }
                  }
            setLayoutAll(false);
            }
      else if (cmd == "note-longa")
            padToggle(PAD_NOTE00);
      else if (cmd == "note-breve")
            padToggle(PAD_NOTE0);
      else if (cmd == "pad-note-1")
            padToggle(PAD_NOTE1);
      else if (cmd == "pad-note-2")
            padToggle(PAD_NOTE2);
      else if (cmd == "pad-note-4")
            padToggle(PAD_NOTE4);
      else if (cmd == "pad-note-8")
            padToggle(PAD_NOTE8);
      else if (cmd == "pad-note-16")
            padToggle(PAD_NOTE16);
      else if (cmd == "pad-note-32")
            padToggle(PAD_NOTE32);
      else if (cmd == "pad-note-64")
            padToggle(PAD_NOTE64);
      else if (cmd == "pad-note-128")
            padToggle(PAD_NOTE128);
      else if (cmd == "pad-rest")
            padToggle(PAD_REST);
      else if (cmd == "pad-dot")
            padToggle(PAD_DOT);
      else if (cmd == "pad-dotdot")
            padToggle(PAD_DOTDOT);
      else if (cmd == "beam-start")
            cmdSetBeamMode(BEAM_BEGIN);
      else if (cmd == "beam-mid")
            cmdSetBeamMode(BEAM_MID);
      else if (cmd == "no-beam")
            cmdSetBeamMode(BEAM_NO);
      else if (cmd == "beam-32")
            cmdSetBeamMode(BEAM_BEGIN32);
      else if (cmd == "sharp2")
            changeAccidental(Accidental::ACC_SHARP2);
      else if (cmd == "sharp")
            changeAccidental(Accidental::ACC_SHARP);
      else if (cmd == "nat")
            changeAccidental(Accidental::ACC_NATURAL);
      else if (cmd == "flat")
            changeAccidental(Accidental::ACC_FLAT);
      else if (cmd == "flat2")
            changeAccidental(Accidental::ACC_FLAT2);
      else if (cmd == "repitch")
            _is.setRepitchMode(a->isChecked());
      else if (cmd == "flip")
            cmdFlip();
      else if (cmd == "stretch+")
            cmdAddStretch(0.1);
      else if (cmd == "stretch-")
            cmdAddStretch(-0.1);
      else if (cmd == "pitch-spell")
            spell();
      else if (cmd == "select-all")
            cmdSelectAll();
      else if (cmd == "select-section")
            cmdSelectSection();
      else if (cmd == "concert-pitch") {
            if (styleB(ST_concertPitch) != a->isChecked())
                  cmdConcertPitchChanged(a->isChecked(), true);
            }
      else if (cmd == "reset-beammode")
            cmdResetBeamMode();
      else if (cmd == "clef-violin")
            cmdInsertClef(CLEF_G);
      else if (cmd == "clef-bass")
            cmdInsertClef(CLEF_F);
      else if (cmd == "voice-x12")
            cmdExchangeVoice(0, 1);
      else if (cmd == "voice-x13")
            cmdExchangeVoice(0, 2);
      else if (cmd == "voice-x14")
            cmdExchangeVoice(0, 3);
      else if (cmd == "voice-x23")
            cmdExchangeVoice(1, 2);
      else if (cmd == "voice-x24")
            cmdExchangeVoice(1, 3);
      else if (cmd == "voice-x34")
            cmdExchangeVoice(2, 3);
      else if (cmd == "system-break" || cmd == "page-break" || cmd == "section-break") {
            LayoutBreakType type;
            if (cmd == "system-break")
                  type = LAYOUT_BREAK_LINE;
            else if (cmd == "page-break")
                  type = LAYOUT_BREAK_PAGE;
            else
                  type = LAYOUT_BREAK_SECTION;

            Element* e = selection().element();
            if (e && e->type() == Element::BAR_LINE && e->parent()->type() == Element::SEGMENT) {
                  Measure* measure = static_cast<Measure*>(e->parent()->parent());
                  if (!measure->lineBreak()) {
                        LayoutBreak* lb = new LayoutBreak(this);
                        lb->setSubtype(type);
                        lb->setTrack(-1);       // this are system elements
                        lb->setParent(measure);
                        undoAddElement(lb);
                        }
                  else {
                        // remove line break
                        foreach(Element* e, *measure->el()) {
                              if (e->type() == Element::LAYOUT_BREAK && static_cast<LayoutBreak*>(e)->subtype() ==type) {
                                    undoRemoveElement(e);
                                    break;
                                    }
                              }
                        }
                  }
            }
      else if (cmd == "reset-stretch")
            resetUserStretch();
      else if (cmd == "mirror-note")
            cmdMirrorNoteHead();
      else if (cmd == "double-duration")
            cmdDoubleDuration();
      else if (cmd == "half-duration")
            cmdHalfDuration();
      else if (cmd == "") {               //Midi note received only?
            if (!noteEntryMode())
                  setLayoutAll(false);
            }
      else if (cmd == "add-audio")
            addAudioTrack();
      else if (cmd == "transpose-up")
            transposeSemitone(1);
      else if (cmd == "transpose-down")
            transposeSemitone(-1);
      else
            qDebug("1unknown cmd <%s>", qPrintable(cmd));
      }

