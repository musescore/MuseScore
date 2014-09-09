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
 Handling of several GUI commands.
*/

#include <assert.h>

#include "score.h"
#include "utils.h"
#include "key.h"
#include "clef.h"
#include "navigate.h"
#include "slur.h"
#include "tie.h"
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
#include "stringdata.h"
#include "stafftype.h"
#include "segment.h"
#include "chordlist.h"
#include "mscore.h"
#include "accidental.h"
#include "sequencer.h"
#include "tremolo.h"

namespace Ms {

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

      for (Score* s : scoreList()) {
            if (s->layoutAll()) {
                  s->_updateAll  = true;
                  s->doLayout();
                  if (s != this)
                        s->deselectAll();
                  }
            const InputState& is = s->inputState();
            if (is.noteEntryMode() && is.segment())
                  s->setPlayPos(is.segment()->tick());
            if (_playlistDirty) {
                  emit playlistChanged();
                  _playlistDirty = false;
                  }
            }

      if (MScore::debugMode)
            qDebug("===endCmd() %d", undo()->current()->childCount());
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
      for (Score* s : scoreList())
            s->end1();
      }

//---------------------------------------------------------
//   update
//    layout & update
//---------------------------------------------------------

void Score::update()
      {
      for (Score* s : scoreList()) {
            if (s->layoutAll()) {
                  s->setUpdateAll(true);
                  s->doLayout();
                  }
            if (s != this)
                  s->deselectAll();
            s->end1();
            }
      }

//---------------------------------------------------------
//   end1
//---------------------------------------------------------

void Score::end1()
      {
      if (_updateAll) {
            for (MuseScoreView* v : viewer)
                  v->updateAll();
            }
      else {
            // update a little more:
            qreal d = spatium() * .5;
            refresh.adjust(-d, -d, 2 * d, 2 * d);
            for (MuseScoreView* v : viewer)
                  v->dataChanged(refresh);
            }
      refresh    = QRectF();
      _updateAll = false;
      }

//---------------------------------------------------------
//   endUndoRedo
///   Common handling for ending undo or redo
//---------------------------------------------------------

void Score::endUndoRedo()
      {
      updateSelection();
      for (Score* score : scoreList()) {
            if (score->layoutAll()) {
                  score->setUndoRedo(true);
                  score->doLayout();
                  score->setUndoRedo(false);
                  score->setUpdateAll(true);
                  }
            const InputState& is = score->inputState();
            if (is.noteEntryMode() && is.segment())
                  score->setPlayPos(is.segment()->tick());
            if (_playlistDirty) {
                  emit playlistChanged();
                  _playlistDirty = false;
                  }
            }
      end();
      }

//---------------------------------------------------------
//   cmdAddSpanner
//   drop VOLTA, OTTAVA, TRILL, PEDAL, DYNAMIC
//        HAIRPIN, and TEXTLINE
//---------------------------------------------------------

