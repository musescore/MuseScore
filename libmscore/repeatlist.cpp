//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "repeatlist.h"
#include "score.h"
#include "measure.h"
#include "tempo.h"
#include "volta.h"
#include "segment.h"
#include "marker.h"
#include "jump.h"

namespace Ms {

//---------------------------------------------------------
//   searchVolta
//    return volta at tick
//---------------------------------------------------------

Volta* Score::searchVolta(int tick) const
      {
      for (const std::pair<int,Spanner*>& p : _spanner.map()) {
            Spanner* s = p.second;
            if (s->type() != Element::Type::VOLTA)
                  continue;
            Volta* volta = static_cast<Volta*>(s);
            if (tick >= volta->tick() && tick < volta->tick2())
                  return volta;
            }
      return 0;
      }

//---------------------------------------------------------
//   searchLabel
//---------------------------------------------------------

Measure* Score::searchLabel(const QString& s)
      {
      if (s == "start")
            return firstMeasure();
      else if (s == "end")
            return lastMeasure();
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (auto e : m->el()) {
                  if (e->type() == Element::Type::MARKER) {
                        const Marker* marker = static_cast<const Marker*>(e);
                        if (marker->label() == s)
                              return m;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   searchLabelWithinSectionFirst
//---------------------------------------------------------

Measure* Score::searchLabelWithinSectionFirst(const QString& s, Measure* sectionStartMeasure, Measure* sectionEndMeasure)
      {
      if (s == "start")
            return sectionStartMeasure;
      else if (s == "end")
            return sectionEndMeasure;
      for (Measure* m = sectionStartMeasure; m && (m != sectionEndMeasure->nextMeasure()); m = m->nextMeasure()) {
            for (auto e : m->el()) {
                  if (e->type() == Element::Type::MARKER) {
                        const Marker* marker = static_cast<const Marker*>(e);
                        if (marker->label() == s)
                              return m;
                        }
                  }
            }

      // if did not find label within section, then search for label in entire score
      return searchLabel(s);
      }

//---------------------------------------------------------
//   RepeatLoop
//---------------------------------------------------------

struct RepeatLoop {
      enum class LoopType : char { REPEAT, JUMP };

      LoopType type;
      Measure* m;   // start measure of LoopType::REPEAT
      int count;
      QString stop, cont;

      RepeatLoop() {}
      RepeatLoop(Measure* _m)  {
            m     = _m;
            count = 0;
            type  = LoopType::REPEAT;
            }
      RepeatLoop(const QString s, const QString c)
         : stop(s), cont(c)
            {
            m    = 0;
            type = LoopType::JUMP;
            }
      };

//---------------------------------------------------------
//   RepeatSegment
//---------------------------------------------------------

RepeatSegment::RepeatSegment()
      {
      tick       = 0;
      len        = 0;
      utick      = 0;
      utime      = 0.0;
      timeOffset = 0.0;
      }

//---------------------------------------------------------
//   RepeatList
//---------------------------------------------------------

RepeatList::RepeatList(Score* s)
      {
      _score = s;
      idx1  = 0;
      idx2  = 0;
      }

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

int RepeatList::ticks()
      {
      if (length() > 0) {
            RepeatSegment* s = last();
            return s->utick + s->len;
            }
      return 0;
      }

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void RepeatList::update()
      {
      const TempoMap* tl = _score->tempomap();

      int utick = 0;
      qreal t  = 0;

      for(RepeatSegment* s : *this) {
            s->utick      = utick;
            s->utime      = t;
            qreal ct      = tl->tick2time(s->tick);
            s->timeOffset = t - ct;
            utick        += s->len;
            t            += tl->tick2time(s->tick + s->len) - ct;
            }
      }

//---------------------------------------------------------
//   utick2tick
//---------------------------------------------------------

int RepeatList::utick2tick(int tick) const
      {
      unsigned n = size();
      if (n == 0)
            return tick;
      if (tick < 0)
            return 0;
      unsigned ii = (idx1 < n) && (tick >= at(idx1)->utick) ? idx1 : 0;
      for (unsigned i = ii; i < n; ++i) {
            if ((tick >= at(i)->utick) && ((i + 1 == n) || (tick < at(i+1)->utick))) {
                  idx1 = i;
                  return tick - (at(i)->utick - at(i)->tick);
                  }
            }
      if (MScore::debugMode) {
            qFatal("tick %d not found in RepeatList", tick);
            }
      return 0;
      }

//---------------------------------------------------------
//   tick2utick
//---------------------------------------------------------

int RepeatList::tick2utick(int tick) const
      {
      for (const RepeatSegment* s : *this) {
            if (tick >= s->tick && tick < (s->tick + s->len))
                  return s->utick + (tick - s->tick);
            }
      return last()->utick + (tick - last()->tick);
      }

//---------------------------------------------------------
//   utick2utime
//---------------------------------------------------------

qreal RepeatList::utick2utime(int tick) const
      {
      unsigned n = size();
      unsigned ii = (idx1 < n) && (tick >= at(idx1)->utick) ? idx1 : 0;
      for (unsigned i = ii; i < n; ++i) {
            if ((tick >= at(i)->utick) && ((i + 1 == n) || (tick < at(i+1)->utick))) {
                  int t     = tick - (at(i)->utick - at(i)->tick);
                  qreal tt = _score->tempomap()->tick2time(t) + at(i)->timeOffset;
                  return tt;
                  }
            }
      return 0.0;
      }

//---------------------------------------------------------
//   utime2utick
//---------------------------------------------------------

int RepeatList::utime2utick(qreal t) const
      {
      unsigned n = size();
      unsigned ii = (idx2 < n) && (t >= at(idx2)->utime) ? idx2 : 0;
      for (unsigned i = ii; i < n; ++i) {
            if ((t >= at(i)->utime) && ((i + 1 == n) || (t < at(i+1)->utime))) {
                  idx2 = i;
                  return _score->tempomap()->time2tick(t - at(i)->timeOffset) + (at(i)->utick - at(i)->tick);
                  }
            }
      if (MScore::debugMode) {
            qFatal("time %f not found in RepeatList", t);
            }
      return 0;
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void RepeatList::dump() const
      {
#if 0
      qDebug("==Dump Repeat List:==");
      foreach(const RepeatSegment* s, *this) {
            qDebug("%p  tick: %3d(%d) %3d(%d) len %d(%d) beats  %f + %f", s,
               s->utick / MScore::division,
               s->utick / MScore::division / 4,
               s->tick / MScore::division,
               s->tick / MScore::division / 4,
               s->len / MScore::division,
               s->len / MScore::division / 4,
               s->utime, s->timeOffset);
            }
#endif
      }

//---------------------------------------------------------
//   unwind
//    implements:
//          - repeats
//          - volta
//          - d.c. al fine
//          - d.s. al fine
//          - d.s. al coda
//---------------------------------------------------------

void RepeatList::unwind()
      {
      qDeleteAll(*this);
      clear();
      Measure* fm = _score->firstMeasure();
      if (!fm)
            return;

 //qDebug("unwind===================");

      for (Measure* m = fm; m; m = m->nextMeasure())
            m->setPlaybackCount(0);

      MeasureBase* sectionStartMeasureBase = NULL; // NULL indicates haven't discovered starting Measure of section
      MeasureBase* sectionEndMeasureBase = NULL;

      // partition score by section breaks and unwind individual sections seperately
      // note: section breaks may occur on non-Measure frames, so must seach list of all MeasureBases
      for (MeasureBase* mb = _score->first(); mb; mb = mb->next()) {

            // unwindSection only deals with real Measures, so sectionEndMeasureBase and sectionStartMeasureBase will only point to real Measures
            if (mb->isMeasure()) {
                  sectionEndMeasureBase = mb; // ending measure of section is the most recently encountered actual Measure
            
                  // starting measure of section will be the first non-NULL actual Measure encountered
                  if (sectionStartMeasureBase == NULL)
                        sectionStartMeasureBase = mb;
            }

            // if found section break or reached final MeasureBase of score, then unwind
            if (mb->sectionBreak() || !mb->nextMeasure()) {

                  // only unwind if section starts and ends with actual real measure
                  if (sectionStartMeasureBase && sectionEndMeasureBase) {
                        unwindSection(reinterpret_cast<Measure*>(sectionStartMeasureBase), reinterpret_cast<Measure*>(sectionEndMeasureBase));
                        sectionStartMeasureBase = sectionEndMeasureBase = NULL; // reset to NULL to indicate that don't know starting Measure of next section after starting new section
                        }
                  else {
                        qDebug( "Will not unroll a section that doesn't start or end with an actual measure. sectionStartMeasureBase = %p, sectionEndMeasureBase = %p",
                                    sectionStartMeasureBase, sectionEndMeasureBase);
                        }
                  }
            }

      update();
      dump();
      }

//---------------------------------------------------------
//   unwindSection
//    unwinds from sectionStartMeasure through sectionEndMeasure
//    appends repeat segments to rs
//---------------------------------------------------------

void RepeatList::unwindSection(Measure* sectionStartMeasure, Measure* sectionEndMeasure)
      {
//      qDebug("unwind %d-measure section starting %p through %p", sectionEndMeasure->no()+1, sectionStartMeasure, sectionEndMeasure);

      QList<Jump*> jumps; // take the jumps only once so store them

      rs         = new RepeatSegment;
      rs->tick   = sectionStartMeasure->tick(); // prepare initial repeat segment for start of this section

      Measure* endGoto    = 0; // measure where the current goto should stop
      Measure* continueAt = 0; // measure where the playback should continue after the endGoto
      Measure* m          = 0;
      int loopEnding      = 1; // which volta endings to enable (previous |: playbackCount())
      bool isGoto         = false;

      for (Measure* nm = sectionStartMeasure; nm; ) {
            m = nm;
            Repeat flags = m->repeatFlags();

//            qDebug("m%d(tick %7d) %p: playbackCount %d loop %d repeatCount %d isGoto %d endRepeat %p continueAt %p flags 0x%x",
//                   m->no()+1, m->tick(), m, m->playbackCount(), loop, repeatCount, isGoto, endRepeat, continueAt, int(flags));

            Volta* volta = _score->searchVolta(m->tick());
            const bool skipMeasure = volta && !volta->hasEnding(loopEnding);
            if (skipMeasure) {
                  if (rs->tick < m->tick()) {
                        rs->len = m->tick() - rs->tick;
                        append(rs);
                        rs = new RepeatSegment;
                        }
                  rs->tick = m->endTick();
                  }
            else if (!isGoto) // don't increase counts during a jump
                  m->setPlaybackCount(m->playbackCount() + 1);

            if (!skipMeasure && (flags & Repeat::START))
                  loopEnding = m->playbackCount();

            // always match endRepeat marker, ignoring "skipMeasure" from volta
            if (isGoto && (endGoto == m)) {
                  isGoto = false;
                  nm = continueAt;
                  endGoto = 0;
                  continueAt = 0;
                  if (nm == 0)
                        // jump to "end" of the track - done.
                        break;
                  rs->len = m->endTick() - rs->tick;
                  append(rs);
                  rs       = new RepeatSegment;
                  rs->tick = nm->tick();
                  continue;
                  }
            else if (!skipMeasure && (flags & Repeat::END)) {
                  // will never repeat during any DC or DS
                  if (!isGoto && m->playbackCount() < m->repeatCount()) {
                        // only check *local* repeat counter, not all loop runs
                        nm = jumpToStartRepeat(m);
                        continue;
                        }
                  }
            // only jump if we didn't skip, didn't jump due to a repeat
            if (!skipMeasure && (flags & Repeat::JUMP)) {
                  Jump* s = 0;
                  foreach(Element* e, m->el()) {
                        if (e->type() == Element::Type::JUMP) {
                              s = static_cast<Jump*>(e);
                              break;
                              }
                        }
                  // jump only once
                  if (jumps.contains(s)) {
                        if (endGoto == _score->searchLabelWithinSectionFirst(s->playUntil(), sectionStartMeasure, sectionEndMeasure)) {
                              // cancel current jump it it has same "playUntil" label as "s"
                              isGoto = false;
                              endGoto = 0;
                              continueAt = 0;
                              }
                        }
                  else if (s) {
                        jumps.append(s);
                        nm          = _score->searchLabelWithinSectionFirst(s->jumpTo()    , sectionStartMeasure, sectionEndMeasure);
                        endGoto   = _score->searchLabelWithinSectionFirst(s->playUntil() , sectionStartMeasure, sectionEndMeasure);
                        continueAt  = _score->searchLabelWithinSectionFirst(s->continueAt(), sectionStartMeasure, sectionEndMeasure);

                        if (nm && endGoto) {
                              isGoto      = true;
                              rs->len = m->endTick() - rs->tick;
                              append(rs);
                              rs = new RepeatSegment;
                              rs->tick  = nm->tick();
                              continue;
                              }
                        }
                  else
                        qDebug("Jump not found");
                  }

            // keep looping until reach end of score or end of the section
            nm = m->nextMeasure();
            if (nm == sectionEndMeasure->nextMeasure())
                  break;
            }

      // append the final repeat segment of that section
      if (rs) {
            rs->len = m->endTick() - rs->tick;
            if (rs->len)
                  append(rs);
            else
                  delete rs;
            }
      }

//---------------------------------------------------------
//   jumpToStartRepeat
//---------------------------------------------------------

Measure* RepeatList::jumpToStartRepeat(Measure* m)
      {
      // finalize the previous repeat segment
      rs->len = m->tick() + m->ticks() - rs->tick;
      append(rs);

      // search backwards until find start of repeat
      while (true) {

            if (m->repeatFlags() & Repeat::START)
                  break;

            if (m == _score->firstMeasure())
                  break;

            if (m->prevMeasure()->sectionBreak())
                  break;

            m = m->prevMeasure();
            }

      // initialize the next repeat segment
      rs        = new RepeatSegment;
      rs->tick  = m->tick();
      return m;
      }

}

