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

#include "libmscore/chordrest.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/note.h"
#include "libmscore/stafftext.h"
#include "libmscore/text.h"
#include "libmscore/measure.h"
#include "libmscore/repeatlist.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/segment.h"
#include "script.h"
#include "cursor.h"

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

Cursor::Cursor(Score* s)
      {
      _score    = s;
      _staffIdx = 0;
      _voice    = 0;
      _segment  = 0;
      _expandRepeat = false;
      _curRepeatSegment = 0;
      _curRepeatSegmentIndex = 0;
      }

Cursor::Cursor(Score* s, bool expandRepeat)
      {
      _score    = s;
      _staffIdx = 0;
      _voice    = 0;
      _segment  = 0;
      _expandRepeat = expandRepeat;
      _curRepeatSegment = 0;
      _curRepeatSegmentIndex = 0;
      _score->updateRepeatList(expandRepeat);
      }

//---------------------------------------------------------
//   rewind
//---------------------------------------------------------

void Cursor::rewind(int type)
      {
      if (type == 0) {
            _segment   = 0;
            Measure* m = _score->firstMeasure();
            if (_expandRepeat && !_score->repeatList()->isEmpty()){
                  _curRepeatSegment = _score->repeatList()->first();
                  _curRepeatSegmentIndex = 0;
                  }
            if (m) {
                  _segment = m->first();
                  if (_staffIdx >= 0) {
                        int track = _staffIdx * VOICES + _voice;
                        while (_segment && ((_segment->subtype() != SegChordRest) || (_segment->element(track) == 0)))
                              _segment = _segment->next1();
                        }
                  }
            }
      else if (type == 1) {
            _segment  = _score->selection().startSegment();
            _staffIdx = _score->selection().staffStart();
            _voice    = 0;
            }
      else if (type == 2) {
            _segment  = _score->selection().endSegment();
            _staffIdx = _score->selection().staffEnd();
            _voice    = 0;
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
      Segment* seg = _segment;
      seg = seg->next1();
      RepeatSegment* rs = repeatSegment();
      if (rs && expandRepeat()){
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
                        seg = m ? m->first() : 0;
                        }
                  else
                        seg = 0;
                  }
            }

      if (_staffIdx >= 0) {
            //int track = _staffIdx * VOICES + _voice;
            while (seg && (!(seg->subtype() & (SegChordRest | SegGrace)) /*|| !seg->element(track)*/)) {
                  seg = seg->next1();
                  }
            }
      _segment = seg;
      return seg != 0;
      }

//---------------------------------------------------------
//   nextMeasure
//    go to first segment of next measure
//    return false if end of score is reached
//---------------------------------------------------------

bool Cursor::nextMeasure()
      {
      Measure* m = segment()->measure();
      m = m->nextMeasure();
      RepeatSegment* rs = repeatSegment();
      if (rs && expandRepeat()){
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
            setSegment(0);
            return false;
            }
      Segment* seg = m->first();
      if (_staffIdx >= 0) {
            int track = _staffIdx * VOICES + _voice;
            while (seg && ((seg->subtype() != SegChordRest) || (seg->element(track) == 0)))
                  seg = seg->next1();
            }
      _segment = seg;
      return seg != 0;
      }

//---------------------------------------------------------
//   putStaffText
//---------------------------------------------------------

void Cursor::putStaffText(Text* s)
      {
#if 0
      if (!element() || !s)
            return;
      s->setTrack(element()->track());
      s->setTextStyleType(TEXT_STYLE_STAFF);
      s->setParent(element()->measure());
      s->score()->undoAddElement(s);
      s->score()->setLayoutAll(true);
#endif
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Cursor::add(ChordRest* c)
      {
#if 0
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
#endif
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int Cursor::tick()
      {
      int offset = 0;
      RepeatSegment* rs = repeatSegment();
      if (rs && expandRepeat())
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

bool Cursor::eos() const
      {
      return _segment == 0;
      }

bool Cursor::isChord() const
      {
      return (element() ? (element()->type() == CHORD) : false);
      }
bool Cursor::isRest() const
      {
      return (element() ? (element()->type() == REST) : false);
      }

Element* Cursor::element() const
      {
      int track = _voice + _staffIdx * VOICES;
      return _segment ? _segment->element(track) : 0;
      }

