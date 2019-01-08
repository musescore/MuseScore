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

#include "types.h"
#include "musescoreCore.h"
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
#include "undo.h"
#include "timesig.h"
#include "repeat.h"
#include "tempotext.h"
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
#include "rehearsalmark.h"
#include "sym.h"

namespace Ms {

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void CmdState::reset()
      {
      layoutFlags         = LayoutFlag::NO_FLAGS;
      _updateMode         = UpdateMode::DoNothing;
      _startTick          = -1;
      _endTick            = -1;
      }

//---------------------------------------------------------
//   setTick
//---------------------------------------------------------

void CmdState::setTick(int t)
      {
      if (_startTick == -1 || t < _startTick)
            _startTick = t;
      if (_endTick == -1 || t > _endTick)
            _endTick = t;
      setUpdateMode(UpdateMode::Layout);
      }

//---------------------------------------------------------
//   setUpdateMode
//---------------------------------------------------------

void CmdState::_setUpdateMode(UpdateMode m)
      {
      _updateMode = m;
      }

void CmdState::setUpdateMode(UpdateMode m)
      {
      if (int(m) > int(_updateMode))
            _setUpdateMode(m);
      }

//---------------------------------------------------------
//   startCmd
///   Start a GUI command by clearing the redraw area
///   and starting a user-visible undo.
//---------------------------------------------------------

void Score::startCmd()
      {
      if (MScore::debugMode)
            qDebug("===startCmd()");

      cmdState().reset();

      // Start collecting low-level undo operations for a
      // user-visible undo action.
      if (undoStack()->active()) {
            qDebug("Score::startCmd(): cmd already active");
            return;
            }
      undoStack()->beginMacro(this);
      }

//---------------------------------------------------------
//   undoRedo
//---------------------------------------------------------

void Score::undoRedo(bool undo, EditData* ed)
      {
      if (readOnly())
            return;
      cmdState().reset();
      if (undo)
            undoStack()->undo(ed);
      else
            undoStack()->redo(ed);
      update();
      updateSelection();
      }

//---------------------------------------------------------
//   endCmd
///   End a GUI command by (if \a undo) ending a user-visble undo
///   and (always) updating the redraw area.
//---------------------------------------------------------

void Score::endCmd(bool rollback)
      {
      if (!undoStack()->active()) {
            qDebug("Score::endCmd(): no cmd active");
            update();
            return;
            }
      if (readOnly() || MScore::_error != MS_NO_ERROR)
            rollback = true;

      if (rollback)
            undoStack()->current()->unwind();

      update();

      if (MScore::debugMode)
            qDebug("===endCmd() %d", undoStack()->current()->childCount());
      const bool noUndo = undoStack()->current()->empty();       // nothing to undo?
      undoStack()->endMacro(noUndo);

      if (dirty()) {
            masterScore()->_playlistDirty = true;  // TODO: flag individual operations
            masterScore()->_autosaveDirty = true;
            }
      MuseScoreCore::mscoreCore->endCmd();
      cmdState().reset();
      }

#ifndef NDEBUG
//---------------------------------------------------------
//   CmdState::dump
//---------------------------------------------------------

void CmdState::dump()
      {
      qDebug("CmdState: mode %d %d-%d", int(_updateMode), _startTick, _endTick);
      // bool _excerptsChanged     { false };
      // bool _instrumentsChanged  { false };

      }
#endif

//---------------------------------------------------------
//   update
//    layout & update
//---------------------------------------------------------

void Score::update()
      {
      bool updateAll = false;
      for (MasterScore* ms : *movements()) {
            CmdState& cs = ms->cmdState();
            ms->deletePostponed();
            if (cs.layoutRange()) {
                  for (Score* s : ms->scoreList())
                        s->doLayoutRange(cs.startTick(), cs.endTick());
                  updateAll = true;
                  }
            }

      for (MasterScore* ms : *movements()) {
            CmdState& cs = ms->cmdState();
            if (updateAll || cs.updateAll()) {
                  for (Score* s : scoreList()) {
                        for (MuseScoreView* v : s->viewer) {
                              v->updateAll();
                              }
                        }
                  }
            else if (cs.updateRange()) {
                  // updateRange updates only current score
                  qreal d = spatium() * .5;
                  _updateState.refresh.adjust(-d, -d, 2 * d, 2 * d);
                  for (MuseScoreView* v : viewer)
                        v->dataChanged(_updateState.refresh);
                  _updateState.refresh = QRectF();
                  }
            const InputState& is = inputState();
            if (is.noteEntryMode() && is.segment()) {
                  setPlayPos(is.segment()->tick());
                  }
            if (_playlistDirty) {
                  emit playlistChanged();
                  _playlistDirty = false;
                  }
            cs.reset();
            }
      if (_selection.isRange())
            _selection.updateSelectedElements();
      }

//---------------------------------------------------------
//   deletePostponed
//---------------------------------------------------------

void Score::deletePostponed()
      {
      for (ScoreElement* e : _updateState._deleteList) {
            if (e->isSystem()) {
                  System* s = toSystem(e);
                  for (SpannerSegment* ss : s->spannerSegments()) {
                        if (ss->system() == s)
                              ss->setSystem(0);
                        }
                  }
            }
      qDeleteAll(_updateState._deleteList);
      _updateState._deleteList.clear();
      }

//---------------------------------------------------------
//   cmdAddSpanner
//   drop VOLTA, OTTAVA, TRILL, PEDAL, DYNAMIC
//        HAIRPIN, LET_RING, VIBRATO and TEXTLINE
//---------------------------------------------------------

void Score::cmdAddSpanner(Spanner* spanner, const QPointF& pos)
      {
      int staffIdx;
      Segment* segment;
      MeasureBase* mb = pos2measure(pos, &staffIdx, 0, &segment, 0);
      // ignore if we do not have a measure
      if (mb == 0 || mb->type() != ElementType::MEASURE) {
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
      else {      // Anchor::MEASURE, Anchor::CHORD, Anchor::NOTE
            Measure* m = toMeasure(mb);
            QRectF b(m->canvasBoundingRect());

            if (pos.x() >= (b.x() + b.width() * .5) && m != lastMeasureMM())
                  m = m->nextMeasure();
            spanner->setTick(m->tick());
            spanner->setTick2(m->endTick());
            }
      spanner->eraseSpannerSegments();

      undoAddElement(spanner);
      select(spanner, SelectType::SINGLE, 0);
      }

//---------------------------------------------------------
//   cmdAddSpanner
//    used when applying a spanner to a selection
//---------------------------------------------------------

void Score::cmdAddSpanner(Spanner* spanner, int staffIdx, Segment* startSegment, Segment* endSegment)
      {
      int track = staffIdx * VOICES;
      spanner->setTrack(track);
      spanner->setTrack2(track);
      for (auto ss : spanner->spannerSegments())
            ss->setTrack(track);
      spanner->setTick(startSegment->tick());
      int tick2;
      if (!endSegment)
            tick2 = lastSegment()->tick();
      else if (endSegment == startSegment)
            tick2 = startSegment->measure()->last()->tick();
      else
            tick2 = endSegment->tick();
      spanner->setTick2(tick2);
#if 0 // TODO
      TextLine* tl = toTextLine(spanner);
      if (tl) {
            StyledPropertyListIdx st;
            Text* t;
            // begin
            t = tl->beginTextElement();
            if (t) {
                  st = t->textStyleType();
                  if (st >= StyledPropertyListIdx::DEFAULT)
                        t->textStyle().restyle(MScore::baseStyle().textStyle(st), textStyle(st));
                  }
            // continue
            t = tl->continueTextElement();
            if (t) {
                  st = t->textStyleType();
                  if (st >= StyledPropertyListIdx::DEFAULT)
                        t->textStyle().restyle(MScore::baseStyle().textStyle(st), textStyle(st));
                  }
            // end
            t = tl->endTextElement();
            if (t) {
                  st = t->textStyleType();
                  if (st >= StyledPropertyListIdx::DEFAULT)
                        t->textStyle().restyle(MScore::baseStyle().textStyle(st), textStyle(st));
                  }
            }
#endif
      undoAddElement(spanner);
      }

//---------------------------------------------------------
//   expandVoice
//    fills gaps in voice with rests,
//    from previous cr (or beginning of measure) to next cr (or end of measure)
//---------------------------------------------------------

void Score::expandVoice(Segment* s, int track)
      {
      if (!s) {
            qDebug("expand voice: no segment");
            return;
            }
      if (s->element(track))
            return;

      // find previous segment with cr in this track
      Segment* ps;
      for (ps = s; ps; ps = ps->prev(SegmentType::ChordRest)) {
            if (ps->element(track))
                  break;
            }
      if (ps) {
            ChordRest* cr = toChordRest(ps->element(track));
            int tick = cr->tick() + cr->actualTicks();
            if (tick > s->tick()) {
                  // previous cr extends past current segment
                  qDebug("expandVoice: cannot insert element here");
                  return;
                  }
            if (cr->isChord()) {
                  // previous cr ends on or before current segment
                  // for chords, move ps to just after cr ends
                  // so we can fill any gap that might exist
                  // but don't move ps if previous cr is a rest
                  // this will be combined with any new rests needed to fill up to s->tick() below
                  ps = ps->measure()->undoGetSegment(SegmentType::ChordRest, tick);
                  }
            }
      //
      // fill up to s->tick() with rests
      //
      Measure* m = s->measure();
      int stick  = ps ?  ps->tick() : m->tick();
      int ticks  = s->tick() - stick;
      if (ticks)
            setRest(stick, track, Fraction::fromTicks(ticks), false, 0);

      //
      // fill from s->tick() until next chord/rest in measure
      //
      Segment* ns;
      for (ns = s->next(SegmentType::ChordRest); ns; ns = ns->next(SegmentType::ChordRest)) {
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

void Score::cmdAddInterval(int val, const std::vector<Note*>& nl)
      {
      startCmd();
      for (Note* on : nl) {
            Note* note = new Note(this);
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
                  Interval v = on->part()->instrument(tick)->transpose();
                  if (v.isZero())
                        ntpc1 = ntpc2 = ntpc;
                  else {
                        if (styleB(Sid::concertPitch)) {
                              v.flip();
                              ntpc1 = ntpc;
                              ntpc2 = Ms::transposeTpc(ntpc, v, true);
                              }
                        else {
                              npitch += v.chromatic;
                              ntpc2 = ntpc;
                              ntpc1 = Ms::transposeTpc(ntpc, v, true);
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
            if (npitch < 0 || npitch > 127) {
                  delete note;
                  endCmd();
                  return;
                  }
            note->setPitch(npitch, ntpc1, ntpc2);

            undoAddElement(note);
            setPlayNote(true);

            select(note, SelectType::SINGLE, 0);
            }
      _is.moveToNextInputPos();
      endCmd();
      }

//---------------------------------------------------------
//   setGraceNote
///   Create a grace note in front of a normal note.
///   \arg ch is the chord of the normal note
///   \arg pitch is the pitch of the grace note
///   \arg is the grace note type
///   \len is the visual duration of the grace note (1/16 or 1/32)
//---------------------------------------------------------

Note* Score::setGraceNote(Chord* ch, int pitch, NoteType type, int len)
      {
      Note* note = new Note(this);
      Chord* chord = new Chord(this);

      // allow grace notes to be added to other grace notes
      // by really adding to parent chord
      if (ch->noteType() != NoteType::NORMAL)
            ch = toChord(ch->parent());

      chord->setTrack(ch->track());
      chord->setParent(ch);
      chord->add(note);

      note->setPitch(pitch);
      // find corresponding note within chord and use its tpc information
      for (Note* n : ch->notes()) {
            if (n->pitch() == pitch) {
                  note->setTpc1(n->tpc1());
                  note->setTpc2(n->tpc2());
                  break;
                  }
            }
      // note with same pitch not found, derive tpc from pitch / key
      if (!tpcIsValid(note->tpc1()) || !tpcIsValid(note->tpc2()))
            note->setTpcFromPitch();

      TDuration d;
      d.setVal(len);
      chord->setDurationType(d);
      chord->setDuration(d.fraction());
      chord->setNoteType(type);
      chord->setMag(ch->staff()->mag(chord->tick()) * styleD(Sid::graceNoteMag));

      undoAddElement(chord);
      select(note, SelectType::SINGLE, 0);
      return note;
      }

//---------------------------------------------------------
//   createCRSequence
//    Create a rest or chord of len f.
//    If f is not a basic len, create several rests or
//    tied chords.
//
//    f     total len of ChordRest
//    cr    prototype CR
//    tick  start position in measure
//---------------------------------------------------------

void Score::createCRSequence(Fraction f, ChordRest* cr, int tick)
      {
      Measure* measure = cr->measure();
      ChordRest* ocr = 0;
      for (TDuration d : toDurationList(f, true)) {
            ChordRest* ncr = toChordRest(cr->clone());
            ncr->setDurationType(d);
            ncr->setDuration(d.fraction());

            if (cr->isChord() && ocr) {
                  Chord* nc = toChord(ncr);
                  Chord* oc = toChord(ocr);
                  for (unsigned int i = 0; i < oc->notes().size(); ++i) {
                        Note* on = oc->notes()[i];
                        Note* nn = nc->notes()[i];
                        Tie* tie = new Tie(this);
                        tie->setStartNote(on);
                        tie->setEndNote(nn);
                        tie->setTrack(cr->track());
                        on->setTieFor(tie);
                        nn->setTieBack(tie);
                        undoAddElement(tie);
                        }
                  }
            undoAddCR(ncr, measure, tick);
            tick += ncr->actualTicks();
            ocr = ncr;
            }
      }

//---------------------------------------------------------
//   setNoteRest
//    pitch == -1  -> set rest
//    return segment of last created note/rest
//---------------------------------------------------------

Segment* Score::setNoteRest(Segment* segment, int track, NoteVal nval, Fraction sd, Direction stemDirection, bool rhythmic)
      {
      Q_ASSERT(segment->segmentType() == SegmentType::ChordRest);

      bool isRest   = nval.pitch == -1;
      int tick      = segment->tick();
      Element* nr   = 0;
      Tie* tie      = 0;
      ChordRest* cr = toChordRest(segment->element(track));

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

            measure = segment->measure();
            std::vector<TDuration> dl;
            if (rhythmic)
                  dl = toRhythmicDurationList(dd, isRest, segment->rtick(), sigmap()->timesig(tick).nominal(), measure, 1);
            else
                  dl = toDurationList(dd, true);
            size_t n = dl.size();
            for (size_t i = 0; i < n; ++i) {
                  const TDuration& d = dl[i];
                  ChordRest* ncr;
                  Note* note = 0;
                  Tie* addTie = 0;
                  if (isRest) {
                        nr = ncr = new Rest(this);
                        nr->setTrack(track);
                        ncr->setDurationType(d);
                        ncr->setDuration(d == TDuration::DurationType::V_MEASURE ? measure->len() : d.fraction());
                        }
                  else {
                        nr = note = new Note(this);

                        if (tie) {
                              tie->setEndNote(note);
                              note->setTieBack(tie);
                              addTie = tie;
                              }
                        Chord* chord = new Chord(this);
                        chord->setTrack(track);
                        chord->setDurationType(d);
                        chord->setDuration(d.fraction());
                        chord->setStemDirection(stemDirection);
                        chord->add(note);
                        note->setNval(nval, tick);
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
                  if (addTie)
                        undoAddElement(addTie);
                  setPlayNote(true);
                  segment = ncr->segment();
                  tick += ncr->actualTicks();
                  }

            sd -= dd;
            if (sd.isZero())
                  break;

            Segment* nseg = tick2segment(tick, false, SegmentType::ChordRest);
            if (nseg == 0) {
                  qDebug("reached end of score");
                  break;
                  }
            segment = nseg;

            cr = toChordRest(segment->element(track));

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
            if (!isRest) {
                  tie = new Tie(this);
                  tie->setStartNote((Note*)nr);
                  tie->setTrack(nr->track());
                  ((Note*)nr)->setTieFor(tie);
                  }
            }
      if (tie)
            connectTies();
      if (nr) {
            if (_is.slur() && nr->type() == ElementType::NOTE) {
                  //
                  // extend slur
                  //
                  Chord* chord = toNote(nr)->chord();
                  _is.slur()->undoChangeProperty(Pid::SPANNER_TICKS, chord->tick() - _is.slur()->tick());
                  for (ScoreElement* se : _is.slur()->linkList()) {
                        Slur* slur = toSlur(se);
                        for (ScoreElement* ee : chord->linkList()) {
                              Element* e = static_cast<Element*>(ee);
                              if (e->score() == slur->score() && e->track() == slur->track2()) {
                                    slur->score()->undo(new ChangeSpannerElements(slur, slur->startElement(), e));
                                    break;
                                    }
                              }
                        }
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
      Fraction akkumulated;
      Fraction sd = _sd;

      //
      // remember first segment which should
      // not be deleted (it may contain other elements we want to preserve)
      //
      Segment* firstSegment = segment;
      int nextTick = segment->tick();

      for (Segment* seg = firstSegment; seg; seg = seg->next(SegmentType::ChordRest)) {
            //
            // voices != 0 may have gaps:
            //
            ChordRest* cr = toChordRest(seg->element(track));
            if (!cr) {
                  if (seg->tick() < nextTick)
                        continue;
                  Segment* seg1 = seg->next(SegmentType::ChordRest);
                  int tick2     = seg1 ? seg1->tick() : seg->measure()->tick() + seg->measure()->ticks();
                  segment       = seg;
                  Fraction td(Fraction::fromTicks(tick2 - seg->tick()));
                  if (td > sd)
                        td = sd;
                  akkumulated += td;
                  sd -= td;
                  if (sd.isZero())
                        return akkumulated;
                  nextTick = tick2;
                  continue;
                  }
            if (seg->tick() > nextTick) {
                  // there was a gap
                  Fraction td(Fraction::fromTicks(seg->tick() - nextTick));
                  if (td > sd)
                        td = sd;
                  akkumulated += td;
                  sd -= td;
                  if (sd.isZero())
                        return akkumulated;
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
            if (cr->isChord()) {
                  Chord* c = toChord(cr);
                  if (c->tremolo()) {
                        Tremolo* tremolo = c->tremolo();
                        if (tremolo->twoNotes())
                              undoRemoveElement(tremolo);
                        }
                  }
            Tuplet* ltuplet = cr->tuplet();
            if (ltuplet != tuplet) {
                  //
                  // Current location points to the start of a (nested)tuplet.
                  // We have to remove the complete tuplet.

                  // get top level tuplet
                  while (ltuplet->tuplet())
                        ltuplet = ltuplet->tuplet();

                  // get last segment of tuplet, drilling down to leaf nodes as necessary
                  Tuplet* t = ltuplet;
                  while (t->elements().back()->isTuplet())
                        t = toTuplet(t->elements().back());
                  seg = toChordRest(t->elements().back())->segment();

                  // now delete the full tuplet
                  td = ltuplet->duration();
                  cmdDeleteTuplet(ltuplet, false);
                  tuplet = 0;
                  }
            else {
                  if (seg != firstSegment || !keepChord)
                        undoRemoveElement(cr);
                  // even if there was a tuplet, we didn't remove it
                  ltuplet = 0;
                  }
            nextTick += td.ticks();
            if (sd < td) {
                  //
                  // we removed too much
                  //
                  akkumulated = _sd;
                  Fraction rd = td - sd;

                  std::vector<TDuration> dList = toDurationList(rd, false);
                  if (dList.empty())
                        return akkumulated;

                  Fraction f = sd / cr->staff()->timeStretch(cr->tick());
                  for (Tuplet* t = tuplet; t; t = t->tuplet())
                        f /= t->ratio();
                  int tick  = cr->tick() + f.ticks();

                  if ((tuplet == 0) && (((measure->tick() - tick) % dList[0].ticks()) == 0)) {
                        for (TDuration d : dList) {
                              if (ltuplet) {
                                    // take care not to recreate tuplet we just deleted
                                    Rest* r = setRest(tick, track, d.fraction(), false, 0, false);
                                    tick += r->actualTicks();
                                    }
                              else {
                                    tick += addClone(cr, tick, d)->actualTicks();
                                    }
                              }
                        }
                  else {
                        for (int i = int(dList.size()) - 1; i >= 0; --i) {
                              if (ltuplet) {
                                    // take care not to recreate tuplet we just deleted
                                    Rest* r = setRest(tick, track, dList[i].fraction(), false, 0, false);
                                    tick += r->actualTicks();
                                    }
                              else {
                                    tick += addClone(cr, tick, dList[i])->actualTicks();
                                    }
                              }
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
      Segment* seg = tick2segment(baseTick, true, SegmentType::ChordRest);
      if (!seg) {
            qDebug("no segment to paste at tick %d", baseTick);
            return false;
            }
      int strack = staffIdx * VOICES;
      for (int track = strack; track < strack + VOICES; track++) {
            if (voiceOffset[track-strack] == -1)
                  continue;
            int tick = baseTick + voiceOffset[track-strack];
            Measure* m   = tick2measure(tick);
            if ((track % VOICES) && !m->hasVoices(staffIdx))
                  continue;
            seg = m->undoGetSegment(SegmentType::ChordRest, tick);

            Fraction newLen = len - Fraction::fromTicks(voiceOffset[track-strack]);
            Q_ASSERT(newLen.numerator() != 0);
            bool result = makeGapVoice(seg, track, newLen, tick);
            if (track == strack && !result) // makeGap failed for first voice
                  return false;
            }
      return true;
      }

bool Score::makeGapVoice(Segment* seg, int track, Fraction len, int tick)
      {
      ChordRest* cr = 0;
      cr = toChordRest(seg->element(track));
      if (!cr) {
            // check if we are in the middle of a chord/rest
            Segment* seg1 = seg->prev(SegmentType::ChordRest);
            for (;;) {
                  if (seg1 == 0) {
                        if (!(track % VOICES))
                              qDebug("no segment before tick %d", tick);
                        // this happens only for voices other than voice 1
                        expandVoice(seg, track);
                        return makeGapVoice(seg,track,len,tick);
                        }
                  if (seg1->element(track))
                        break;
                  seg1 = seg1->prev(SegmentType::ChordRest);
                  }
            ChordRest* cr1 = toChordRest(seg1->element(track));
            Fraction srcF = cr1->duration();
            Fraction dstF = Fraction::fromTicks(tick - cr1->tick());
            std::vector<TDuration> dList = toDurationList(dstF, true);
            size_t n = dList.size();
            undoChangeChordRestLen(cr1, TDuration(dList[0]));
            if (n > 1) {
                  int crtick = cr1->tick() + cr1->actualTicks();
                  Measure* measure = tick2measure(crtick);
                  if (cr1->type() == ElementType::CHORD) {
                        // split Chord
                        Chord* c = toChord(cr1);
                        for (size_t i = 1; i < n; ++i) {
                              TDuration d = dList[i];
                              Chord* c2 = addChord(crtick, d, c, true, c->tuplet());
                              c = c2;
                              seg1 = c->segment();
                              crtick += c->actualTicks();
                              }
                        }
                  else {
                        // split Rest
                        Rest* r       = toRest(cr1);
                        for (size_t i = 1; i < n; ++i) {
                              TDuration d = dList[i];
                              Rest* r2      = toRest(r->clone());
                              r2->setDuration(d.fraction());
                              r2->setDurationType(d);
                              undoAddCR(r2, measure, crtick);
                              seg1 = r2->segment();
                              crtick += r2->actualTicks();
                              }
                        }
                  }
            setRest(tick, track, srcF - dstF, true, 0);
            for (;;) {
                  seg1 = seg1->next1(SegmentType::ChordRest);
                  if (seg1 == 0) {
                        qDebug("no segment");
                        return false;
                        }
                  if (seg1->element(track)) {
                        cr = toChordRest(seg1->element(track));
                        break;
                        }
                  }
            }

      for (;;) {
            if (!cr) {
                  qDebug("cannot make gap");
                  return false;
                  }
            Fraction l = makeGap(cr->segment(), cr->track(), len, 0);
            if (l.isZero()) {
                  qDebug("returns zero gap");
                  return false;
                  }
            len -= l;
            if (len.isZero())
                  break;
            // go to next cr
            Measure* m = cr->measure()->nextMeasure();
            if (m == 0) {
                  qDebug("EOS reached");
                  insertMeasure(ElementType::MEASURE, 0, false);
                  m = cr->measure()->nextMeasure();
                  if (m == 0) {
                        qDebug("===EOS reached");
                        return true;
                        }
                  }
            // first segment in measure was removed, have to recreate it
            Segment* s = m->undoGetSegment(SegmentType::ChordRest, m->tick());
            int t  = cr->track();
            cr = toChordRest(s->element(t));
            if (cr == 0) {
                  addRest(s, t, TDuration(TDuration::DurationType::V_MEASURE), 0);
                  cr = toChordRest(s->element(t));
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
            if (tuplet->tuplet())
                  return flist; // do no deal with nested tuplets
            Fraction rest = tuplet->elementsDuration();
            for (DurationElement* de : tuplet->elements()) {
                  if (de == cr)
                        break;
                  rest -= de->duration();
                  }
            if (rest < gap)
                  qDebug("does not fit in tuplet");
            else
                  flist.append(gap);
            return flist;
            }

      Segment* s = cr->segment();
      while (gap > Fraction(0)) {
            Measure* m    = s->measure();
            Fraction timeStretch = cr->staff()->timeStretch(s->tick());
            Fraction rest = (m->len() - s->rfrac()) * timeStretch;
            if (rest >= gap) {
                  flist.append(gap);
                  return flist;
                  }
            flist.append(rest);
            gap -= rest;
            m = m->nextMeasure();
            if (m == 0)
                  return flist;
            s = m->first(SegmentType::ChordRest);
            }
      return flist;
      }

//---------------------------------------------------------
//   changeCRlen
//---------------------------------------------------------

void Score::changeCRlen(ChordRest* cr, const TDuration& d)
      {
      Fraction dstF;
      if (d.type() == TDuration::DurationType::V_MEASURE)
            dstF = cr->measure()->stretchedLen(cr->staff());
      else
            dstF = d.fraction();
      changeCRlen(cr, dstF);
      }

void Score::changeCRlen(ChordRest* cr, const Fraction& dstF, bool fillWithRest)
      {
      if (cr->isRepeatMeasure()) {
            // it is not clear what should this
            // operation mean for measure repeats.
            return;
            }
      Fraction srcF(cr->duration());
      if (srcF == dstF)
            return;

      //keep selected element if any
      Element* selElement = selection().isSingle() ? getSelectedElement() : 0;

      int track      = cr->track();
      Tuplet* tuplet = cr->tuplet();

      if (srcF > dstF) {
            //
            // make shorter and fill with rest
            //
            deselectAll();
            if (cr->isChord()) {
                  //
                  // remove ties and tremolo between 2 notes
                  //
                  Chord* c = toChord(cr);
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
            std::vector<TDuration> dList = toDurationList(dstF, true);
            undoChangeChordRestLen(cr, dList[0]);
            int tick2 = cr->tick();
            for (unsigned i = 1; i < dList.size(); ++i) {
                  tick2 += dList[i-1].ticks();
                  TDuration d = dList[i];
                  setRest(tick2, track, d.fraction(), (d.dots() > 0), tuplet);
                  }
            if (fillWithRest)
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
      if (flist.empty())
            return;

      deselectAll();

      int tick       = cr->tick();
      Fraction f     = dstF;
      ChordRest* cr1 = cr;
      Chord* oc      = 0;

      bool first = true;
      for (Fraction f2 : flist) {
            f  -= f2;
            makeGap(cr1->segment(), cr1->track(), f2, tuplet, first);

            if (cr->isRest()) {
                  Fraction timeStretch = cr1->staff()->timeStretch(cr1->tick());
                  Rest* r = toRest(cr);
                  if (first) {
                        std::vector<TDuration> dList = toDurationList(f2, true);
                        undoChangeChordRestLen(cr, dList[0]);
                        int tick2 = cr->tick();
                        for (unsigned i = 1; i < dList.size(); ++i) {
                              tick2 += dList[i-1].ticks();
                              TDuration d = dList[i];
                              setRest(tick2, track, d.fraction() * timeStretch, (d.dots() > 0), tuplet);
                              }
                        }
                  else {
                        r = setRest(tick, track, f2 * timeStretch, false, tuplet);
                        }
                  if (first) {
                        select(r, SelectType::SINGLE, 0);
                        first = false;
                        }
                  tick += f2.ticks() * timeStretch.numerator() / timeStretch.denominator();
                  }
            else {
                  std::vector<TDuration> dList = toDurationList(f2, true);
                  Measure* measure             = tick2measure(tick);
                  int etick                    = measure->tick();

                  if (((tick - etick) % dList[0].ticks()) == 0) {
                        for (TDuration du : dList) {
                              bool genTie;
                              Chord* cc;
                              if (oc) {
                                    genTie = true;
                                    cc = oc;
                                    oc = addChord(tick, du, cc, genTie, tuplet);
                                    }
                              else {
                                    genTie = false;
                                    cc = toChord(cr);
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
                        for (int i = int(dList.size()) - 1; i >= 0; --i) {
                              bool genTie;
                              Chord* cc;
                              if (oc) {
                                    genTie = true;
                                    cc = oc;
                                    oc = addChord(tick, dList[i], cc, genTie, tuplet);
                                    }
                              else {
                                    genTie = false;
                                    cc = toChord(cr);
                                    undoChangeChordRestLen(cr, dList[i]);
                                    oc = cc;
                                    }
                              if (first) {
                                    // select(oc, SelectType::SINGLE, 0);
                                    if (selElement)
                                          select(selElement, SelectType::SINGLE, 0);
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
            Segment* s = m1->first(SegmentType::ChordRest);
            expandVoice(s, track);
            cr1 = toChordRest(s->element(track));
            }
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

void Score::upDown(bool up, UpDownMode mode, bool updateSelection)
      {
      QList<Note*> el = selection().uniqueNotes();

      for (Note* oNote : el) {
            int tick     = oNote->chord()->tick();
            Staff* staff = oNote->staff();
            Part* part   = staff->part();
            Key key      = staff->key(tick);
            int tpc1     = oNote->tpc1();
            int tpc2     = oNote->tpc2();
            int pitch    = oNote->pitch();
            int newTpc1  = tpc1;      // default to unchanged
            int newTpc2  = tpc2;      // default to unchanged
            int newPitch = pitch;     // default to unchanged
            int string   = oNote->string();
            int fret     = oNote->fret();

            switch (staff->staffType(oNote->chord()->tick())->group()) {
                  case StaffGroup::PERCUSSION:
                        {
                        const Drumset* ds = part->instrument()->drumset();
                        if (ds) {
                              newPitch = up ? ds->nextPitch(pitch) : ds->prevPitch(pitch);
                              newTpc1 = pitch2tpc(newPitch, Key::C, Prefer::NEAREST);
                              newTpc2 = newTpc1;
                              }
                        }
                        break;
                  case StaffGroup::TAB:
                        {
                        const StringData* stringData = part->instrument()->stringData();
                        switch (mode) {
                              case UpDownMode::OCTAVE:          // move same note to next string, if possible
                                    {
                                    const StaffType* stt = staff->staffType(tick);
                                    string = stt->physStringToVisual(string);
                                    string += (up ? -1 : 1);
                                    if (string < 0 || string >= stringData->strings())
                                          return;           // no next string to move to
                                    string = stt->visualStringToPhys(string);
                                    fret = stringData->fret(pitch, string, staff, tick);
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
                                    // compute new fret
                                    if (!stringData->frets()) {
                                          qDebug("upDown tab chromatic: no frets?");
                                          return;
                                          }
                                    fret += (up ? 1 : -1);
                                    if (fret < 0 || fret > stringData->frets()) {
                                          qDebug("upDown tab in-string: out of fret range");
                                          return;
                                          }
                                    // update pitch and tpc's and check it matches stringData
                                    upDownChromatic(up, pitch, oNote, key, tpc1, tpc2, newPitch, newTpc1, newTpc2);
                                    if (newPitch != stringData->getPitch(string, fret, staff, tick) ) {
                                          // oh-oh: something went very wrong!
                                          qDebug("upDown tab in-string: pitch mismatch");
                                          return;
                                    }
                                    // store the fretting change before undoChangePitch() chooses
                                    // a fretting of its own liking!
                                    oNote->undoChangeProperty(Pid::FRET, fret);
                                    }
                                    break;
                              }
                        }
                        break;
                  case StaffGroup::STANDARD:
                        switch (mode) {
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
                  auto l = oNote->linkList();
                  for (ScoreElement* e : l) {
                        Note* ln = toNote(e);
                        if (ln->accidental())
                              undo(new RemoveElement(ln->accidental()));
                        }
                  undoChangePitch(oNote, newPitch, newTpc1, newTpc2);
                  }

            // store fret change only if undoChangePitch has not been called,
            // as undoChangePitch() already manages fret changes, if necessary
            else if (staff->staffType(tick)->group() == StaffGroup::TAB) {
                  bool refret = false;
                  if (oNote->string() != string) {
                        oNote->undoChangeProperty(Pid::STRING, string);
                        refret = true;
                        }
                  if (oNote->fret() != fret) {
                        oNote->undoChangeProperty(Pid::FRET, fret);
                        refret = true;
                        }
                  if (refret) {
                        const StringData* stringData = part->instrument()->stringData();
                        stringData->fretChords(oNote->chord());
                        }
                  }

            // play new note with velocity 80 for 0.3 sec:
            setPlayNote(true);
            }

      if (updateSelection) {
            _selection.clear();
            for (Note* note : el)
                  _selection.add(note);
            }
      }

//---------------------------------------------------------
//   upDownDelta
///   Add the delta to the pitch of note.
//---------------------------------------------------------

void Score::upDownDelta(int pitchDelta, bool updateSelection)
      {
      while (pitchDelta > 0) {
            upDown(true, UpDownMode::CHROMATIC, updateSelection);
            pitchDelta--;
            }

      while (pitchDelta < 0) {
            upDown(false, UpDownMode::CHROMATIC, updateSelection);
            pitchDelta++;
            }
      }

//---------------------------------------------------------
//   addArticulation
///   Add attribute \a attr to all selected notes/rests.
///
///   Called from padToggle() to add note prefix/accent.
//---------------------------------------------------------

void Score::addArticulation(SymId attr)
      {
      QSet<Chord*> set;
      for (Element* el : selection().elements()) {
            if (el->isNote() || el->isChord()) {
                  Chord* cr = 0;
                  // apply articulation on a given chord only once
                  if (el->isNote()) {
                        cr = toNote(el)->chord();
                        if (set.contains(cr))
                              continue;
                        }
                  Articulation* na = new Articulation(this);
                  na->setSymId(attr);
                  if (!addArticulation(el, na))
                        delete na;
                  if (cr)
                        set.insert(cr);
                  }
            }
      }

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \a idx for all selected
///   notes.
//---------------------------------------------------------

void Score::changeAccidental(AccidentalType idx)
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
      Staff* st     = chord->staff();
      int fret      = n->fret();
      int string    = n->string();

      if (st->isTabStaff(chord->tick())) {
            if (pitch != n->pitch()) {
                  //
                  // as pitch has changed, calculate new
                  // string & fret
                  //
                  const StringData* stringData = n->part()->instrument()->stringData();
                  if (stringData)
                        stringData->convertPitch(pitch, st, chord->tick(), &string, &fret);
                  }
            }
      int tpc1;
      int tpc2 = n->transposeTpc(tpc);
      if (score->styleB(Sid::concertPitch))
            tpc1 = tpc;
      else {
            tpc1 = tpc2;
            tpc2 = tpc;
            }

      if (!st->isTabStaff(chord->tick())) {
            //
            // handle ties
            //
            if (n->tieBack()) {
                  if (pitch != n->pitch()) {
                        score->undoRemoveElement(n->tieBack());
                        if (n->tieFor())
                              score->undoRemoveElement(n->tieFor());
                        }
                  }
            else {
                  Note* nn = n;
                  while (nn->tieFor()) {
                        nn = nn->tieFor()->endNote();
                        score->undo(new ChangePitch(nn, pitch, tpc1, tpc2));
                        }
                  }
            }
      score->undoChangePitch(n, pitch, tpc1, tpc2);
      }

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \accidental for
///   note \a note.
//---------------------------------------------------------

void Score::changeAccidental(Note* note, AccidentalType accidental)
      {
      Chord* chord = note->chord();
      if (!chord)
            return;
      Segment* segment = chord->segment();
      if (!segment)
            return;
      Measure* measure = segment->measure();
      if (!measure)
            return;
      int tick = segment->tick();
      Staff* estaff = staff(chord->staffIdx() + chord->staffMove());
      if (!estaff)
            return;
      ClefType clef = estaff->clef(tick);
      int step      = ClefInfo::pitchOffset(clef) - note->line();
      while (step < 0)
            step += 7;
      step %= 7;
      //
      // accidental change may result in pitch change
      //
      AccidentalVal acc2 = measure->findAccidental(note);
      AccidentalVal acc = (accidental == AccidentalType::NONE) ? acc2 : Accidental::subtype2value(accidental);

      int pitch = line2pitch(note->line(), clef, Key::C) + int(acc);
      if (!note->concertPitch())
            pitch += note->transposition();

      int tpc = step2tpc(step, acc);

      bool forceRemove = false;
      bool forceAdd = false;

      // delete accidental
      // both for this note and for any linked notes
      if (accidental == AccidentalType::NONE)
            forceRemove = true;

      // precautionary or microtonal accidental
      // either way, we display it unconditionally
      // both for this note and for any linked notes
      else if (acc == acc2 || pitch == note->pitch() || Accidental::isMicrotonal(accidental))
            forceAdd = true;

      for (ScoreElement* se : note->linkList()) {
            Note* ln = toNote(se);
            if (ln->concertPitch() != note->concertPitch())
                  continue;
            Score* lns    = ln->score();
            Accidental* a = ln->accidental();
            if (forceRemove) {
                  if (a)
                        lns->undoRemoveElement(a);
                  if (ln->tieBack())
                        continue;
                  }
            else if (forceAdd) {
                  if (a)
                        undoRemoveElement(a);
                  Accidental* a1 = new Accidental(lns);
                  a1->setParent(ln);
                  a1->setAccidentalType(accidental);
                  a1->setRole(AccidentalRole::USER);
                  lns->undoAddElement(a1);
                  }
            changeAccidental2(ln, pitch, tpc);
            }
      setPlayNote(true);
      }

//---------------------------------------------------------
//   addArticulation
//---------------------------------------------------------

bool Score::addArticulation(Element* el, Articulation* a)
      {
      Chord* c;
      if (el->isNote())
            c = toNote(el)->chord();
      else if (el->isChord())
            c = toChord(el);
      else
            return false;
      Articulation* oa = c->hasArticulation(a);
      if (oa) {
            undoRemoveElement(oa);
            return false;
            }
      a->setParent(c);
      a->setTrack(c->track()); // make sure it propagates between score and parts
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
      if (!s1)
            m1 = firstMeasureMM();
      else
            m1 = s1->measure();
      if (!s2)
            m2 = lastMeasureMM();
      else
            m2 = s2->measure();
      if (!m1 || !m2)               // should not happen!
            return;

      for (Measure* m = m1; m; m = m->nextMeasureMM()) {
            m->undoChangeProperty(Pid::USER_STRETCH, 1.0);
            if (m == m2)
                  break;
            }
      }

//---------------------------------------------------------
//   moveUp
//---------------------------------------------------------

void Score::moveUp(ChordRest* cr)
      {
      Staff* staff  = cr->staff();
      Part* part    = staff->part();
      int rstaff    = staff->rstaff();
      int staffMove = cr->staffMove();

      if ((staffMove == -1) || (rstaff + staffMove <= 0))
            return;

      QList<Staff*>* staves = part->staves();
      // we know that staffMove+rstaff-1 index exists due to the previous condition.
      if (staff->staffType(cr->tick())->group() != StaffGroup::STANDARD ||
          staves->at(rstaff+staffMove-1)->staffType(cr->tick())->group() != StaffGroup::STANDARD) {
            qDebug("User attempted to move a note from/to a staff which does not use standard notation - ignoring.");
            }
      else  {
            // move the chord up a staff
            undo(new ChangeChordStaffMove(cr, staffMove - 1));
            }
      }

//---------------------------------------------------------
//   moveDown
//---------------------------------------------------------

void Score::moveDown(ChordRest* cr)
      {
      Staff* staff  = cr->staff();
      Part* part    = staff->part();
      int rstaff    = staff->rstaff();
      int staffMove = cr->staffMove();
      // calculate the number of staves available so that we know whether there is another staff to move down to
      int rstaves   = part->nstaves();

      if ((staffMove == 1) || (rstaff + staffMove >= rstaves - 1)) {
            qDebug("moveDown staffMove==%d  rstaff %d rstaves %d", staffMove, rstaff, rstaves);
            return;
            }

      QList<Staff*>* staves = part->staves();
      // we know that staffMove+rstaff+1 index exists due to the previous condition.
      if (staff->staffType(cr->tick())->group() != StaffGroup::STANDARD ||
          staves->at(staffMove+rstaff+1)->staffType(cr->tick())->group() != StaffGroup::STANDARD) {
            qDebug("User attempted to move a note from/to a staff which does not use standard notation - ignoring.");
            }
      else  {
            // move the chord down a staff
            undo(new ChangeChordStaffMove(cr, staffMove + 1));
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
            if (stretch < 0)
                  stretch = 0;
            m->undoChangeProperty(Pid::USER_STRETCH, stretch);
            }
      }

//---------------------------------------------------------
//   cmdResetBeamMode
//---------------------------------------------------------

void Score::cmdResetBeamMode()
      {
      bool noSelection = selection().isNone();
      if (noSelection)
            cmdSelectAll();
      else if (!selection().isRange()) {
            qDebug("no system or staff selected");
            return;
            }

      int endTick   = selection().tickEnd();

      for (Segment* seg = selection().firstChordRestSegment(); seg && seg->tick() < endTick; seg = seg->next1(SegmentType::ChordRest)) {
            for (int track = selection().staffStart() * VOICES; track < selection().staffEnd() * VOICES; ++track) {
                  ChordRest* cr = toChordRest(seg->element(track));
                  if (cr == 0)
                        continue;
                  if (cr->type() == ElementType::CHORD) {
                        if (cr->beamMode() != Beam::Mode::AUTO)
                              cr->undoChangeProperty(Pid::BEAM_MODE, int(Beam::Mode::AUTO));
                        }
                  else if (cr->type() == ElementType::REST) {
                        if (cr->beamMode() != Beam::Mode::NONE)
                              cr->undoChangeProperty(Pid::BEAM_MODE, int(Beam::Mode::NONE));
                        }
                  }
            }
      }

//---------------------------------------------------------
//   cmdResetStyle
//---------------------------------------------------------

void Score::cmdResetStyle()
      {
      style().reset(this);
      }

//---------------------------------------------------------
//   cmdResetNoteAndRestGroupings
//---------------------------------------------------------

void Score::cmdResetNoteAndRestGroupings()
      {
      if (selection().isNone())
            cmdSelectAll();
      else if (!selection().isRange()) {
            qDebug("no system or staff selected");
            return;
            }

      // save selection values because selection changes during grouping
      int sTick = selection().tickStart();
      int eTick = selection().tickEnd();
      int sStaff = selection().staffStart();
      int eStaff = selection().staffEnd();

      startCmd();
      for (int staff = sStaff; staff < eStaff; staff++) {
            int sTrack = staff * VOICES;
            int eTrack = sTrack + VOICES;
            for (int track = sTrack; track < eTrack; track++) {
                  if (selectionFilter().canSelectVoice(track))
                        regroupNotesAndRests(sTick, eTick, track);
                  }
            }
      endCmd();
      }

//---------------------------------------------------------
//   resetElementShapePosition
//    For use with Score::scanElements.
//    Reset positions and autoplacement for the given
//    element.
//---------------------------------------------------------

static void resetElementPosition(void*, Element* e)
      {
      if (e->generated())
            return;
      e->undoResetProperty(Pid::AUTOPLACE);
      e->undoResetProperty(Pid::OFFSET);
      if (e->isSpanner())
            e->undoResetProperty(Pid::OFFSET2);
      }

//---------------------------------------------------------
//   cmdResetAllPositions
//---------------------------------------------------------

void Score::cmdResetAllPositions()
      {
      startCmd();
      scanElements(nullptr, resetElementPosition);
      endCmd();
      }

//---------------------------------------------------------
//   processMidiInput
//---------------------------------------------------------

bool Score::processMidiInput()
      {
      if (midiInputQueue()->empty())
            return false;

      if (MScore::debugMode)
          qDebug("processMidiInput");

      NoteEntryMethod entryMethod = _is.noteEntryMethod();
      bool cmdActive = false;
      while (!midiInputQueue()->empty()) {
            MidiInputEvent ev = midiInputQueue()->dequeue();
            for (auto itr = activeMidiPitches()->begin(); itr != activeMidiPitches()->end();) {
                  if ((*itr).pitch == ev.pitch)
                        itr = activeMidiPitches()->erase(itr);
                  else
                        ++itr;
                  }
            if (MScore::debugMode)
                  qDebug("<-- !noteentry dequeue %i", ev.pitch);
            if (!noteEntryMode()
                        || entryMethod == NoteEntryMethod::REALTIME_AUTO
                        || entryMethod == NoteEntryMethod::REALTIME_MANUAL) {
                  int staffIdx = selection().staffStart();
                  Part* p;
                  if (staffIdx < 0 || staffIdx >= nstaves())
                        p = staff(0)->part();
                  else
                        p = staff(staffIdx)->part();
                  if (p) {
                        if (!styleB(Sid::concertPitch)) {
                              ev.pitch += p->instrument(selection().tickStart())->transpose().chromatic;
                              }
                        MScore::seq->startNote(
                                          p->instrument()->channel(0)->channel(),
                                          ev.pitch,
                                          ev.velocity,
                                          0.0);
                        }
                  }
            if (noteEntryMode()) {
                  if (ev.velocity == 0) {
                        // delete note in realtime mode
                        //Chord* chord = toChord(_is.cr());
                        //std::vector<Note*> notes = chord->notes();
                        if (entryMethod == NoteEntryMethod::REALTIME_AUTO || entryMethod == NoteEntryMethod::REALTIME_MANUAL) {
                              if (_is.cr()->isChord()) {
                                    Note* n = toChord(_is.cr())->findNote(ev.pitch);
                                    if (n) {
                                          qDebug("Pitches match! Note %i, Pitch %i", n->pitch(), ev.pitch);
                                          if (!cmdActive) {
                                                startCmd();
                                                cmdActive = true;
                                                }
                                          deleteItem(n->tieBack());
                                          deleteItem(n);
                                          }
                                    }
                              }
                        continue;
                        }
                  if (!cmdActive) {
                        startCmd();
                        cmdActive = true;
                        }
                  if (activeMidiPitches()->empty())
                        ev.chord = false;
                  else
                        ev.chord = true;

                  // holding shift while inputting midi will add the new pitch to the prior existing chord
                  if (qApp->keyboardModifiers() & Qt::ShiftModifier) {
                        Element* cr = _is.lastSegment()->element(_is.track());
                        if (cr && cr->isChord())
                              ev.chord = true;
                        }

                  // TODO: add shadow note instead of real note in realtime modes
                  // (note becomes real when realtime-advance triggered).
                  addMidiPitch(ev.pitch, ev.chord);
                  activeMidiPitches()->push_back(ev);
                  }
            }
      if (cmdActive) {
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
      if (noteEntryMode()) {
            // if selection exists and is grace note, use it
            // otherwise use chord/rest at input position
            // also use it if we are moving to next chord
            // to catch up with the cursor and not move the selection by 2 positions
            cr = selection().cr();
            if (cr && (cr->isGrace() || cmd == "next-chord" || cmd == "prev-chord"))
                  ;
            else
                  cr = inputState().cr();
            }
      else if (selection().activeCR())
            cr = selection().activeCR();
      else
            cr = selection().lastChordRest();

      // no chord/rest found? look for another type of element
      if (cr == 0) {
            if (selection().elements().empty())
                  return 0;
            // retrieve last element of section list
            Element* el = selection().elements().last();
            Element* trg = 0;

            // get parent of element and process accordingly:
            // trg is the element to select on "next-chord" cmd
            // cr is the ChordRest to move from on other cmd's
            int track = el->track();            // keep note of element track
            el = el->parent();
            // element with no parent (eg, a newly-added line) - no way to find context
            if (!el)
                  return 0;
            switch (el->type()) {
                  case ElementType::NOTE:           // a note is a valid target
                        trg = el;
                        cr  = toNote(el)->chord();
                        break;
                  case ElementType::CHORD:          // a chord or a rest are valid targets
                  case ElementType::REST:
                        trg = el;
                        cr  = toChordRest(trg);
                        break;
                  case ElementType::SEGMENT: {      // from segment go to top chordrest in segment
                        Segment* seg  = toSegment(el);
                        // if segment is not chord/rest or grace, move to next chord/rest or grace segment
                        if (!seg->isChordRest()) {
                              seg = seg->next1(SegmentType::ChordRest);
                              if (seg == 0)     // if none found, return failure
                                    return 0;
                              }
                        // segment for sure contains chords/rests,
                        int size = int(seg->elist().size());
                        // if segment has a chord/rest in original element track, use it
                        if (track > -1 && track < size && seg->element(track)) {
                              trg  = seg->element(track);
                              cr = toChordRest(trg);
                              break;
                              }
                        // if not, get topmost chord/rest
                        for (int i = 0; i < size; i++)
                              if (seg->element(i)) {
                                    trg  = seg->element(i);
                                    cr = toChordRest(trg);
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
                  if (trg->type() == ElementType::CHORD)
                        trg = toChord(trg)->upNote();
                  setPlayNote(true);
                  select(trg, SelectType::SINGLE, 0);
                  return trg;
                  }
            // if no chordrest found, do nothing
            if (cr == 0)
                  return 0;
            // if some chordrest found, continue with default processing
            }

      Element* el = 0;
      Segment* ois = noteEntryMode() ? _is.segment() : nullptr;
      Measure* oim = ois ? ois->measure() : nullptr;

      if (cmd == "next-chord") {
            // note input cursor
            if (noteEntryMode())
                  _is.moveToNextInputPos();

            // selection "cursor"
            // find next chordrest, which might be a grace note
            // this may override note input cursor
            el = nextChordRest(cr);
            while (el && el->isRest() && toRest(el)->isGap())
                  el = nextChordRest(toChordRest(el));
            if (el && noteEntryMode()) {
                  // do not use if not in original or new measure (don't skip measures)
                  Measure* m = toChordRest(el)->measure();
                  Segment* nis = _is.segment();
                  Measure* nim = nis ? nis->measure() : nullptr;
                  if (m != oim && m != nim)
                        el = cr;
                  // do not use if new input segment is current cr
                  // this means input cursor just caught up to current selection
                  else if (cr && nis == cr->segment())
                        el = cr;
                  }
            else if (!el)
                  el = cr;
            }
      else if (cmd == "prev-chord") {
            // note input cursor
            if (noteEntryMode() && _is.segment()) {
                  Measure* m = _is.segment()->measure();
                  Segment* s = _is.segment()->prev1(SegmentType::ChordRest);
                  int track = _is.track();
                  for (; s; s = s->prev1(SegmentType::ChordRest)) {
                        if (s->element(track) || s->measure() != m) {
                              if (s->element(track)) {
                                    if (s->element(track)->isRest() && toRest(s->element(track))->isGap())
                                          continue;
                                    }
                              break;
                              }
                        }
                  _is.moveInputPos(s);
                  }

            // selection "cursor"
            // find previous chordrest, which might be a grace note
            // this may override note input cursor
            el = prevChordRest(cr);
            while (el && el->isRest() && toRest(el)->isGap())
                  el = prevChordRest(toChordRest(el));
            if (el && noteEntryMode()) {
                  // do not use if not in original or new measure (don't skip measures)
                  Measure* m = toChordRest(el)->measure();
                  Segment* nis = _is.segment();
                  Measure* nim = nis ? nis->measure() : nullptr;
                  if (m != oim && m != nim)
                        el = cr;
                  // do not use if new input segment is current cr
                  // this means input cursor just caught up to current selection
                  else if (cr && nis == cr->segment())
                        el = cr;
                  }
            else if (!el)
                  el = cr;

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
            if (el->type() == ElementType::CHORD)
                  el = toChord(el)->upNote();       // originally downNote
            setPlayNote(true);
            if (noteEntryMode()) {
                  // if cursor moved into a gap, selection cannot follow
                  // only select & play el if it was not already selected (does not normally happen)
                  if (_is.cr() || !el->selected())
                        select(el, SelectType::SINGLE, 0);
                  else
                        setPlayNote(false);
                  for (MuseScoreView* view : viewer)
                        view->moveCursor();
                  }
            else {
                  select(el, SelectType::SINGLE, 0);
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
            el = nextChordRest(cr, true);
      else if (cmd == "select-prev-chord")
            el = prevChordRest(cr, true);
      else if (cmd == "select-next-measure")
            el = nextMeasure(cr, true, true);
      else if (cmd == "select-prev-measure")
            el = prevMeasure(cr, true);
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
            Measure* measure = firstMeasureMM();
            if (!measure)
                  return 0;
            el = measure->first()->nextChordRest(cr->track());
            }
      else if (cmd == "select-end-score") {
            Measure* measure = lastMeasureMM();
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
            if (e->type() == ElementType::NOTE) {
                  Note* note = toNote(e);
                  if (note->staff() && note->staff()->isTabStaff(note->chord()->tick()))
                        e->undoChangeProperty(Pid::GHOST, !note->ghost());
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
//   cmdIncDecDuration
//     When stepDotted is false and nSteps is 1 or -1, will halve or double the duration
//     When stepDotted is true, will step by nearest dotted or undotted note
//---------------------------------------------------------

void Score::cmdIncDecDuration(int nSteps, bool stepDotted)
      {
      Element* el = selection().element();
      if (el == 0)
            return;
      if (el->type() == ElementType::NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = toChordRest(el);

      // if measure rest is selected as input, then the correct initialDuration will be the duration of the measure's time signature, else is just the input state's duration
      TDuration initialDuration = (cr->durationType() == TDuration::DurationType::V_MEASURE) ? TDuration(cr->measure()->timesig()) : _is.duration();

      TDuration d = initialDuration.shiftRetainDots(nSteps, stepDotted);
      if (!d.isValid())
            return;
      if (cr->type() == ElementType::CHORD && (toChord(cr)->noteType() != NoteType::NORMAL)) {
            //
            // handle appoggiatura and acciaccatura
            //
            undoChangeChordRestLen(cr, d);
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
            if (el->type() == ElementType::NOTE) {
                  Note* n = toNote(el);
                  n->addParentheses();
                  }
            else if (el->type() == ElementType::ACCIDENTAL) {
                  Accidental* acc = toAccidental(el);
                  acc->undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::PARENTHESIS));
                  }
            else if (el->type() == ElementType::HARMONY) {
                  Harmony* h = toHarmony(el);
                  h->setLeftParen(true);
                  h->setRightParen(true);
                  h->render();
                  }
            }
      }


//---------------------------------------------------------
//   cmdMoveRest
//---------------------------------------------------------

void Score::cmdMoveRest(Rest* rest, Direction dir)
      {
      QPointF pos(rest->offset());
      if (dir == Direction::UP)
            pos.ry() -= spatium();
      else if (dir == Direction::DOWN)
            pos.ry() += spatium();
      rest->undoChangeProperty(Pid::OFFSET, pos);
      }

//---------------------------------------------------------
//   cmdMoveLyrics
//---------------------------------------------------------

void Score::cmdMoveLyrics(Lyrics* lyrics, Direction dir)
      {
      int verse = lyrics->no() + (dir == Direction::UP ? -1 : 1);
      if (verse < 0)
            return;
      lyrics->undoChangeProperty(Pid::VERSE, verse);
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
      undoChangeClef(cr->staff(), cr->segment(), clef->clefType());
      delete clef;
      }

//---------------------------------------------------------
//   cmdAddGrace
///   adds grace note of specified type to selected notes
//---------------------------------------------------------

void Score::cmdAddGrace (NoteType graceType, int duration)
      {
      for (Element* e : selection().elements()) {
            if (e->type() == ElementType::NOTE) {
                  Note* n = toNote(e);
                  setGraceNote(n->chord(), n->pitch(), graceType, duration);
                  }
            }
      }

//---------------------------------------------------------
//   cmdExplode
///   explodes contents of top selected staff into subsequent staves
//---------------------------------------------------------

void Score::cmdExplode()
      {
      if (!selection().isRange())
            return;

      int srcStaff  = selection().staffStart();
      int lastStaff = selection().staffEnd();
      int srcTrack  = srcStaff * VOICES;

      // reset selection to top staff only
      // force complete measures
      Segment* startSegment = selection().startSegment();
      Segment* endSegment = selection().endSegment();
      Measure* startMeasure = startSegment->measure();
      Measure* endMeasure = endSegment ? endSegment->measure() : lastMeasure();

      int lTick = endMeasure->endTick();
      bool voice = false;

      for (Measure* m = startMeasure; m && m->tick() != lTick; m = m->nextMeasure()) {
            if (m->hasVoices(srcStaff)) {
                  voice = true;
                  break;
                  }
            }
      if (! voice) {
            // force complete measures
            deselectAll();
            select(startMeasure, SelectType::RANGE, srcStaff);
            select(endMeasure, SelectType::RANGE, srcStaff);
            startSegment = selection().startSegment();
            endSegment = selection().endSegment();
            if (srcStaff == lastStaff - 1) {
                  // only one staff was selected up front - determine number of staves
                  // loop through all chords looking for maximum number of notes
                  int n = 0;
                  for (Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
                        Element* e = s->element(srcTrack);
                        if (e && e->type() == ElementType::CHORD) {
                              Chord* c = toChord(e);
                              n = qMax(n, int(c->notes().size()));
                              }
                        }
                  lastStaff = qMin(nstaves(), srcStaff + n);
                  }

            const QByteArray mimeData(selection().mimeData());
            // copy to all destination staves
            Segment* firstCRSegment = startMeasure->tick2segment(startMeasure->tick());
            for (int i = 1; srcStaff + i < lastStaff; ++i) {
                  int track = (srcStaff + i) * VOICES;
                  ChordRest* cr = toChordRest(firstCRSegment->element(track));
                  if (cr) {
                        XmlReader e(mimeData);
                        e.setPasteMode(true);
                        pasteStaff(e, cr->segment(), cr->staffIdx());
                        }
                  }

            // loop through each staff removing all but one note from each chord
            for (int i = 0; srcStaff + i < lastStaff; ++i) {
                  int track = (srcStaff + i) * VOICES;
                  for (Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
                        Element* e = s->element(track);
                        if (e && e->type() == ElementType::CHORD) {
                              Chord* c = toChord(e);
                              std::vector<Note*> notes = c->notes();
                              int nnotes = int(notes.size());
                              // keep note "i" from top, which is backwards from nnotes - 1
                              // reuse notes if there are more instruments than notes
                              int stavesPerNote = qMax((lastStaff - srcStaff) / nnotes, 1);
                              int keepIndex = qMax(nnotes - 1 - (i / stavesPerNote), 0);
                              Note* keepNote = c->notes()[keepIndex];
                              foreach (Note* n, notes) {
                                    if (n != keepNote)
                                          undoRemoveElement(n);
                                    }
                              }
                        }
                  }
            }
      else {
            int sTracks[VOICES];
            int dTracks[VOICES];
            if (srcStaff == lastStaff - 1)
                  lastStaff = qMin(nstaves(), srcStaff + VOICES);

            for (int i = 0; i < VOICES; i++) {
                  sTracks[i] = -1;
                  dTracks[i] = -1;
                  }
            int full = 0;

            for (Segment* seg = startSegment; seg && seg->tick() < lTick; seg = seg->next1()) {
                  for (int i = srcTrack; i < srcTrack + VOICES && full != VOICES; i ++) {
                        bool t = true;
                        for (int j = 0; j < VOICES; j++) {
                              if (i == sTracks[j]) {
                                    t = false;
                                    break;
                                    }
                              }

                        if (!seg->measure()->hasVoice(i) || seg->measure()->isOnlyRests(i) || !t)
                              continue;
                        sTracks[full] = i;

                        for(int j = srcTrack + full * VOICES; j < lastStaff * VOICES; j++) {
                              if (i == j) {
                                    dTracks[full] = j;
                                    break;
                                    }
                              for(Measure* m = seg->measure(); m && m->tick() < lTick; m = m->nextMeasure()) {
                                    if (!m->hasVoice(j) || (m->hasVoice(j) && m->isOnlyRests(j)))
                                          dTracks[full] = j;
                                    else {
                                          dTracks[full] = -1;
                                          break;
                                          }
                                    }
                              if (dTracks[full] != -1)
                                    break;
                              }
                        full++;
                        }
                  }

            for (int i = srcTrack, j = 0; i < lastStaff * VOICES && j < VOICES ; i += VOICES, j++) {
                  int strack = sTracks[j % VOICES];
                  int dtrack = dTracks[j % VOICES];
                  if (strack != -1 && strack != dtrack && dtrack != -1)
                        undo(new CloneVoice(startSegment, lTick, startSegment, strack, dtrack, -1, false));
                  }
            }

      // select exploded region
      deselectAll();
      select(startMeasure, SelectType::RANGE, srcStaff);
      select(endMeasure, SelectType::RANGE, lastStaff - 1);
      }

//---------------------------------------------------------
//   cmdImplode
///   implodes contents of selected staves into top staff
///   for single staff, merge voices
//---------------------------------------------------------

void Score::cmdImplode()
      {
      if (!selection().isRange())
            return;

      int dstStaff   = selection().staffStart();
      int endStaff   = selection().staffEnd();
      int dstTrack   = dstStaff * VOICES;
      int startTrack = dstStaff * VOICES;
      int endTrack   = endStaff * VOICES;
      Segment* startSegment = selection().startSegment();
      Segment* endSegment = selection().endSegment();
      Measure* startMeasure = startSegment->measure();
      Measure* endMeasure = endSegment ? endSegment->measure() : lastMeasure();
      int startTick       = startSegment->tick();
      int endTick         = endSegment ? endSegment->tick() : lastMeasure()->endTick();

      // if single staff selected, combine voices
      // otherwise combine staves
      if (dstStaff == endStaff - 1) {

            // loop through segments adding notes to chord on top staff
            for (Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
                  if (!s->isChordRestType())
                        continue;
                  Element* dst = s->element(dstTrack);
                  if (dst && dst->isChord()) {
                        Chord* dstChord = toChord(dst);
                        // see if we are tying in to this chord
                        Chord* tied = 0;
                        for (Note* n : dstChord->notes()) {
                              if (n->tieBack()) {
                                    tied = n->tieBack()->startNote()->chord();
                                    break;
                                    }
                              }
                        // loop through each subsequent staff (or track within staff)
                        // looking for notes to add
                        for (int srcTrack = startTrack + 1; srcTrack < endTrack; srcTrack++) {
                              Element* src = s->element(srcTrack);
                              if (src && src->isChord()) {
                                    Chord* srcChord = toChord(src);
                                    // when combining voices, skip if not same duration
                                    if (srcChord->duration() != dstChord->duration())
                                          continue;
                                    // add notes
                                    for (Note* n : srcChord->notes()) {
                                          NoteVal nv(n->pitch());
                                          nv.tpc1 = n->tpc1();
                                          // skip duplicates
                                          if (dstChord->findNote(nv.pitch))
                                                continue;
                                          Note* nn = addNote(dstChord, nv);
                                          // add tie to this note if original chord was tied
                                          if (tied) {
                                                // find note to tie to
                                                for (Note *tn : tied->notes()) {
                                                      if (nn->pitch() == tn->pitch() && nn->tpc() == tn->tpc() && !tn->tieFor()) {
                                                            // found note to tie
                                                            Tie* tie = new Tie(this);
                                                            tie->setStartNote(tn);
                                                            tie->setEndNote(nn);
                                                            tie->setTrack(tn->track());
                                                            undoAddElement(tie);
                                                            }
                                                      }
                                                }
                                          }
                                    }
                              // delete chordrest from source track if possible
                              if (src && src->voice())
                                    undoRemoveElement(src);
                              }
                        }
                  // TODO - use first voice that actually has a note and implode remaining voices on it?
                  // see https://musescore.org/en/node/174111
                  else if (dst) {
                        // destination track has something, but it isn't a chord
                        // remove rests from other voices if in "voice mode"
                        for (int i = 1; i < VOICES; ++i) {
                              Element* e = s->element(dstTrack + i);
                              if (e && e->isRest())
                                    undoRemoveElement(e);
                              }
                        }
                  }
            // delete orphaned spanners (TODO: figure out solution to reconnect orphaned spanners to their cloned notes)
            checkSpanner(startTick, endTick);
            }
      else {
            int tracks[VOICES];
            for (int i = 0; i < VOICES; i++)
                  tracks[i] = -1;
            int full = 0;

            // identify tracks to combine, storing the source track numbers in tracks[]
            // first four non-empty tracks to win
            for (int track = startTrack; track < endTrack && full < VOICES; ++track) {
                  for (Measure* m = startMeasure; m && m != endMeasure; m = m->nextMeasure()) {
                        if (m->hasVoice(track) && !m->isOnlyRests(track)) {
                              tracks[full++] = track;
                              break;
                              }
                        }
                  }

            // clone source tracks into destination
            for (int i = dstTrack; i < dstTrack + VOICES; i++) {
                  int strack = tracks[i % VOICES];
                  if (strack != -1 && strack != i) {
                        undo( new CloneVoice(startSegment, endTick, startSegment, strack, i, i, false));
                        }
                  }
            }

      // select destination staff only
      deselectAll();
      select(startMeasure, SelectType::RANGE, dstStaff);
      select(endMeasure, SelectType::RANGE, dstStaff);
      }

//---------------------------------------------------------
//   cmdSlashFill
///   fills selected region with slashes
//---------------------------------------------------------

void Score::cmdSlashFill()
      {
      int startStaff = selection().staffStart();
      int endStaff = selection().staffEnd();
      Segment* startSegment = selection().startSegment();
      if (!startSegment) // empty score?
            return;

      Segment* endSegment = selection().endSegment();
      int endTick = endSegment ? endSegment->tick() : lastSegment()->tick() + 1;
      Chord* firstSlash = 0;
      Chord* lastSlash = 0;

      // loop through staves in selection
      for (int staffIdx = startStaff; staffIdx < endStaff; ++staffIdx) {
            int track = staffIdx * VOICES;
            int voice = -1;
            // loop through segments adding slashes on each beat
            for (Segment* s = startSegment; s && s->tick() < endTick; s = s->next1()) {
                  if (s->segmentType() != SegmentType::ChordRest)
                        continue;
                  // determine beat type based on time signature
                  int d = s->measure()->timesig().denominator();
                  int n = (d > 4 && s->measure()->timesig().numerator() % 3 == 0) ? 3 : 1;
                  Fraction f(n, d);
                  // skip over any leading segments before next (first) beat
                  if (s->rtick() % f.ticks())
                        continue;
                  // determine voice to use - first available voice for this measure / staff
                  if (voice == -1 || s->rtick() == 0) {
                        bool needGap[VOICES];
                        for (voice = 0; voice < VOICES; ++voice) {
                              needGap[voice] = false;
                              ChordRest* cr = toChordRest(s->element(track + voice));
                              // no chordrest == treat as ordinary rest for purpose of determining availbility of voice
                              // but also, we will need to make a gap for this voice if we do end up choosing it
                              if (!cr)
                                    needGap[voice] = true;
                              // chord == keep looking for an available voice
                              else if (cr->type() == ElementType::CHORD)
                                    continue;
                              // full measure rest == OK to use voice
                              else if (cr->durationType() == TDuration::DurationType::V_MEASURE)
                                    break;
                              // no chordrest or ordinary rest == OK to use voice
                              // if there are nothing but rests for duration of measure / selection
                              bool ok = true;
                              for (Segment* ns = s->next(SegmentType::ChordRest); ns && ns != endSegment; ns = ns->next(SegmentType::ChordRest)) {
                                    ChordRest* ncr = toChordRest(ns->element(track + voice));
                                    if (ncr && ncr->type() == ElementType::CHORD) {
                                          ok = false;
                                          break;
                                          }
                                    }
                              if (ok)
                                    break;
                              }
                        // no available voices, just use voice 0
                        if (voice == VOICES)
                              voice = 0;
                        // no cr was found in segment for this voice, so make gap
                        if (needGap[voice])
                              makeGapVoice(s, track + voice, f, s->tick());
                        }
                  // construct note
                  int line = 0;
                  bool error = false;
                  NoteVal nv;
                  if (staff(staffIdx)->staffType(s->tick())->group() == StaffGroup::TAB)
                        line = staff(staffIdx)->lines(s->tick()) / 2;
                  else
                        line = staff(staffIdx)->middleLine(s->tick());     // staff(staffIdx)->lines() - 1;
                  if (staff(staffIdx)->staffType(s->tick())->group() == StaffGroup::PERCUSSION) {
                        nv.pitch = 0;
                        nv.headGroup = NoteHead::Group::HEAD_SLASH;
                        }
                  else {
                        Position p;
                        p.segment = s;
                        p.staffIdx = staffIdx;
                        p.line = line;
                        p.fret = FRET_NONE;
                        _is.setRest(false);     // needed for tab
                        nv = noteValForPosition(p, error);
                        }
                  if (error)
                        continue;
                  // insert & turn into slash
                  s = setNoteRest(s, track + voice, nv, f);
                  Chord* c = toChord(s->element(track + voice));
                  if (c->links()) {
                        for (ScoreElement* e : *c->links()) {
                              Chord* lc = toChord(e);
                              lc->setSlash(true, true);
                              }
                        }
                  else
                        c->setSlash(true, true);
                  lastSlash = c;
                  if (!firstSlash)
                        firstSlash = c;
                  }
            }

      // re-select the slashes
      deselectAll();
      if (firstSlash && lastSlash) {
            select(firstSlash, SelectType::RANGE);
            select(lastSlash, SelectType::RANGE);
            }
      }

//---------------------------------------------------------
//   cmdSlashRhythm
///   converts rhythms in selected region to slashes
//---------------------------------------------------------

void Score::cmdSlashRhythm()
      {
      QList<Chord*> chords;
      // loop through all notes in selection
      foreach (Element* e, selection().elements()) {
            if (e->voice() >= 2 && e->isRest()) {
                  Rest* r = toRest(e);
                  if (r->links()) {
                        for (ScoreElement* se : *r->links()) {
                              Rest* lr = toRest(se);
                              lr->setAccent(!lr->accent());
                              }
                        }
                  else
                        r->setAccent(!r->accent());
                  continue;
                  }
            else if (e->isNote()) {
                  Note* n = toNote(e);
                  if (n->noteType() != NoteType::NORMAL)
                        continue;
                  Chord* c = n->chord();
                  // check for duplicates (chords with multiple notes)
                  if (chords.contains(c))
                        continue;
                  chords.append(c);
                  // toggle slash setting
                  if (c->links()) {
                        for (ScoreElement* se : *c->links()) {
                              Chord* lc = toChord(se);
                              lc->setSlash(!lc->slash(), false);
                              }
                        }
                  else
                        c->setSlash(!c->slash(), false);
                  }
            }
      }

//---------------------------------------------------------
//   cmdResequenceRehearsalMarks
///   resequences rehearsal marks within a range selection
///   or, if nothing is selected, the entire score
//---------------------------------------------------------

void Score::cmdResequenceRehearsalMarks()
      {
      bool noSelection = !selection().isRange();

      if (noSelection)
            cmdSelectAll();
      else if (!selection().isRange())
            return;

      RehearsalMark* last = 0;
      for (Segment* s = selection().startSegment(); s && s != selection().endSegment(); s = s->next1()) {
            for (Element* e : s->annotations()) {
                  if (e->type() == ElementType::REHEARSAL_MARK) {
                        RehearsalMark* rm = toRehearsalMark(e);
                        if (last) {
                              QString rmText = nextRehearsalMarkText(last, rm);
                              for (ScoreElement* le : rm->linkList())
                                    le->undoChangeProperty(Pid::TEXT, rmText);
                              }
                        last = rm;
                        }
                  }
            }

      if (noSelection)
            deselectAll();
      }

//---------------------------------------------------------
//   addRemoveBreaks
//    interval lock
//    0        false    remove all linebreaks
//    > 0      false    add linebreak every interval measure
//    d.c.     true     add linebreak at every system end
//---------------------------------------------------------

void Score::addRemoveBreaks(int interval, bool lock)
      {
      Segment* startSegment = selection().startSegment();
      if (!startSegment) // empty score?
            return;
      Segment* endSegment   = selection().endSegment();
      Measure* startMeasure = startSegment->measure();
      Measure* endMeasure   = endSegment ? endSegment->measure() : lastMeasureMM();
      Measure* lastMeasure  = lastMeasureMM();

      // loop through measures in selection
      // count mmrests as a single measure
      int count = 0;
      for (Measure* mm = startMeasure; mm; mm = mm->nextMeasureMM()) {

            // even though we are counting mmrests as a single measure,
            // we need to find last real measure within mmrest for the actual break
            Measure* m = mm->isMMRest() ? mm->mmRestLast() : mm;

            if (lock) {
                  // skip last measure of score
                  if (mm == lastMeasure)
                        break;
                  // skip if it already has a break
                  if (m->lineBreak() || m->pageBreak())
                        continue;
                  // add break if last measure of system
                  if (mm->system() && mm->system()->lastMeasure() == mm)
                        m->undoSetLineBreak(true);
                  }
            else {
                  if (interval == 0) {
                        // remove line break if present
                        if (m->lineBreak())
                              m->undoSetLineBreak(false);
                        }
                  else {
                        if (++count == interval) {
                              // skip last measure of score
                              if (mm == lastMeasure)
                                    break;
                              // found place for break; add if not already one present
                              if (!(m->lineBreak() || m->pageBreak()))
                                    m->undoSetLineBreak(true);
                              // reset count
                              count = 0;
                              }
                        else if (m->lineBreak()) {
                              // remove line break if present in wrong place
                              m->undoSetLineBreak(false);
                              }
                        }
                  }

            if (mm == endMeasure)
                  break;
            }

      }

//---------------------------------------------------------
//   cmdRemoveEmptyTrailingMeasures
//---------------------------------------------------------

void Score::cmdRemoveEmptyTrailingMeasures()
      {
      MasterScore* score = masterScore();
      Measure* firstMeasure;
      Measure* lastMeasure = score->lastMeasure();
      if (!lastMeasure || !lastMeasure->isFullMeasureRest())
            return;
      firstMeasure = lastMeasure;
      for (firstMeasure = lastMeasure;;) {
            Measure* m = firstMeasure->prevMeasure();
            if (!m || !m->isFullMeasureRest())
                  break;
            firstMeasure = m;
            }
      deleteMeasures(firstMeasure, lastMeasure);
      }

//---------------------------------------------------------
//   cmdPitchUp
//---------------------------------------------------------

void Score::cmdPitchUp()
      {
      Element* el = selection().element();
      if (el && el->isLyrics())
            cmdMoveLyrics(toLyrics(el), Direction::UP);
      else if (el && (el->isArticulation() || el->isTextBase()))
            el->undoChangeProperty(Pid::OFFSET, el->offset() + QPointF(0.0, -MScore::nudgeStep * el->spatium()));
      else if (el && el->isRest())
            cmdMoveRest(toRest(el), Direction::UP);
      else
            upDown(true, UpDownMode::CHROMATIC);
      }

//---------------------------------------------------------
//   cmdPitchDown
//---------------------------------------------------------

void Score::cmdPitchDown()
      {
      Element* el = selection().element();
      if (el && el->isLyrics())
            cmdMoveLyrics(toLyrics(el), Direction::DOWN);
      else if (el && (el->isArticulation() || el->isTextBase()))
            el->undoChangeProperty(Pid::OFFSET, el->offset() + QPointF(0.0, MScore::nudgeStep * el->spatium()));
      else if (el && el->isRest())
            cmdMoveRest(toRest(el), Direction::DOWN);
      else
            upDown(false, UpDownMode::CHROMATIC);
      }

//---------------------------------------------------------
//   cmdTimeDelete
//---------------------------------------------------------

void Score::cmdTimeDelete()
      {
      Element* e = selection().element();
      if (e && e->isBarLine() && toBarLine(e)->segment()->isEndBarLineType()) {
            Measure* m = toBarLine(e)->segment()->measure();
            cmdJoinMeasure(m, m->nextMeasure());
            }
      else {
            if (_is.insertMode())
                  globalTimeDelete();
            else
                  localTimeDelete();
            }
      }

//---------------------------------------------------------
//   cmdPitchUpOctave
//---------------------------------------------------------

void Score::cmdPitchUpOctave()
      {
      Element* el = selection().element();
      if (el && (el->isArticulation() || el->isTextBase()))
            el->undoChangeProperty(Pid::OFFSET, el->offset() + QPointF(0.0, -MScore::nudgeStep10 * el->spatium()), PropertyFlags::UNSTYLED);
      else
            upDown(true, UpDownMode::OCTAVE);
      }

//---------------------------------------------------------
//   cmdPitchDownOctave
//---------------------------------------------------------

void Score::cmdPitchDownOctave()
      {
      Element* el = selection().element();
      if (el && (el->isArticulation() || el->isTextBase()))
            el->undoChangeProperty(Pid::OFFSET, el->offset() + QPointF(0.0, MScore::nudgeStep10 * el->spatium()), PropertyFlags::UNSTYLED);
      else
            upDown(false, UpDownMode::OCTAVE);
      }

/*//---------------------------------------------------------
//   cmdnextElement
//---------------------------------------------------------

void Score::cmdNextElement()
      {
      Element* el = selection().element();
            if (!el && !selection().elements().isEmpty() )
                el = selection().elements().first();

            if (el){
                  Element* next = nextElement();
                  int staffId = el->staffIdx();
                  selectSingle(next, staffId);
                  }
            else
                  selectSingle(score()->firstElement(), 0); // check staffId

      }

//---------------------------------------------------------
//   cmdprevElement
//---------------------------------------------------------

void Score::cmdPrevElement()
      {
      Element* el = selection().element();
            if (!el && !selection().elements().isEmpty() )
                el = selection().elements().last();

            if (el){
                  Element* prev = prevElement();
                  int staffId = el->staffIdx();
                  selectSingle(prev, staffId);
                  }
            else
                  selectSingle(score()->lastElement(), 0); // check staffId

      }
*/
//---------------------------------------------------------
//   cmdPadNoteInclreaseTAB
//---------------------------------------------------------

void Score::cmdPadNoteIncreaseTAB()
      {
      switch (_is.duration().type() ) {
// cycle back from longest to shortest?
//          case TDuration::V_LONG:
//                padToggle(Pad::NOTE128);
//                break;
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

//---------------------------------------------------------
//   cmdPadNoteDecreaseTAB
//---------------------------------------------------------

void Score::cmdPadNoteDecreaseTAB()
      {
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
//          case TDuration::DurationType::V_128TH:
//                padToggle(Pad::NOTE00);
//                break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   cmdToggleLayoutBreak
//---------------------------------------------------------

void Score::cmdToggleLayoutBreak(LayoutBreak::Type type)
      {
      // find measure(s)
      QList<MeasureBase*> mbl;
      if (selection().isRange()) {
            Measure* startMeasure = nullptr;
            Measure* endMeasure = nullptr;
            if (!selection().measureRange(&startMeasure, &endMeasure))
                  return;
            if (!startMeasure || !endMeasure)
                  return;
#if 1
            // toggle break on the last measure of the range
            mbl.append(endMeasure);
            // if more than one measure selected,
            // also toggle break *before* the range (to try to fit selection on a single line)
            if (startMeasure != endMeasure && startMeasure->prev())
                  mbl.append(startMeasure->prev());
#else
            // toggle breaks throughout the selection
            for (Measure* m = startMeasure; m; m = m->nextMeasure()) {
                  mbl.append(m);
                  if (m == endMeasure)
                        break;
                  }
#endif
            }
      else {
            MeasureBase* mb = nullptr;
            for (Element* el : selection().elements()) {
                  switch (el->type()) {
                        case ElementType::HBOX:
                        case ElementType::VBOX:
                        case ElementType::TBOX:
                              mb = toMeasureBase(el);
                              break;
                        default: {
                              // find measure
                              Measure* measure = toMeasure(el->findMeasure());
                              // if measure is mmrest, then propagate to last original measure
                              if (measure)
                                    mb = measure->isMMRest() ? measure->mmRestLast() : measure;
                              }
                        }
                  }
                  if (mb)
                        mbl.append(mb);
            }
      // toggle the breaks
      for (MeasureBase* mb: mbl) {
            if (mb) {
                  bool val = false;
                  switch (type) {
                        case LayoutBreak::Type::LINE:
                              val = !mb->lineBreak();
                              mb->undoSetBreak(val, type);
                              // remove page break if appropriate
                              if (val && mb->pageBreak())
                                    mb->undoSetBreak(false, LayoutBreak::Type::PAGE);
                              break;
                        case LayoutBreak::Type::PAGE:
                              val = !mb->pageBreak();
                              mb->undoSetBreak(val, type);
                              // remove line break if appropriate
                              if (val && mb->lineBreak())
                                    mb->undoSetBreak(false, LayoutBreak::Type::LINE);
                              break;
                        case LayoutBreak::Type::SECTION:
                              val = !mb->sectionBreak();
                              mb->undoSetBreak(val, type);
                              break;
                        default:
                              break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   cmdToggleMmrest
//---------------------------------------------------------

void Score::cmdToggleMmrest()
      {
      bool val = !styleB(Sid::createMultiMeasureRests);
      deselectAll();
      undo(new ChangeStyleVal(this, Sid::createMultiMeasureRests, val));
      }

//---------------------------------------------------------
//   cmdToggleHideEmpty
//---------------------------------------------------------

void Score::cmdToggleHideEmpty()
      {
      bool val = !styleB(Sid::hideEmptyStaves);
      deselectAll();
      undo(new ChangeStyleVal(this, Sid::hideEmptyStaves, val));
      }

//---------------------------------------------------------
//   cmdSetVisible
//---------------------------------------------------------

void Score::cmdSetVisible()
      {
      for (Element* e : selection().elements())
            undo(new ChangeProperty(e, Pid::VISIBLE, true));
      }

//---------------------------------------------------------
//   cmdUnsetVisible
//---------------------------------------------------------

void Score::cmdUnsetVisible()
      {
      for (Element* e : selection().elements())
            undo(new ChangeProperty(e, Pid::VISIBLE, false));
      }

//---------------------------------------------------------
//   cmdAddPitch
///   insert note or add note to chord
//    c d e f g a b entered:
//---------------------------------------------------------

void Score::cmdAddPitch(const EditData& ed, int note, bool addFlag, bool insert)
      {
      InputState& is = inputState();
      if (is.track() == -1)          // invalid state
            return;
      if (is.segment() == 0) {
            qDebug("cannot enter notes here (no chord rest at current position)");
            return;
            }
      is.setRest(false);
      const Drumset* ds = is.drumset();
      int octave = 4;
      if (ds) {
            char note1 = "CDEFGAB"[note];
            int pitch = -1;
            int voice = 0;
            for (int i = 0; i < 127; ++i) {
                  if (!ds->isValid(i))
                        continue;
                  if (ds->shortcut(i) && (ds->shortcut(i) == note1)) {
                        pitch = i;
                        voice = ds->voice(i);
                        break;
                        }
                  }
            if (pitch == -1) {
                  qDebug("  shortcut %c not defined in drumset", note1);
                  return;
                  }
            is.setDrumNote(pitch);
            is.setTrack((is.track() / VOICES) * VOICES + voice);
            octave = pitch / 12;
            if (is.segment()) {
                  Segment* seg = is.segment();
                  while (seg) {
                        if (seg->element(is.track()))
                              break;
                        seg = seg->prev(SegmentType::ChordRest);
                        }
                  if (seg)
                        is.setSegment(seg);
                  else
                        is.setSegment(is.segment()->measure()->first(SegmentType::ChordRest));
                  }
            }
      else {
            static const int tab[] = { 0, 2, 4, 5, 7, 9, 11 };

            // if adding notes, add above the upNote of the current chord
            Element* el = selection().element();
            if (addFlag && el && el->isNote()) {
                  Chord* chord = toNote(el)->chord();
                  Note* n      = chord->upNote();
                  octave = n->epitch() / 12;
                  int tpc = n->tpc();
                  if (tpc == Tpc::TPC_C_BB || tpc == Tpc::TPC_C_B)
                        ++octave;
                  else if (tpc == Tpc::TPC_B_S || tpc == Tpc::TPC_B_SS)
                        --octave;
                  if (note <= tpc2step(tpc))
                        octave++;
                  }
            else {
                  int curPitch = 60;
                  if (is.segment()) {
                        Staff* staff = Score::staff(is.track() / VOICES);
                        Segment* seg = is.segment()->prev1(SegmentType::ChordRest | SegmentType::Clef | SegmentType::HeaderClef);
                        while (seg) {
                              if (seg->isChordRestType()) {
                                    Element* p = seg->element(is.track());
                                    if (p && p->isChord()) {
                                          curPitch = toChord(p)->downNote()->epitch();
                                          break;
                                          }
                                    }
                              else if (seg->isClefType() || seg->isHeaderClefType()) {
                                    Element* p = seg->element( (is.track() / VOICES) * VOICES); // clef on voice 1
                                    if (p && p->isClef()) {
                                          Clef* clef = toClef(p);
                                          // check if it's an actual change or just a courtesy
                                          ClefType ctb = staff->clef(clef->tick() - 1);
                                          if (ctb != clef->clefType() || clef->tick() == 0) {
                                                curPitch = line2pitch(4, clef->clefType(), Key::C); // C 72 for treble clef
                                                break;
                                                }
                                          }
                                    }
                              seg = seg->prev1MM(SegmentType::ChordRest | SegmentType::Clef);
                              }
                        octave = curPitch / 12;
                        }

                  int delta = octave * 12 + tab[note] - curPitch;
                  if (delta > 6)
                        --octave;
                  else if (delta < -6)
                        ++octave;
                  }
            }
      ed.view->startNoteEntryMode();

      int step = octave * 7 + note;
      cmdAddPitch(step,  addFlag, insert);
      ed.view->adjustCanvasPosition(is.cr(), false);
      }

void Score::cmdAddPitch(int step, bool addFlag, bool insert)
      {
      insert = insert || inputState().usingNoteEntryMethod(NoteEntryMethod::TIMEWISE);
      Position pos;
      if (addFlag) {
            Element* el = selection().element();
            if (el && el->isNote()) {
                  Note* selectedNote = toNote(el);
                  Chord* chord  = selectedNote->chord();
                  Segment* seg  = chord->segment();
                  pos.segment   = seg;
                  pos.staffIdx  = selectedNote->track() / VOICES;
                  ClefType clef = staff(pos.staffIdx)->clef(seg->tick());
                  pos.line      = relStep(step, clef);
                  bool error;
                  NoteVal nval = noteValForPosition(pos, error);
                  if (error)
                        return;
                  addNote(chord, nval);
                  return;
                  }
            }

      pos.segment   = inputState().segment();
      pos.staffIdx  = inputState().track() / VOICES;
      ClefType clef = staff(pos.staffIdx)->clef(pos.segment->tick());
      pos.line      = relStep(step, clef);

      if (inputState().usingNoteEntryMethod(NoteEntryMethod::REPITCH))
            repitchNote(pos, !addFlag);
      else {
            if (insert)
                  insertChord(pos);
            else
                  putNote(pos, !addFlag);
            }
      }

//---------------------------------------------------------
//   cmdToggleVisible
//---------------------------------------------------------

void Score::cmdToggleVisible()
      {
      QSet<Element*> spanners;
      for (Element* e : selection().elements()) {
            if (e->isBracket())     // ignore
                  continue;
            bool spannerSegment = e->isSpannerSegment();
            if (!spannerSegment || !spanners.contains(toSpannerSegment(e)->spanner()))
                  e->undoChangeProperty(Pid::VISIBLE, !e->getProperty(Pid::VISIBLE).toBool());
            if (spannerSegment)
                  spanners.insert(toSpannerSegment(e)->spanner());
            }
      }

//---------------------------------------------------------
//   cmdAddFret
///   insert note with given fret on current string
//---------------------------------------------------------

void Score::cmdAddFret(int fret)
      {
      InputState& is = inputState();
      if (is.track() == -1)                     // invalid state
            return;
      if (!is.segment()) {
            qDebug("cannot enter notes here (no chord rest at current position)");
            return;
            }
      Position pos;
      pos.segment   = is.segment();
      pos.staffIdx  = is.track() / VOICES;
      pos.line      = is.string();
      pos.fret      = fret;
      putNote(pos, false);
      }

//---------------------------------------------------------
//   cmdRelayout
//---------------------------------------------------------

void Score::cmdRelayout()
      {
      setLayoutAll();
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void Score::cmd(const QAction* a, EditData& ed)
      {
      QString cmd(a ? a->data().toString() : "");
      if (MScore::debugMode)
            qDebug("<%s>", qPrintable(cmd));

      struct ScoreCmd {
            const char* name;
            std::function<void()> cmd;
            };
      const std::vector<ScoreCmd> cmdList {
            { "note-c",                     [this,ed]{ cmdAddPitch(ed, 0, false, false);                        }},
            { "note-d",                     [this,ed]{ cmdAddPitch(ed, 1, false, false);                        }},
            { "note-e",                     [this,ed]{ cmdAddPitch(ed, 2, false, false);                        }},
            { "note-f",                     [this,ed]{ cmdAddPitch(ed, 3, false, false);                        }},
            { "note-g",                     [this,ed]{ cmdAddPitch(ed, 4, false, false);                        }},
            { "note-a",                     [this,ed]{ cmdAddPitch(ed, 5, false, false);                        }},
            { "note-b",                     [this,ed]{ cmdAddPitch(ed, 6, false, false);                        }},
            { "chord-c",                    [this,ed]{ cmdAddPitch(ed, 0, true, false);                         }},
            { "chord-d",                    [this,ed]{ cmdAddPitch(ed, 1, true, false);                         }},
            { "chord-e",                    [this,ed]{ cmdAddPitch(ed, 2, true, false);                         }},
            { "chord-f",                    [this,ed]{ cmdAddPitch(ed, 3, true, false);                         }},
            { "chord-g",                    [this,ed]{ cmdAddPitch(ed, 4, true, false);                         }},
            { "chord-a",                    [this,ed]{ cmdAddPitch(ed, 5, true, false);                         }},
            { "chord-b",                    [this,ed]{ cmdAddPitch(ed, 6, true, false);                         }},
            { "insert-c",                   [this,ed]{ cmdAddPitch(ed, 0, false, true);                         }},
            { "insert-d",                   [this,ed]{ cmdAddPitch(ed, 1, false, true);                         }},
            { "insert-e",                   [this,ed]{ cmdAddPitch(ed, 2, false, true);                         }},
            { "insert-f",                   [this,ed]{ cmdAddPitch(ed, 3, false, true);                         }},
            { "insert-g",                   [this,ed]{ cmdAddPitch(ed, 4, false, true);                         }},
            { "insert-a",                   [this,ed]{ cmdAddPitch(ed, 5, false, true);                         }},
            { "insert-b",                   [this,ed]{ cmdAddPitch(ed, 6, false, true);                         }},
            { "fret-0",                     [this]{ cmdAddFret(0);                                              }},
            { "fret-1",                     [this]{ cmdAddFret(1);                                              }},
            { "fret-2",                     [this]{ cmdAddFret(2);                                              }},
            { "fret-3",                     [this]{ cmdAddFret(3);                                              }},
            { "fret-4",                     [this]{ cmdAddFret(4);                                              }},
            { "fret-5",                     [this]{ cmdAddFret(5);                                              }},
            { "fret-6",                     [this]{ cmdAddFret(6);                                              }},
            { "fret-7",                     [this]{ cmdAddFret(7);                                              }},
            { "fret-8",                     [this]{ cmdAddFret(8);                                              }},
            { "fret-9",                     [this]{ cmdAddFret(9);                                              }},
            { "fret-10",                    [this]{ cmdAddFret(10);                                             }},
            { "fret-11",                    [this]{ cmdAddFret(11);                                             }},
            { "fret-12",                    [this]{ cmdAddFret(12);                                             }},
            { "fret-13",                    [this]{ cmdAddFret(13);                                             }},
            { "fret-14",                    [this]{ cmdAddFret(14);                                             }},
            { "toggle-visible",             [this]{ cmdToggleVisible();                                         }},
            { "reset-stretch",              [this]{ resetUserStretch();                                         }},
            { "mirror-note",                [this]{ cmdMirrorNoteHead();                                        }},
            { "double-duration",            [this]{ cmdDoubleDuration();                                        }},
            { "half-duration",              [this]{ cmdHalfDuration();                                          }},
            { "inc-duration-dotted",        [this]{ cmdIncDurationDotted();                                     }},
            { "dec-duration-dotted",        [this]{ cmdDecDurationDotted();                                     }},
            { "add-staccato",               [this]{ addArticulation(SymId::articStaccatoAbove);                 }},
            { "add-tenuto",                 [this]{ addArticulation(SymId::articTenutoAbove);                   }},
            { "add-marcato",                [this]{ addArticulation(SymId::articMarcatoAbove);                  }},
            { "add-sforzato",               [this]{ addArticulation(SymId::articAccentAbove);                   }},
            { "add-trill",                  [this]{ addArticulation(SymId::ornamentTrill);                      }},
            { "add-up-bow",                 [this]{ addArticulation(SymId::stringsUpBow);                       }},
            { "add-down-bow",               [this]{ addArticulation(SymId::stringsDownBow);                     }},
            { "add-8va",                    [this]{ cmdAddOttava(OttavaType::OTTAVA_8VA);                       }},
            { "add-8vb",                    [this]{ cmdAddOttava(OttavaType::OTTAVA_8VB);                       }},
            { "note-longa",                 [this]{ padToggle(Pad::NOTE00);                                     }},
            { "note-longa-TAB",             [this]{ padToggle(Pad::NOTE00);                                     }},
            { "note-breve",                 [this]{ padToggle(Pad::NOTE0);                                      }},
            { "note-breve-TAB",             [this]{ padToggle(Pad::NOTE0);                                      }},
            { "pad-note-1",                 [this]{ padToggle(Pad::NOTE1);                                      }},
            { "pad-note-1-TAB",             [this]{ padToggle(Pad::NOTE1);                                      }},
            { "pad-note-2",                 [this]{ padToggle(Pad::NOTE2);                                      }},
            { "pad-note-2-TAB",             [this]{ padToggle(Pad::NOTE2);                                      }},
            { "pad-note-4",                 [this]{ padToggle(Pad::NOTE4);                                      }},
            { "pad-note-4-TAB",             [this]{ padToggle(Pad::NOTE4);                                      }},
            { "pad-note-8",                 [this]{ padToggle(Pad::NOTE8);                                      }},
            { "pad-note-8-TAB",             [this]{ padToggle(Pad::NOTE8);                                      }},
            { "pad-note-16",                [this]{ padToggle(Pad::NOTE16);                                     }},
            { "pad-note-16-TAB",            [this]{ padToggle(Pad::NOTE16);                                     }},
            { "pad-note-32",                [this]{ padToggle(Pad::NOTE32);                                     }},
            { "pad-note-32-TAB",            [this]{ padToggle(Pad::NOTE32);                                     }},
            { "pad-note-64",                [this]{ padToggle(Pad::NOTE64);                                     }},
            { "pad-note-64-TAB",            [this]{ padToggle(Pad::NOTE64);                                     }},
            { "pad-note-128",               [this]{ padToggle(Pad::NOTE128);                                    }},
            { "pad-note-128-TAB",           [this]{ padToggle(Pad::NOTE128);                                    }},
            { "reset-style",                [this]{ cmdResetStyle();                                            }},
            { "reset-beammode",             [this]{ cmdResetBeamMode();                                         }},
            { "reset-groupings",            [this]{ cmdResetNoteAndRestGroupings();                             }},
            { "clef-violin",                [this]{ cmdInsertClef(ClefType::G);                                 }},
            { "clef-bass",                  [this]{ cmdInsertClef(ClefType::F);                                 }},
            { "voice-x12",                  [this]{ cmdExchangeVoice(0, 1);                                     }},
            { "voice-x13",                  [this]{ cmdExchangeVoice(0, 2);                                     }},
            { "voice-x14",                  [this]{ cmdExchangeVoice(0, 3);                                     }},
            { "voice-x23",                  [this]{ cmdExchangeVoice(1, 2);                                     }},
            { "voice-x24",                  [this]{ cmdExchangeVoice(1, 3);                                     }},
            { "voice-x34",                  [this]{ cmdExchangeVoice(2, 3);                                     }},
            { "pad-rest",                   [this]{ padToggle(Pad::REST);                                       }},
            { "pad-dot",                    [this]{ padToggle(Pad::DOT);                                        }},
            { "pad-dotdot",                 [this]{ padToggle(Pad::DOTDOT);                                     }},
            { "pad-dot3",                   [this]{ padToggle(Pad::DOT3);                                       }},
            { "pad-dot4",                   [this]{ padToggle(Pad::DOT4);                                       }},
            { "beam-start",                 [this]{ cmdSetBeamMode(Beam::Mode::BEGIN);                          }},
            { "beam-mid",                   [this]{ cmdSetBeamMode(Beam::Mode::MID);                            }},
            { "no-beam",                    [this]{ cmdSetBeamMode(Beam::Mode::NONE);                           }},
            { "beam-32",                    [this]{ cmdSetBeamMode(Beam::Mode::BEGIN32);                        }},
            { "sharp2",                     [this]{ changeAccidental(AccidentalType::SHARP2);                   }},
            { "sharp",                      [this]{ changeAccidental(AccidentalType::SHARP);                    }},
            { "nat",                        [this]{ changeAccidental(AccidentalType::NATURAL);                  }},
            { "flat",                       [this]{ changeAccidental(AccidentalType::FLAT);                     }},
            { "flat2",                      [this]{ changeAccidental(AccidentalType::FLAT2);                    }},
            { "flip",                       [this]{ cmdFlip();                                                  }},
            { "stretch+",                   [this]{ cmdAddStretch(0.1);                                         }},
            { "stretch-",                   [this]{ cmdAddStretch(-0.1);                                        }},
            { "pitch-spell",                [this]{ spell();                                                    }},
            { "select-all",                 [this]{ cmdSelectAll();                                             }},
            { "select-section",             [this]{ cmdSelectSection();                                         }},
            { "add-brackets",               [this]{ cmdAddBracket();                                            }},
            { "acciaccatura",               [this]{ cmdAddGrace(NoteType::ACCIACCATURA, MScore::division / 2);  }},
            { "appoggiatura",               [this]{ cmdAddGrace(NoteType::APPOGGIATURA, MScore::division / 2);  }},
            { "grace4",                     [this]{ cmdAddGrace(NoteType::GRACE4, MScore::division);            }},
            { "grace16",                    [this]{ cmdAddGrace(NoteType::GRACE16, MScore::division / 4);       }},
            { "grace32",                    [this]{ cmdAddGrace(NoteType::GRACE32, MScore::division / 8);       }},
            { "grace8after",                [this]{ cmdAddGrace(NoteType::GRACE8_AFTER, MScore::division / 2);  }},
            { "grace16after",               [this]{ cmdAddGrace(NoteType::GRACE16_AFTER, MScore::division / 4); }},
            { "grace32after",               [this]{ cmdAddGrace(NoteType::GRACE32_AFTER, MScore::division / 8); }},
            { "explode",                    [this]{ cmdExplode();                                               }},
            { "implode",                    [this]{ cmdImplode();                                               }},
            { "slash-fill",                 [this]{ cmdSlashFill();                                             }},
            { "slash-rhythm",               [this]{ cmdSlashRhythm();                                           }},
            { "resequence-rehearsal-marks", [this]{ cmdResequenceRehearsalMarks();                              }},
            { "del-empty-measures",         [this]{ cmdRemoveEmptyTrailingMeasures();                           }},
            { "add-audio",                  [this]{ addAudioTrack();                                            }},
            { "transpose-up",               [this]{ transposeSemitone(1);                                       }},
            { "transpose-down",             [this]{ transposeSemitone(-1);                                      }},
            { "delete",                     [this]{ cmdDeleteSelection();                                       }},
            { "full-measure-rest",          [this]{ cmdFullMeasureRest();                                       }},
            { "toggle-insert-mode",         [this]{ _is.setInsertMode(!_is.insertMode());                       }},
            { "pitch-up",                   [this]{ cmdPitchUp();                                               }},
            { "pitch-down",                 [this]{ cmdPitchDown();                                             }},
            { "time-delete",                [this]{ cmdTimeDelete();                                            }},
            { "pitch-up-octave",            [this]{ cmdPitchUpOctave();                                         }},
            { "pitch-down-octave",          [this]{ cmdPitchDownOctave();                                       }},
            { "pad-note-increase",          [this]{ cmdPadNoteIncreaseTAB();                                    }},
            { "pad-note-decrease",          [this]{ cmdPadNoteDecreaseTAB();                                    }},
            { "pad-note-increase-TAB",      [this]{ cmdPadNoteIncreaseTAB();                                    }},
            { "pad-note-decrease-TAB",      [this]{ cmdPadNoteDecreaseTAB();                                    }},
            { "toggle-mmrest",              [this]{ cmdToggleMmrest();                                          }},
            { "toggle-hide-empty",          [this]{ cmdToggleHideEmpty();                                       }},
            { "set-visible",                [this]{ cmdSetVisible();                                            }},
            { "unset-visible",              [this]{ cmdUnsetVisible();                                          }},
            { "system-break",               [this]{ cmdToggleLayoutBreak(LayoutBreak::Type::LINE);              }},
            { "page-break",                 [this]{ cmdToggleLayoutBreak(LayoutBreak::Type::PAGE);              }},
            { "section-break",              [this]{ cmdToggleLayoutBreak(LayoutBreak::Type::SECTION);           }},
            { "relayout",                   [this]{ cmdRelayout();                                              }},
            { "",                           [this]{                                                             }},
            };

      for (const auto& c : cmdList) {
            if (cmd == c.name) {
                  startCmd();
                  c.cmd();
                  endCmd();
                  return;
                  }
            }
      qDebug("unknown cmd <%s>", qPrintable(cmd));
      }


}

