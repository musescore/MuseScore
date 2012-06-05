//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/chordrest.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/note.h"
#include "libmscore/stafftext.h"
#include "libmscore/measure.h"
#include "libmscore/repeatlist.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/segment.h"
#include "cursor.h"

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

Cursor::Cursor(Score* s)
   : QObject(0)
      {
      _score                 = s;
      _track                 = 0;
      _expandRepeats         = false;
      _segment               = 0;
      _curRepeatSegment      = 0;
      _curRepeatSegmentIndex = 0;
      }

//---------------------------------------------------------
//   rewind
//---------------------------------------------------------

void Cursor::rewind(int type)
      {
      if (type == 0) {
            _segment   = 0;
            Measure* m = _score->firstMeasure();
            if (_expandRepeats && !_score->repeatList()->isEmpty()){
                  _curRepeatSegment = _score->repeatList()->first();
                  _curRepeatSegmentIndex = 0;
                  }
            if (m) {
                  _segment = m->first(SegChordRest);
                  while (_segment && _segment->element(_track) == 0)
                        _segment = _segment->next1(SegChordRest);
                  }
            }
      else if (type == 1) {
            _segment  = _score->selection().startSegment();
            _track    = _score->selection().staffStart() * VOICES;
            }
      else if (type == 2) {
            _segment  = _score->selection().endSegment();
            _track    = _score->selection().staffEnd() * VOICES;
            }
      }

//---------------------------------------------------------
//   next
//    go to next segment
//    return false if end of score is reached
//---------------------------------------------------------

bool Cursor::next()
      {
      if (!_segment)
            return false;
      Segment* seg = _segment->next1(SegChordRest | SegGrace);
      if (!seg) {
            _segment = 0;
            return  false;
            }
      RepeatSegment* rs = repeatSegment();
      if (rs && _expandRepeats){
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            if ((seg  && (seg->tick() >= endTick) ) || (!seg) ){
                  int rsIdx = repeatSegmentIndex();
                  rsIdx ++;
                  if (rsIdx < score()->repeatList()->size()){ //there is a next repeat segment
                        rs = score()->repeatList()->at(rsIdx);
                        setRepeatSegment(rs);
                        setRepeatSegmentIndex(rsIdx);
                        Measure* m = score()->tick2measure(rs->tick);
                        seg = m ? m->first(SegChordRest | SegGrace) : 0;
                        }
                  else
                        seg = 0;
                  }
            }
      _segment = seg;
      return _segment != 0;
      }

//---------------------------------------------------------
//   nextMeasure
//    go to first segment of next measure
//    return false if end of score is reached
//---------------------------------------------------------

bool Cursor::nextMeasure()
      {
      if (_segment == 0)
            return false;
      Measure* m = _segment->measure()->nextMeasure();
      RepeatSegment* rs = repeatSegment();
      if (rs && _expandRepeats){
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            if ((m  && (m->tick() + m->ticks() > endTick) ) || (!m) ){
                  int rsIdx = repeatSegmentIndex();
                  rsIdx ++;
                  if (rsIdx < score()->repeatList()->size()) {       //there is a next repeat segment
                        rs = score()->repeatList()->at(rsIdx);
                        setRepeatSegment(rs);
                        setRepeatSegmentIndex(rsIdx);
                        m = score()->tick2measure(rs->tick);
                        }
                  else{
                        m = 0;
                        }
                  }
            }

      if (m == 0) {
            _segment = 0;
            return false;
            }
      Segment* seg = m->first(SegChordRest | SegGrace);
      while (seg->element(_track) == 0)
            seg = seg->next1(SegChordRest | SegGrace);
      _segment = seg;
      return _segment != 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Cursor::add(Element* s)
      {
      if (!_segment)
            return;
      s->setTrack(_track);
      s->score()->startCmd();
      s->score()->undoAddElement(s);
      s->score()->setLayoutAll(true);
      s->score()->endCmd();
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

#if 0
void Cursor::add(ChordRest* c)
      {
      ChordRest* chordRest = cr();
      int track       = _staffIdx * VOICES + _voice;

      if (!chordRest) {
            if(_voice > 0) { //create rests
                int t = tick();
                //trick : go to the start if we don't have segment nor chord.
                if(t == _score->lastMeasure()->tick() + _score->lastMeasure()->ticks())
                      t = 0;
                Measure* measure = score()->tick2measure(t);
                SegmentType st = SegChordRest;
                Segment* seg = measure->findSegment(st, t);
                if (seg == 0) {
                      seg = new Segment(measure, st, t);
                      score()->undoAddElement(seg);
                      }
                chordRest = score()->addRest(seg, track, TDuration(TDuration::V_MEASURE), 0);
                }
            if (!chordRest) {
                  qDebug("Cursor::add: no cr\n");
                  return;
                  }
            }
      int tick = chordRest->tick();
      Fraction len(c->durationType().fraction());

      Fraction gap    = score()->makeGap(chordRest->segment(), chordRest->track(),
         len, chordRest->tuplet());
      if (gap < len) {
            qDebug("cannot make gap\n");
            return;
            }
      Measure* measure = score()->tick2measure(tick);
      SegmentType st = SegChordRest;
      Segment* seg = measure->findSegment(st, tick);
      if (seg == 0) {
            seg = new Segment(measure, st, tick);
            score()->undoAddElement(seg);
            }
      c->setScore(score());
      if (c->type() == CHORD) {
            foreach(Note* n, static_cast<Chord*>(c)->notes())
                  n->setScore(score());
            }

      setSegment(seg);
      c->setParent(seg);
      c->setTrack(track);
      score()->undoAddElement(c);
      }
#endif

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int Cursor::tick()
      {
      int offset = 0;
      RepeatSegment* rs = repeatSegment();
      if (rs && _expandRepeats)
            offset = rs->utick - rs->tick;
      if (_segment)
          return _segment->tick() + offset;
      else
          return _score->lastMeasure()->tick() + _score->lastMeasure()->ticks() + offset;  // end of score
      }

//---------------------------------------------------------
//   time
//---------------------------------------------------------

double Cursor::time()
      {
      return score()->utick2utime(tick()) * 1000;
      }

Element* Cursor::element() const
      {
      return _segment ? _segment->element(_track) : 0;
      }

void Cursor::setTrack(int v)
      {
      _track = v;
      int tracks = _score->nstaves() * VOICES;
      if (_track < 0)
            _track = 0;
      else if (_track >= tracks)
            _track = tracks - 1;
      }

void Cursor::setStaffIdx(int v)
      {
      _track = v * VOICES + _track % VOICES;
      int tracks = _score->nstaves() * VOICES;
      if (_track < 0)
            _track = 0;
      else if (_track >= tracks)
            _track = tracks - 1;
      }

void Cursor::setVoice(int v)
      {
      _track = (_track / VOICES) * VOICES + v;
      int tracks = _score->nstaves() * VOICES;
      if (_track < 0)
            _track = 0;
      else if (_track >= tracks)
            _track = tracks - 1;
      }

int Cursor::staffIdx() const
      {
      return _track / VOICES;
      }

int Cursor::voice() const
      {
      return _track % VOICES;
      }