void Score::cmdAddSpanner(Spanner* spanner, const QPointF& pos)
      {
      int staffIdx;
      Segment* segment;
      MeasureBase* mb = pos2measure(pos, &staffIdx, 0, &segment, 0);
      if (mb == 0 || mb->type() != Element::Type::MEASURE) {
            qDebug("cmdAddSpanner: cannot put object here");
            delete spanner;
            return;
            }

      // all spanners live in voice 0 (except slurs/ties)
      int track = staffIdx == -1 ? -1 : staffIdx * VOICES;

      spanner->setTrack(track);
      spanner->setTrack2(track);

      if (spanner->anchor() == Spanner::Anchor::SEGMENT) {
            spanner->setTick(segment->tick());
            int lastTick = lastMeasure()->tick() + lastMeasure()->ticks();
            int tick2 = qMin(segment->measure()->tick() + segment->measure()->ticks(), lastTick);
            spanner->setTick2(tick2);
            }
      else {      // Anchor::MEASURE
            Measure* m = static_cast<Measure*>(mb);
            QRectF b(m->canvasBoundingRect());

            if (pos.x() >= (b.x() + b.width() * .5) && m != lastMeasureMM())
                  m = m->nextMeasure();
            spanner->setTick(m->tick());
            spanner->setTick2(m->endTick());
            }

      undoAddElement(spanner);
      select(spanner, SelectType::SINGLE, 0);

      if (spanner->type() == Element::Type::TRILL) {
            Element* e = segment->element(staffIdx * VOICES);
            if (e && e->type() == Element::Type::CHORD) {
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
                  s = s->next1(Segment::Type::ChordRest);
                  while (s) {
                        Element* e = s->element(staffIdx * VOICES);
                        if (e)
                              break;
                        s = s->next1(Segment::Type::ChordRest);
                        }
                  if (s)
                        spanner->setTick2(s->tick());
                  Fraction d(1,32);
                  Fraction e = l / d;
                  int n = e.numerator() / e.denominator();
                  QList<NoteEvent*> events;
                  int pitch  = chord->upNote()->ppitch();
                  Key key    = chord->staff()->key(segment->tick());
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
      for (ps = s; ps; ps = ps->prev(Segment::Type::ChordRest)) {
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
      for (ns = s->next(Segment::Type::ChordRest); ns; ns = ns->next(Segment::Type::ChordRest)) {
            if (ns->element(track))
                  break;
            }
      ticks  = ns ? (ns->tick() - s->tick()) : (m->ticks() - s->rtick());
      if (ticks == m->ticks())
            addRest(s, track, TDuration(TDuration::DurationType::V_MEASURE), 0);
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
//   cmdAddInterval
//---------------------------------------------------------

void Score::cmdAddInterval(int val, const QList<Note*>& nl)
      {
      startCmd();
      for (Note* on : nl) {
            Note* note = new Note(*on);
            Chord* chord = on->chord();
            note->setParent(chord);
            int valTmp = val < 0 ? val+1 : val-1;

            int npitch;
            int ntpc1;
            int ntpc2;
            if (abs(valTmp) != 7) {
                  int line      = on->line() - valTmp;
                  int tick      = chord->tick();
                  Staff* estaff = staff(on->staffIdx() + chord->staffMove());
                  ClefType clef = estaff->clef(tick);
                  Key key       = estaff->key(tick);
                  npitch        = line2pitch(line, clef, key);

                  int ntpc   = pitch2tpc(npitch, key, Prefer::NEAREST);
                  Interval v = on->staff()->part()->instr()->transpose();
                  if (v.isZero())
                        ntpc1 = ntpc2 = ntpc;
                  else {
                        if (styleB(StyleIdx::concertPitch)) {
                              v.flip();
                              ntpc1 = ntpc;
                              ntpc2 = Ms::transposeTpc(ntpc, v, false);
                              }
                        else {
                              npitch += v.chromatic;
                              ntpc2 = ntpc;
                              ntpc1 = Ms::transposeTpc(ntpc, v, false);
                              }
                        }
                  }
            else { //special case for octave
                  Interval interval(7, 12);
                  if (val < 0)
                        interval.flip();
                  transposeInterval(on->pitch(), on->tpc(), &npitch, &ntpc1, interval, false);
                  ntpc1 = on->tpc1();
                  ntpc2 = on->tpc2();
                  }
            note->setPitch(npitch, ntpc1, ntpc2);

            undoAddElement(note);
            _playNote = true;

            select(note, SelectType::SINGLE, 0);
            }
      Chord* c = nl.front()->chord();
      c->measure()->cmdUpdateNotes(c->staffIdx());
      setLayoutAll(true);
      _is.moveToNextInputPos();
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

void Score::setGraceNote(Chord* ch, int pitch, NoteType type, int len)
      {
      Note* note = new Note(this);
      Chord* chord = new Chord(this);
      chord->setTrack(ch->track());
      chord->setParent(ch);
      chord->add(note);

      note->setPitch(pitch);
      note->setTpcFromPitch();

      TDuration d;
      d.setVal(len);
      chord->setDurationType(d);
      chord->setDuration(d.fraction());
      chord->setNoteType(type);
      chord->setMag(ch->staff()->mag() * styleD(StyleIdx::graceNoteMag));

      undoAddElement(chord);
      select(note, SelectType::SINGLE, 0);
      }

//---------------------------------------------------------
//   setNoteRest
//    pitch == -1  -> set rest
//    return segment of last created note/rest
//---------------------------------------------------------

Segment* Score::setNoteRest(Segment* segment, int track, NoteVal nval, Fraction sd,
   MScore::Direction stemDirection)
      {
      Q_ASSERT(segment->segmentType() == Segment::Type::ChordRest);

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

            Segment* nseg = tick2segment(tick, false, Segment::Type::ChordRest);
            if (nseg == 0) {
                  qDebug("reached end of score");
                  break;
                  }
            segment = nseg;

            cr = static_cast<ChordRest*>(segment->element(track));

            if (cr == 0) {
                  if (track % VOICES)
                        cr = addRest(segment, track, TDuration(TDuration::DurationType::V_MEASURE), 0);
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
      if (nr) {
            if (_is.slur() && nr->type() == Element::Type::NOTE) {
                  //
                  // extend slur
                  //
                  Chord* chord = static_cast<Note*>(nr)->chord();
                  _is.slur()->undoChangeProperty(P_ID::SPANNER_TICKS, chord->tick() - _is.slur()->tick());
                  for (Element* e : _is.slur()->linkList()) {
                        Slur* slur = static_cast<Slur*>(e);
                        for (Element* e : chord->linkList()) {
                              if (e->score() == slur->score() && e->track() == slur->track2()) {
                                    slur->score()->undo(new ChangeSpannerElements(slur, slur->startElement(), e));
                                    break;
                                    }
                              }
                        }
                  setLayoutAll(true);
                  }
            select(nr, SelectType::SINGLE, 0);
            }
      return segment;
      }

//---------------------------------------------------------
//   makeGap
//    make time gap at tick by removing/shortening
//    chord/rest
//
//    if keepChord, the chord at tick is not removed
//
//    gap does not exceed measure or scope of tuplet
//
//    return size of actual gap
//---------------------------------------------------------

Fraction Score::makeGap(Segment* segment, int track, const Fraction& _sd, Tuplet* tuplet, bool keepChord)
      {
      Q_ASSERT(_sd.numerator());

      Measure* measure = segment->measure();
      setLayoutAll(true);
      Fraction akkumulated;
      Fraction sd = _sd;

      //
      // remember first segment which should
      // not be deleted (it may contain other elements we want to preserve)
      //
      Segment* firstSegment = segment;
      int nextTick = segment->tick();

      for (Segment* seg = firstSegment; seg; seg = seg->next(Segment::Type::ChordRest)) {
            //
            // voices != 0 may have gaps:
            //
            ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
            if (!cr) {
                  if (seg->tick() < nextTick)
                        continue;
                  Segment* seg1 = seg->next(Segment::Type::ChordRest);
                  int tick2 = seg1 ? seg1->tick() : seg->measure()->tick() + seg->measure()->ticks();
                  Fraction td(Fraction::fromTicks(tick2 - seg->tick()));
                  segment = seg;
                  if (td > sd)
                        td = sd;
                  akkumulated += td;
                  sd -= td;
                  if (sd.isZero())
                        return akkumulated;
                  nextTick = tick2;
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
                  if (tupletEnd)
                        return akkumulated;
                  }
            Fraction td(cr->duration());

            // remove tremolo between 2 notes, if present
            if (cr->type() == Element::Type::CHORD) {
                  Chord* c = static_cast<Chord*>(cr);
                  if (c->tremolo()) {
                        Tremolo* tremolo = c->tremolo();
                        if (tremolo->twoNotes())
                              undoRemoveElement(tremolo);
                        }
                  }
            Tuplet* ltuplet = cr->tuplet();
            if (cr->tuplet() != tuplet) {
                  //
                  // Current location points to the start of a (nested)tuplet.
                  // We have to remove the complete tuplet.

                  Tuplet* t = ltuplet;
                  while (t->elements().last()->type() == Element::Type::TUPLET)
                        t = static_cast<Tuplet*>(t->elements().last());
                  seg = static_cast<ChordRest*>(t->elements().last())->segment();

                  td = ltuplet->duration();
                  cmdDeleteTuplet(ltuplet, false);
                  tuplet = 0;
                  }
            else {
                  if (seg != firstSegment || !keepChord)
                        undoRemoveElement(cr);
                  }
            nextTick += td.ticks();
            if (sd < td) {
                  //
                  // we removed too much
                  //
                  akkumulated = _sd;
                  Fraction rd = td - sd;

                  QList<TDuration> dList = toDurationList(rd, false);
                  if (dList.isEmpty())
                        return akkumulated;

                  Fraction f = sd / cr->staff()->timeStretch(cr->tick());
                  for (Tuplet* t = tuplet; t; t = t->tuplet())
                        f /= t->ratio();
                  int tick  = cr->tick() + f.ticks();

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
                  return akkumulated;
                  }
            akkumulated += td;
            sd          -= td;
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
//    make time gap for each voice
//    starting at tick+voiceOffset[voice] by removing/shortening
//    chord/rest
//    - cr is top level (not part of a tuplet)
//    - do not stop at measure end
//---------------------------------------------------------

bool Score::makeGap1(int baseTick, int staffIdx, Fraction len, int voiceOffset[VOICES])
      {
      Segment* seg = tick2segment(baseTick, true, Segment::Type::ChordRest);
      if (!seg) {
            qDebug("1:makeGap1: no segment to paste at tick %d", baseTick);
            return false;
            }
      int strack = staffIdx * VOICES;
      for (int track = strack; track < strack + VOICES; track++) {
            if (voiceOffset[track-strack] == -1)
                  continue;
            int tick = baseTick + voiceOffset[track-strack];
            Measure* m   = tick2measure(tick);
            seg = m->undoGetSegment(Segment::Type::ChordRest, tick);

            Fraction newLen = len - Fraction::fromTicks(voiceOffset[track-strack]);
            Q_ASSERT(newLen.numerator() != 0);
            bool result = makeGapVoice(seg, track, newLen, tick);
            if(track == strack && !result) // makeGap failed for first voice
                  return false;
            }
      return true;
      }

bool Score::makeGapVoice(Segment* seg, int track, Fraction len, int tick)
      {
      ChordRest* cr = 0;
      cr = static_cast<ChordRest*>(seg->element(track));
      if (!cr) {
            // check if we are in the middle of a chord/rest
            Segment* seg1 = seg->prev(Segment::Type::ChordRest);;
            for (;;) {
                  if (seg1 == 0) {
                        qDebug("1:makeGapVoice: no segment before tick %d", tick);
                        // this happens only for voices other than voice 1
                        expandVoice(seg, track);
                        return makeGapVoice(seg,track,len,tick);
                        }
                  if (seg1->element(track))
                        break;
                  seg1 = seg1->prev(Segment::Type::ChordRest);
                  }
            ChordRest* cr1 = static_cast<ChordRest*>(seg1->element(track));
            Fraction srcF = cr1->duration();
            Fraction dstF = Fraction::fromTicks(tick - cr1->tick());
            undoChangeChordRestLen(cr1, TDuration(dstF));
            setRest(tick, track, srcF - dstF, true, 0);
            for (;;) {
                  seg1 = seg1->next1(Segment::Type::ChordRest);
                  if (seg1 == 0) {
                        qDebug("2:makeGapVoice: no segment");
                        return false;
                        }
                  if (seg1->element(track)) {
                        cr = static_cast<ChordRest*>(seg1->element(track));
                        break;
                        }
                  }
            }

      for (;;) {
            if (!cr) {
                  qDebug("3:makeGapVoice: cannot make gap");
                  return false;
                  }
            Fraction l = makeGap(cr->segment(), cr->track(), len, 0);
            if (l.isZero()) {
                  qDebug("4:makeGapVoice: makeGap returns zero gap");
                  return false;
                  }
            len -= l;
            if (len.isZero())
                  break;
            // go to next cr
            Measure* m = cr->measure()->nextMeasure();
            if (m == 0) {
                  qDebug("EOS reached");
                  insertMeasure(Element::Type::MEASURE, 0, false);
                  m = cr->measure()->nextMeasure();
                  if (m == 0) {
                        qDebug("===EOS reached");
                        return true;
                        }
                  }
            // first segment in measure was removed, have to recreate it
            Segment* s = m->undoGetSegment(Segment::Type::ChordRest, m->tick());
            int track  = cr->track();
            cr = static_cast<ChordRest*>(s->element(track));
            if (cr == 0) {
                  addRest(s, track, TDuration(TDuration::DurationType::V_MEASURE), 0);
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
            if(tuplet->tuplet())
                  return flist; // do no deal with nested tuplets
            Fraction rest = Fraction::fromTicks(tuplet->tick() + tuplet->duration().ticks() - cr->segment()->tick()) * tuplet->ratio();
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
            s = m->first(Segment::Type::ChordRest);
            }
      return flist;
      }

//---------------------------------------------------------
//   changeCRlen
//---------------------------------------------------------

void Score::changeCRlen(ChordRest* cr, const TDuration& d)
      {
      Fraction srcF(cr->duration());
      Fraction dstF;
      if (d.type() == TDuration::DurationType::V_MEASURE)
            dstF = cr->measure()->stretchedLen(cr->staff());
      else
            dstF = d.fraction();

      if (srcF == dstF)
            return;

qDebug("changeCRlen: %d/%d -> %d/%d", srcF.numerator(), srcF.denominator(), dstF.numerator(), dstF.denominator());

      //keep selected element if any
      Element* selElement = selection().isSingle() ? getSelectedElement() : 0;

      int track = cr->track();
      Tuplet* tuplet = cr->tuplet();

      if (srcF > dstF) {
            //
            // make shorter and fill with rest
            //
            deselectAll();
            if (cr->type() == Element::Type::CHORD) {
                  //
                  // remove ties and tremolo between 2 notes
                  //
                  Chord* c = static_cast<Chord*>(cr);
                  if (c->tremolo()) {
                        Tremolo* tremolo = c->tremolo();
                        if (tremolo->twoNotes())
                              undoRemoveElement(tremolo);
                        }
                  foreach (Note* n, c->notes()) {
                        if (n->tieFor())
                              undoRemoveElement(n->tieFor());
                        }
                  }
            undoChangeChordRestLen(cr, TDuration(dstF));
qDebug("  setRest at %d+%d, %d/%d", cr->tick(), cr->actualTicks(), (srcF-dstF).numerator(), (srcF-dstF).denominator());
            setRest(cr->tick() + cr->actualTicks(), track, srcF - dstF, false, tuplet);

            if (selElement)
                  select(selElement, SelectType::SINGLE, 0);
            return;
            }

      //
      // make longer
      //
      // split required len into Measures
      QList<Fraction> flist = splitGapToMeasureBoundaries(cr, dstF);
      if (flist.isEmpty())
            return;

      deselectAll();
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
            makeGap(cr1->segment(), cr1->track(), f2, tuplet, first);

            if (cr->type() == Element::Type::REST) {
qDebug("  +ChangeCRLen::setRest %d/%d", f2.numerator(), f2.denominator());
                  Fraction timeStretch = cr1->staff()->timeStretch(cr1->tick());
                  Rest* r = static_cast<Rest*>(cr);
                  if (first) {
                        QList<TDuration> dList = toDurationList(f2, true);
                        undoChangeChordRestLen(cr, dList[0]);
                        if(dList.size() > 1) {
                              TDuration remain = TDuration(f2) - dList[0];
                              setRest(tick +dList[0].ticks(), track, remain.fraction() * timeStretch, (remain.dots() > 0), tuplet);
                              }
                        }
                  else {
                        r = setRest(tick, track, f2 * timeStretch, (d.dots() > 0), tuplet);
                        }
                  if (first) {
                        select(r, SelectType::SINGLE, 0);
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
                        foreach(TDuration du, dList) {
                              bool genTie;
                              Chord* cc;
                              if (oc) {
                                    genTie = true;
                                    cc = oc;
                                    oc = addChord(tick, du, cc, genTie, tuplet);
                                    }
                              else {
                                    genTie = false;
                                    cc = static_cast<Chord*>(cr);
                                    undoChangeChordRestLen(cr, du);
                                    oc = cc;
                                    }
                              if (oc && first) {
                                    if (!selElement)
                                          select(oc, SelectType::SINGLE, 0);
                                    else
                                          select(selElement, SelectType::SINGLE, 0);
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
                                    oc = addChord(tick, dList[i], cc, genTie, tuplet);
                                    }
                              else {
                                    genTie = false;
                                    cc = static_cast<Chord*>(cr);
                                    undoChangeChordRestLen(cr, dList[i]);
                                    oc = cc;
                                    }
                              if (first) {
                                    select(oc, SelectType::SINGLE, 0);
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
            Segment* s = m1->first(Segment::Type::ChordRest);
            expandVoice(s, track);
            cr1 = static_cast<ChordRest*>(s->element(track));
            }
//      checkSpanner(cr->tick(), cr->tick() + d.ticks());
      connectTies();
      }

//---------------------------------------------------------
//   upDownChromatic
//---------------------------------------------------------

static void upDownChromatic(bool up, int pitch, Note* n, Key key, int tpc1, int tpc2, int& newPitch, int& newTpc1, int& newTpc2)
      {
      if (up && pitch < 127) {
            newPitch = pitch + 1;
            if (n->concertPitch()) {
                  if (tpc1 > Tpc::TPC_A + int(key))
                        newTpc1 = tpc1 - 5;   // up semitone diatonic
                  else
                        newTpc1 = tpc1 + 7;   // up semitone chromatic
                  newTpc2 = n->transposeTpc(newTpc1);
                  }
            else {
                  if (tpc2 > Tpc::TPC_A + int(key))
                        newTpc2 = tpc2 - 5;   // up semitone diatonic
                  else
                        newTpc2 = tpc2 + 7;   // up semitone chromatic
                  newTpc1 = n->transposeTpc(newTpc2);
                  }
            }
      else if (!up && pitch > 0) {
            newPitch = pitch - 1;
            if (n->concertPitch()) {
                  if (tpc1 > Tpc::TPC_C + int(key))
                        newTpc1 = tpc1 - 7;   // down semitone chromatic
                  else
                        newTpc1 = tpc1 + 5;   // down semitone diatonic
                  newTpc2 = n->transposeTpc(newTpc1);
                  }
            else {
                  if (tpc2 > Tpc::TPC_C + int(key))
                        newTpc2 = tpc2 - 7;   // down semitone chromatic
                  else
                        newTpc2 = tpc2 + 5;   // down semitone diatonic
                  newTpc1 = n->transposeTpc(newTpc2);
                  }
            }
      }

//---------------------------------------------------------
//   setTpc
//---------------------------------------------------------

static void setTpc(Note* oNote, int tpc, int& newTpc1, int& newTpc2)
      {
      if (oNote->concertPitch()) {
            newTpc1 = tpc;
            newTpc2 = oNote->transposeTpc(tpc);
            }
      else {
            newTpc2 = tpc;
            newTpc1 = oNote->transposeTpc(tpc);
            }
      }

//---------------------------------------------------------
//   upDown
///   Increment/decrement pitch of note by one or by an octave.
//---------------------------------------------------------

void Score::upDown(bool up, UpDownMode mode)
      {
      QList<Note*> el = selection().uniqueNotes();
      if (el.empty())
            return;

      foreach (Note* oNote, el) {
            int tick     = oNote->chord()->tick();
            Part* part   = oNote->staff()->part();
            Key key      = oNote->staff()->key(tick);
            int tpc1     = oNote->tpc1();
            int tpc2     = oNote->tpc2();
            int pitch    = oNote->pitch();
            int newTpc1  = tpc1;      // default to unchanged
            int newTpc2  = tpc2;      // default to unchanged
            int newPitch = pitch;     // default to unchanged
            int string   = oNote->string();
            int fret     = oNote->fret();

            switch (oNote->staff()->staffType()->group()) {
                  case StaffGroup::PERCUSSION:
                        {
                        Drumset* ds = part->instr()->drumset();
                        if (ds)
                              newPitch = up ? ds->prevPitch(pitch) : ds->nextPitch(pitch);
                        }
                        break;
                  case StaffGroup::TAB:
                        {
                        const StringData* stringData = part->instr()->stringData();
                        switch (mode) {
                              case UpDownMode::OCTAVE:          // move same note to next string, if possible
                                    {
                                    StaffType* stt = oNote->staff()->staffType();
                                    string = stt->physStringToVisual(string);
                                    string += (up ? -1 : 1);
                                    if (string < 0 || string >= stringData->strings())
                                          return;           // no next string to move to
                                    string = stt->visualStringToPhys(string);
                                    fret = stringData->fret(pitch, string);
                                    if (fret == -1)          // can't have that note on that string
                                          return;
                                    // newPitch and newTpc remain unchanged
                                    }
                                    break;

                              case UpDownMode::DIATONIC:        // increase / decrease the pitch,
                                                            // letting the algorithm to choose fret & string
                                    upDownChromatic(up, pitch, oNote, key, tpc1, tpc2, newPitch, newTpc1, newTpc2);
                                    break;

                              case UpDownMode::CHROMATIC:       // increase / decrease the fret
                                    {                       // without changing the string
                                    if (!stringData->frets())
                                          qDebug("upDown tab chromatic: no frets?");
                                    fret += (up ? 1 : -1);
                                    if (fret < 0)
                                          fret = 0;
                                    else if (fret >= stringData->frets())
                                          fret = stringData->frets() - 1;
                                    newPitch    = stringData->getPitch(string, fret);
                                    if (newPitch == -1) {
                                          qDebug("upDown tab chromatic: getPitch(%d,%d) returns -1", string, fret);
                                          newPitch = oNote->pitch();
                                          }
                                    // TAB's are by definition non-transposing
                                    newTpc1 = newTpc2 = pitch2tpc(newPitch, key, up ? Prefer::SHARPS : Prefer::FLATS);
                                    // store the fretting change before undoChangePitch() chooses
                                    // a fretting of its own liking!
                                    undoChangeProperty(oNote, P_ID::FRET, fret);
                                    undoChangeProperty(oNote, P_ID::STRING, string);
                                    }
                                    break;
                              }
                        }
                        break;
                  case StaffGroup::STANDARD:
                        switch(mode) {
                              case UpDownMode::OCTAVE:
                                    if (up) {
                                          if (pitch < 116)
                                                newPitch = pitch + 12;
                                          }
                                    else {
                                          if (pitch > 11)
                                                newPitch = pitch - 12;
                                          }
                                    // newTpc remains unchanged
                                    break;

                              case UpDownMode::CHROMATIC:
                                    upDownChromatic(up, pitch, oNote, key, tpc1, tpc2, newPitch, newTpc1, newTpc2);
                                    break;

                              case UpDownMode::DIATONIC:
                                    {
                                    int tpc = oNote->tpc();
                                    if (up) {
                                          if (tpc > Tpc::TPC_A + int(key)) {
                                                if (pitch < 127) {
                                                      newPitch = pitch + 1;
                                                      setTpc(oNote, tpc - 5, newTpc1, newTpc2);
                                                      }
                                                }
                                          else {
                                                if (pitch < 126) {
                                                      newPitch = pitch + 2;
                                                      setTpc(oNote, tpc + 2, newTpc1, newTpc2);
                                                      }
                                                }
                                          }
                                    else {
                                          if (tpc > Tpc::TPC_C + int(key)) {
                                                if (pitch > 1) {
                                                      newPitch = pitch - 2;
                                                      setTpc(oNote, tpc - 2, newTpc1, newTpc2);
                                                      }
                                                }
                                          else {
                                                if (pitch > 0) {
                                                      newPitch = pitch - 1;
                                                      setTpc(oNote, tpc + 5, newTpc1, newTpc2);
                                                      }
                                                }
                                          }
                                    }
                                    break;
                              }
                        break;
                  }

            if ((oNote->pitch() != newPitch) || (oNote->tpc1() != newTpc1) || oNote->tpc2() != newTpc2) {
                  // remove accidental if present to make sure
                  // user added accidentals are removed here.
                  if (oNote->links()) {
                        for (Element* e : *oNote->links()) {
                              Note* ln = static_cast<Note*>(e);
                              if (ln->accidental())
                                    undoRemoveElement(ln->accidental());
                              }
                        }
                  else if (oNote->accidental())
                        undoRemoveElement(oNote->accidental());
                  undoChangePitch(oNote, newPitch, newTpc1, newTpc2);
                  }

            // store fret change only if undoChangePitch has not been called,
            // as undoChangePitch() already manages fret changes, if necessary
            else if (oNote->staff()->staffType()->group() == StaffGroup::TAB) {
                  bool refret = false;
                  if (oNote->string() != string) {
                        undoChangeProperty(oNote, P_ID::STRING, string);
                        refret = true;
                        }
                  if (oNote->fret() != fret) {
                        undoChangeProperty(oNote, P_ID::FRET, fret);
                        refret = true;
                        }
                  if (refret) {
                        const StringData* stringData = part->instr()->stringData();
                        stringData->fretChords(oNote->chord());
                        }
                  }

            // play new note with velocity 80 for 0.3 sec:
            _playNote = true;
            }

      _selection.clear();
      for (Note* note : el)
            _selection.add(note);
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
            if (el->type() == Element::Type::NOTE || el->type() == Element::Type::CHORD) {
                  Articulation* na = new Articulation(this);
                  na->setArticulationType(attr);
                  if (!addArticulation(el, na))
                        delete na;
                  }
            }
      }

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \a idx for all selected
///   notes.
//---------------------------------------------------------

void Score::changeAccidental(Accidental::Type idx)
      {
      foreach(Note* note, selection().noteList())
            changeAccidental(note, idx);
      }

//---------------------------------------------------------
//   changeAccidental2
//---------------------------------------------------------

static void changeAccidental2(Note* n, int pitch, int tpc)
      {
      Score* score  = n->score();
      Chord* chord  = n->chord();
      int staffIdx  = chord->staffIdx();
      Staff* st     = chord->staff();
      int fret      = n->fret();
      int string    = n->string();

      if (st->isTabStaff()) {
            if (pitch != n->pitch()) {
                  //
                  // as pitch has changed, calculate new
                  // string & fret
                  //
                  const StringData* stringData = n->staff()->part()->instr()->stringData();
                  if (stringData)
                        stringData->convertPitch(pitch, &string, &fret);
                  }
            }
      int tpc1;
      int tpc2 = n->transposeTpc(tpc);
      if (score->styleB(StyleIdx::concertPitch))
            tpc1 = tpc;
      else {
            tpc1 = tpc2;
            tpc2 = tpc;
            }
      score->undoChangePitch(n, pitch, tpc1, tpc2);
      if (!st->isTabStaff()) {
            //
            // handle ties
            //
            if (n->tieBack()) {
                  score->undoRemoveElement(n->tieBack());
                  if (n->tieFor())
                        score->undoRemoveElement(n->tieFor());
                  }
            else {
                  Note* nn = n;
                  while (nn->tieFor()) {
                        nn = nn->tieFor()->endNote();
                        score->undo(new ChangePitch(nn, pitch, tpc1, tpc2));
                        }
                  }
            }
      //
      // recalculate needed accidentals for
      // whole measure
      //
      chord->measure()->cmdUpdateNotes(staffIdx);
      }

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \accidental for
///   note \a note.
//---------------------------------------------------------

void Score::changeAccidental(Note* note, Accidental::Type accidental)
      {
      Chord* chord     = note->chord();
      if (!chord)
            return;
      Segment* segment = chord->segment();
      if (!segment)
            return;
      Measure* measure = segment->measure();
      if (!measure)
            return;
      int tick         = segment->tick();
      Staff* estaff    = staff(chord->staffIdx() + chord->staffMove());
      if (!estaff)
            return;
      ClefType clef    = estaff->clef(tick);
      int step         = ClefInfo::pitchOffset(clef) - note->line();
      while (step < 0)
            step += 7;
      step %= 7;
      //
      // accidental change may result in pitch change
      //
      AccidentalVal acc2 = measure->findAccidental(note);
      AccidentalVal acc = (accidental == Accidental::Type::NONE) ? acc2 : Accidental::subtype2value(accidental);

      int pitch = line2pitch(note->line(), clef, Key::C) + int(acc);
      if (!note->concertPitch())
            pitch += note->transposition();

      int tpc = step2tpc(step, acc);

      bool forceRemove = false;
      bool forceAdd = false;

      // delete accidental
      // both for this note and for any linked notes
      if (accidental == Accidental::Type::NONE)
            forceRemove = true;

      // precautionary or microtonal accidental
      // either way, we display it unconditionally
      // both for this note and for any linked notes
      else if (acc == acc2 || accidental > Accidental::Type::NATURAL)
            forceAdd = true;

      if (note->links()) {
            for (Element* e : *note->links()) {
                  Note* ln = static_cast<Note*>(e);
                  if (ln->concertPitch() != note->concertPitch())
                        continue;
                  Score* lns = ln->score();
                  if (forceRemove) {
                        Accidental* a = ln->accidental();
                        if (a)
                              lns->undoRemoveElement(a);
                        }
                  else if (forceAdd) {
                        Accidental* a = new Accidental(lns);
                        a->setParent(ln);
                        a->setAccidentalType(accidental);
                        a->setRole(Accidental::Role::USER);
                        lns->undoAddElement(a);
                        }
                  changeAccidental2(ln, pitch, tpc);
                  }
            }

      else {
            if (forceRemove) {
                  Accidental* a = note->accidental();
                  if (a)
                        undoRemoveElement(a);
                  }
            else if (forceAdd) {
                  Accidental* a = new Accidental(this);
                  a->setParent(note);
                  a->setAccidentalType(accidental);
                  a->setRole(Accidental::Role::USER);
                  undoAddElement(a);
                  }
            changeAccidental2(note, pitch, tpc);
            }

      }

//---------------------------------------------------------
//   addArticulation
//---------------------------------------------------------

bool Score::addArticulation(Element* el, Articulation* a)
      {
      ChordRest* cr;
      if (el->type() == Element::Type::NOTE)
            cr = static_cast<ChordRest*>(static_cast<Note*>(el)->chord());
      else if (el->type() == Element::Type::REST
         || el->type() == Element::Type::CHORD
         || el->type() == Element::Type::REPEAT_MEASURE)
            cr = static_cast<ChordRest*>(el);
      else
            return false;
      Articulation* oa = cr->hasArticulation(a);
      if (oa) {
            undoRemoveElement(oa);
            return false;
            }
      a->setParent(cr);
      undoAddElement(a);
      return true;
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
            undo(new ChangeStretch(m, 1.0));
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
      Staff* staff  = chord->staff();
      Part* part    = staff->part();
      int rstaff    = staff->rstaff();
      int staffMove = chord->staffMove();

      if ((staffMove == -1) || (rstaff + staffMove <= 0))
            return;

      QList<Staff*>* staves = part->staves();
      // we know that staffMove+rstaff-1 index exists due to the previous condition.
      if (staff->staffType()->group() != StaffGroup::STANDARD ||
          staves->at(rstaff+staffMove-1)->staffType()->group() != StaffGroup::STANDARD) {
            qDebug("User attempted to move a note from/to a staff which does not use standard notation - ignoring.");
            }
      else  {
            // move the chord up a staff
            undo(new ChangeChordStaffMove(chord, staffMove - 1));
            }
      }
//---------------------------------------------------------
//   moveDown
//---------------------------------------------------------

void Score::moveDown(Chord* chord)
      {
      Staff* staff  = chord->staff();
      Part* part    = staff->part();
      int rstaff    = staff->rstaff();
      int staffMove = chord->staffMove();
      // calculate the number of staves available so that we know whether there is another staff to move down to
      int rstaves   = part->nstaves();

      if ((staffMove == 1) || (rstaff + staffMove >= rstaves - 1)) {
            qDebug("moveDown staffMove==%d  rstaff %d rstaves %d", staffMove, rstaff, rstaves);
            return;
            }

      QList<Staff*>* staves = part->staves();
      // we know that staffMove+rstaff+1 index exists due to the previous condition.
      if (staff->staffType()->group() != StaffGroup::STANDARD ||
          staves->at(staffMove+rstaff+1)->staffType()->group() != StaffGroup::STANDARD) {
            qDebug("User attempted to move a note from/to a staff which does not use standard notation - ignoring.");
            }
      else  {
            // move the chord down a staff
            undo(new ChangeChordStaffMove(chord, staffMove + 1));
            }
      }

//---------------------------------------------------------
//   cmdAddStretch
//---------------------------------------------------------

void Score::cmdAddStretch(qreal val)
      {
      if (!selection().isRange())
            return;
      int startTick = selection().tickStart();
      int endTick   = selection().tickEnd();
      for (Measure* m = firstMeasureMM(); m; m = m->nextMeasureMM()) {
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
//   cmdResetBeamMode
//---------------------------------------------------------

void Score::cmdResetBeamMode()
      {
      if (!selection().isRange()) {
            qDebug("no system or staff selected");
            return;
            }
      int startTick = selection().tickStart();
      int endTick   = selection().tickEnd();

      Segment::Type st = Segment::Type::ChordRest;
      for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            if (seg->tick() < startTick)
                  continue;
            if (seg->tick() >= endTick)
                  break;
            for (int track = 0; track < nstaves() * VOICES; ++track) {
                  ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
                  if (cr == 0)
                        continue;
                  if (cr->type() == Element::Type::CHORD) {
                        if (cr->beamMode() != Beam::Mode::AUTO)
                              undoChangeProperty(cr, P_ID::BEAM_MODE, int(Beam::Mode::AUTO));
                        }
                  else if (cr->type() == Element::Type::REST) {
                        if (cr->beamMode() != Beam::Mode::NONE)
                              undoChangeProperty(cr, P_ID::BEAM_MODE, int(Beam::Mode::NONE));
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
                  if (p) {
                        if (!styleB(StyleIdx::concertPitch)) {
                              ev.pitch += p->instr(selection().tickStart())->transpose().chromatic;
                        }
                        MScore::seq->startNote(
                                          p->instr()->channel(0).channel,
                                          ev.pitch,
                                          80,
                                          MScore::defaultPlayDuration,
                                          0.0);
                        }
                  }
            else  {
                  if (!cmdActive) {
                        startCmd();
                        cmdActive = true;
                        }
                  NoteVal nval(ev.pitch);
                  Staff* st = staff(inputState().track() / VOICES);
                  Key key = st->key(inputState().tick());
                  nval.tpc = pitch2tpc(nval.pitch, key, Prefer::NEAREST);
                  if (!styleB(StyleIdx::concertPitch)) {
                      nval.pitch += st->part()->instr(inputState().tick())->transpose().chromatic;
                      }
                  addPitch(nval, ev.chord);
                  }
            }
      if (cmdActive) {
            _layoutAll = true;
            endCmd();
            //after relayout
            Element* e = inputState().cr();
            if (e) {
                  for(MuseScoreView* v : viewer)
                        v->adjustCanvasPosition(e, false);
                  }
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   move
//    move current selection
//---------------------------------------------------------

Element* Score::move(const QString& cmd)
      {
      ChordRest* cr;
      if (noteEntryMode())
            cr = inputState().cr();
      else if (selection().activeCR())
            cr = selection().activeCR();
      else
            cr = selection().lastChordRest();

      // no chord/rest found? look for another type of element
      if (cr == 0) {
            if (selection().elements().isEmpty())
                  return 0;
            // retrieve last element of section list
            Element* el = selection().elements().last();
            Element* trg = 0;

            // get parent of element and process accordingly:
            // trg is the element to select on "next-chord" cmd
            // cr is the ChordRest to move from on other cmd's
            int track = el->track();            // keep note of element track
            el = el->parent();
            switch (el->type()) {
                  case Element::Type::NOTE:           // a note is a valid target
                        trg = el;
                        cr  = static_cast<Note*>(el)->chord();
                        break;
                  case Element::Type::CHORD:          // a chord or a rest are valid targets
                  case Element::Type::REST:
                        trg = el;
                        cr  = static_cast<ChordRest*>(trg);
                        break;
                  case Element::Type::SEGMENT: {      // from segment go to top chordrest in segment
                        Segment* seg  = static_cast<Segment*>(el);
                        // if segment is not chord/rest or grace, move to next chord/rest or grace segment
                        if (!seg->isChordRest()) {
                              seg = seg->next1(Segment::Type::ChordRest);
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
                  if (trg->type() == Element::Type::CHORD)
                        trg = static_cast<Chord*>(trg)->upNote();
                  _playNote = true;
                  select(trg, SelectType::SINGLE, 0);
                  return trg;
                  }
            // if no chordrest found, do nothing
            if (cr == 0)
                  return 0;
            // if some chordrest found, continue with default processing
            }

      Element* el = 0;
      if (cmd == "next-chord") {
            if (noteEntryMode())
                  _is.moveToNextInputPos();
            el = nextChordRest(cr);
            }
      else if (cmd == "prev-chord") {
            if (noteEntryMode() && _is.segment()) {
                  Segment* s = _is.segment()->prev1();
                  //
                  // if _is._segment is first chord/rest segment in measure
                  // make sure "m" points to previous measure
                  //
                  while (s && s->segmentType() != Segment::Type::ChordRest)
                        s = s->prev1();
                  if (s == 0)
                        return 0;
                  Measure* m = s->measure();

                  int track  = _is.track();
                  for (; s; s = s->prev1()) {
                        if (s->segmentType() != Segment::Type::ChordRest)
                              continue;
                        if (s->element(track) || s->measure() != m)
                              break;
                        }
                  if (s && !s->element(track))
                        s = m->first(Segment::Type::ChordRest);
                  _is.moveInputPos(s);
                  }
            el = prevChordRest(cr);
            }
      else if (cmd == "next-measure") {
            el = nextMeasure(cr);
            if (noteEntryMode())
                  _is.moveInputPos(el);
            }
      else if (cmd == "prev-measure") {
            el = prevMeasure(cr);
            if (noteEntryMode())
                  _is.moveInputPos(el);
            }
      else if (cmd == "next-track") {
            el = nextTrack(cr);
            if (noteEntryMode())
                  _is.moveInputPos(el);
            }
      else if (cmd == "prev-track") {
            el = prevTrack(cr);
            if (noteEntryMode())
                  _is.moveInputPos(el);
            }
      if (el) {
            if (el->type() == Element::Type::CHORD)
                  el = static_cast<Chord*>(el)->upNote();       // originally downNote
            _playNote = true;
            select(el, SelectType::SINGLE, 0);
            if (noteEntryMode()) {
                  foreach (MuseScoreView* view ,viewer)
                        view->moveCursor();
                  }
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
      if (cr == 0 && noteEntryMode())
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
            select(el, SelectType::RANGE, el->staffIdx());
      return el;
      }

//---------------------------------------------------------
//   cmdMirrorNoteHead
//---------------------------------------------------------

void Score::cmdMirrorNoteHead()
      {
      const QList<Element*>& el = selection().elements();
      foreach(Element* e, el) {
            if (e->type() == Element::Type::NOTE) {
                  Note* note = static_cast<Note*>(e);
                  if (note->staff() && note->staff()->isTabStaff())
                        note->score()->undoChangeProperty(e, P_ID::GHOST, true);
                  else {
                        MScore::DirectionH d = note->userMirror();
                        if (d == MScore::DirectionH::AUTO)
                              d = note->chord()->up() ? MScore::DirectionH::RIGHT : MScore::DirectionH::LEFT;
                        else
                              d = d == MScore::DirectionH::LEFT ? MScore::DirectionH::RIGHT : MScore::DirectionH::LEFT;
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
      if (el->type() == Element::Type::NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = static_cast<ChordRest*>(el);
      TDuration d = _is.duration().shift(1);
      if (!d.isValid() || (d.type() > TDuration::DurationType::V_64TH))
            return;
      if (cr->type() == Element::Type::CHORD && (static_cast<Chord*>(cr)->noteType() != NoteType::NORMAL)) {
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
      if (el->type() == Element::Type::NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = static_cast<ChordRest*>(el);
      TDuration d = _is.duration().shift(-1);
      if (!d.isValid() || (d.type() < TDuration::DurationType::V_WHOLE))
            return;
      if (cr->type() == Element::Type::CHORD && (static_cast<Chord*>(cr)->noteType() != NoteType::NORMAL)) {
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
//   cmdAddBracket
//---------------------------------------------------------

void Score::cmdAddBracket()
      {
      for(Element* el : selection().elements()) {
            if (el->type() == Element::Type::NOTE) {
                  Note* n = static_cast<Note*>(el);
                  n->addBracket();
                  }
            else if (el->type() == Element::Type::ACCIDENTAL) {
                  Accidental* acc = static_cast<Accidental*>(el);
                  acc->undoSetHasBracket(true);
                  }
            else if (el->type() == Element::Type::HARMONY) {
                  Harmony* h = static_cast<Harmony*>(el);
                  h->setLeftParen(true);
                  h->setRightParen(true);
                  h->render();
                  }
            }
      }


//---------------------------------------------------------
//   cmdMoveRest
//---------------------------------------------------------

void Score::cmdMoveRest(Rest* rest, MScore::Direction dir)
      {
      QPointF pos(rest->userOff());
      if (dir == MScore::Direction::UP)
            pos.ry() -= spatium();
      else if (dir == MScore::Direction::DOWN)
            pos.ry() += spatium();
      undoChangeProperty(rest, P_ID::USER_OFF, pos);
      setLayoutAll(rest->beam() != nullptr);    // layout all if rest is beamed
      }

//---------------------------------------------------------
//   cmdMoveLyrics
//---------------------------------------------------------

void Score::cmdMoveLyrics(Lyrics* lyrics, MScore::Direction dir)
      {
      ChordRest* cr      = lyrics->chordRest();
      QList<Lyrics*>& ll = cr->lyricsList();
      int no             = lyrics->no();
      if (dir == MScore::Direction::UP) {
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
            if (el && (el->type() == Element::Type::ARTICULATION || el->isText()))
                  el->undoChangeProperty(P_ID::USER_OFF, el->userOff() + QPointF(0.0, -MScore::nudgeStep * el->spatium()));
            else if (el && el->type() == Element::Type::REST)
                  cmdMoveRest(static_cast<Rest*>(el), MScore::Direction::UP);
            else if (el && el->type() == Element::Type::LYRICS)
                  cmdMoveLyrics(static_cast<Lyrics*>(el), MScore::Direction::UP);
            else
                  upDown(true, UpDownMode::CHROMATIC);
            }
      else if (cmd == "pitch-down") {
            if (el && (el->type() == Element::Type::ARTICULATION || el->isText()))
                  el->undoChangeProperty(P_ID::USER_OFF, el->userOff() + QPointF(0.0, MScore::nudgeStep * el->spatium()));
            else if (el && el->type() == Element::Type::REST)
                  cmdMoveRest(static_cast<Rest*>(el), MScore::Direction::DOWN);
            else if (el && el->type() == Element::Type::LYRICS)
                  cmdMoveLyrics(static_cast<Lyrics*>(el), MScore::Direction::DOWN);
            else
                  upDown(false, UpDownMode::CHROMATIC);
            }
      else if (cmd == "add-staccato")
            addArticulation(ArticulationType::Staccato);
      else if (cmd == "add-tenuto")
            addArticulation(ArticulationType::Tenuto);
      else if (cmd == "add-marcato")
            addArticulation(ArticulationType::Marcato);
      else if (cmd == "add-trill")
            addArticulation(ArticulationType::Trill);
      else if (cmd == "add-8va")
            cmdAddOttava(Ottava::Type::OTTAVA_8VA);
      else if (cmd == "add-8vb")
            cmdAddOttava(Ottava::Type::OTTAVA_8VB);
      else if (cmd == "delete-measures")
            cmdDeleteSelectedMeasures();
      else if (cmd == "time-delete") {
            // TODO:
            // remove measures if stave-range is 0-nstaves()
            cmdDeleteSelectedMeasures();
            }
      else if (cmd == "pitch-up-octave") {
            if (el && (el->type() == Element::Type::ARTICULATION || el->isText()))
                  el->undoChangeProperty(P_ID::USER_OFF, el->userOff() + QPointF(0.0, -MScore::nudgeStep10 * el->spatium()));
            else
                  upDown(true, UpDownMode::OCTAVE);
            }
      else if (cmd == "pitch-down-octave") {
            if (el && (el->type() == Element::Type::ARTICULATION || el->isText()))
                  el->undoChangeProperty(P_ID::USER_OFF, el->userOff() + QPointF(0.0, MScore::nudgeStep10 * el->spatium()));
            else
                  upDown(false, UpDownMode::OCTAVE);
            }
      else if (cmd == "note-longa"   || cmd == "note-longa-TAB")
            padToggle(Pad::NOTE00);
      else if (cmd == "note-breve"   || cmd == "note-breve-TAB")
            padToggle(Pad::NOTE0);
      else if (cmd == "pad-note-1"   || cmd == "pad-note-1-TAB")
            padToggle(Pad::NOTE1);
      else if (cmd == "pad-note-2"   || cmd == "pad-note-2-TAB")
            padToggle(Pad::NOTE2);
      else if (cmd == "pad-note-4"   || cmd == "pad-note-4-TAB")
            padToggle(Pad::NOTE4);
      else if (cmd == "pad-note-8"   || cmd == "pad-note-8-TAB")
            padToggle(Pad::NOTE8);
      else if (cmd == "pad-note-16"  || cmd == "pad-note-16-TAB")
            padToggle(Pad::NOTE16);
      else if (cmd == "pad-note-32"  || cmd == "pad-note-32-TAB")
            padToggle(Pad::NOTE32);
      else if (cmd == "pad-note-64"  || cmd == "pad-note-64-TAB")
            padToggle(Pad::NOTE64);
      else if (cmd == "pad-note-128" || cmd == "pad-note-128-TAB")
            padToggle(Pad::NOTE128);
      else if (cmd == "pad-note-increase-TAB") {
            switch (_is.duration().type() ) {
// cycle back from longest to shortest?
//                  case TDuration::V_LONG:
//                        padToggle(Pad::NOTE128);
//                        break;
                  case TDuration::DurationType::V_BREVE:
                        padToggle(Pad::NOTE00);
                        break;
                  case TDuration::DurationType::V_WHOLE:
                        padToggle(Pad::NOTE0);
                        break;
                  case TDuration::DurationType::V_HALF:
                        padToggle(Pad::NOTE1);
                        break;
                  case TDuration::DurationType::V_QUARTER:
                        padToggle(Pad::NOTE2);
                        break;
                  case TDuration::DurationType::V_EIGHTH:
                        padToggle(Pad::NOTE4);
                        break;
                  case TDuration::DurationType::V_16TH:
                        padToggle(Pad::NOTE8);
                        break;
                  case TDuration::DurationType::V_32ND:
                        padToggle(Pad::NOTE16);
                        break;
                  case TDuration::DurationType::V_64TH:
                        padToggle(Pad::NOTE32);
                        break;
                  case TDuration::DurationType::V_128TH:
                        padToggle(Pad::NOTE64);
                        break;
                  default:
                        break;
                  }
            }
      else if (cmd == "pad-note-decrease-TAB") {
            switch (_is.duration().type() ) {
                  case TDuration::DurationType::V_LONG:
                        padToggle(Pad::NOTE0);
                        break;
                  case TDuration::DurationType::V_BREVE:
                        padToggle(Pad::NOTE1);
                        break;
                  case TDuration::DurationType::V_WHOLE:
                        padToggle(Pad::NOTE2);
                        break;
                  case TDuration::DurationType::V_HALF:
                        padToggle(Pad::NOTE4);
                        break;
                  case TDuration::DurationType::V_QUARTER:
                        padToggle(Pad::NOTE8);
                        break;
                  case TDuration::DurationType::V_EIGHTH:
                        padToggle(Pad::NOTE16);
                        break;
                  case TDuration::DurationType::V_16TH:
                        padToggle(Pad::NOTE32);
                        break;
                  case TDuration::DurationType::V_32ND:
                        padToggle(Pad::NOTE64);
                        break;
                  case TDuration::DurationType::V_64TH:
                        padToggle(Pad::NOTE128);
                        break;
// cycle back from shortest to longest?
//                  case TDuration::DurationType::V_128TH:
//                        padToggle(Pad::NOTE00);
//                        break;
                  default:
                        break;
                  }
            }
      else if (cmd == "pad-rest")
            padToggle(Pad::REST);
      else if (cmd == "pad-dot")
            padToggle(Pad::DOT);
      else if (cmd == "pad-dotdot")
            padToggle(Pad::DOTDOT);
      else if (cmd == "beam-start")
            cmdSetBeamMode(Beam::Mode::BEGIN);
      else if (cmd == "beam-mid")
            cmdSetBeamMode(Beam::Mode::MID);
      else if (cmd == "no-beam")
            cmdSetBeamMode(Beam::Mode::NONE);
      else if (cmd == "beam-32")
            cmdSetBeamMode(Beam::Mode::BEGIN32);
      else if (cmd == "sharp2")
            changeAccidental(Accidental::Type::SHARP2);
      else if (cmd == "sharp")
            changeAccidental(Accidental::Type::SHARP);
      else if (cmd == "nat")
            changeAccidental(Accidental::Type::NATURAL);
      else if (cmd == "flat")
            changeAccidental(Accidental::Type::FLAT);
      else if (cmd == "flat2")
            changeAccidental(Accidental::Type::FLAT2);
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
            if (styleB(StyleIdx::concertPitch) != a->isChecked())
                  cmdConcertPitchChanged(a->isChecked(), true);
            }
      else if (cmd == "reset-beammode")
            cmdResetBeamMode();
      else if (cmd == "clef-violin")
            cmdInsertClef(ClefType::G);
      else if (cmd == "clef-bass")
            cmdInsertClef(ClefType::F);
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
            LayoutBreak::Type type;
            if (cmd == "system-break")
                  type = LayoutBreak::Type::LINE;
            else if (cmd == "page-break")
                  type = LayoutBreak::Type::PAGE;
            else
                  type = LayoutBreak::Type::SECTION;

            if (el && el->type() == Element::Type::BAR_LINE && el->parent()->type() == Element::Type::SEGMENT) {
                  Measure* measure = static_cast<Measure*>(el->parent()->parent());
                  if (measure->isMMRest()) {
                        // if measure is mm rest, then propagate to last original measure
                        measure = measure->nextMeasure();
                        if (measure)
                              measure = measure->prevMeasure();
                        }
                  if (measure)
                        measure->undoSetBreak(!measure->lineBreak(), type);
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
      else if (cmd == "toggle-mmrest") {
            bool val = !styleB(StyleIdx::createMultiMeasureRests);
            deselectAll();
            undo(new ChangeStyleVal(this, StyleIdx::createMultiMeasureRests, val));
            }
      else if (cmd == "add-brackets") {
            cmdAddBracket();
      }
      else
            qDebug("unknown cmd <%s>", qPrintable(cmd));
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
//   cmdInsertClef
//    insert clef before cr
//---------------------------------------------------------

void Score::cmdInsertClef(Clef* clef, ChordRest* cr)
      {
      Clef* gclef = 0;
      for (Element* e : cr->linkList()) {
            ChordRest* cr = static_cast<ChordRest*>(e);
            Score* score  = cr->score();

            //
            // create a clef segment before cr if it does not exist
            //
            Segment* s  = cr->segment();
            Segment* cs = s->prev();
            int tick    = s->tick();
            if (!cs || cs->segmentType() != Segment::Type::Clef) {
                  cs = new Segment(cr->measure(), Segment::Type::Clef, tick);
                  cs->setNext(s);
                  score->undo(new AddElement(cs));
                  }
            else if (cs == cr->measure()->first() && cr->measure()->prevMeasure()) {
                  // move to end of previous measure
                  Measure* m = cr->measure()->prevMeasure();
                  cs = m->undoGetSegment(Segment::Type::Clef, tick);
                  }
            Clef* c = static_cast<Clef*>(gclef ? gclef->linkedClone() : clef->clone());
            gclef = c;
            c->setParent(cs);
            c->setScore(score);
            c->setTrack(cr->staffIdx() * VOICES);
            if (cs->element(c->track()))
                  score->undo(new RemoveElement(cs->element(c->track())));
            score->undo(new AddElement(c));
            }
      delete clef;
      }

}

