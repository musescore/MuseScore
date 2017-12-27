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
            if (!s->isVolta())
                  continue;
            Volta* volta = toVolta(s);
            if (tick >= volta->tick() && tick < volta->tick2())
                  return volta;
            }
      return 0;
      }

//---------------------------------------------------------
//   searchLabel
//    @param startMeasure From this measure, if nullptr from firstMeasure
//    @param endMeasure   Up to and including this measure, if nullptr till end of score
//---------------------------------------------------------

Measure* Score::searchLabel(const QString& s, Measure* startMeasure, Measure* endMeasure)
      {
      if (nullptr == startMeasure)
            startMeasure = firstMeasure();
      if (nullptr == endMeasure)
            endMeasure = lastMeasure();

      if (s == "start")
            return startMeasure;
      else if (s == "end")
            return endMeasure;

      endMeasure = endMeasure->nextMeasure(); // stop comparison needs measure one past the last one to check
      for (Measure* m = startMeasure; m && (m != endMeasure); m = m->nextMeasure()) {
            for (auto e : m->el()) {
                  if (   (e->isMarker())
                      && (toMarker(e)->label() == s)) {
                        return m;
                        }
                  }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   searchLabelWithinSectionFirst
//---------------------------------------------------------

Measure* Score::searchLabelWithinSectionFirst(const QString& s, Measure* sectionStartMeasure, Measure* sectionEndMeasure)
      {
      Measure* result = searchLabel(s, sectionStartMeasure, sectionEndMeasure);
      if ((nullptr == result) && (sectionStartMeasure != firstMeasure())) { // not found, expand to the front
            result = searchLabel(s, nullptr, sectionStartMeasure->prevMeasure());
            }
      if ((nullptr == result) && (sectionEndMeasure != lastMeasure())) { // not found, expand to the end
            result = searchLabel(s, sectionEndMeasure->nextMeasure(), nullptr);
            }
      return result;
      }

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
                        sectionStartMeasureBase = 0; // reset to NULL to indicate that don't know starting Measure of next section after starting new section
                        sectionEndMeasureBase   = 0;
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
//    appends repeat segments using rs
//---------------------------------------------------------

void RepeatList::unwindSection(Measure* sectionStartMeasure, Measure* sectionEndMeasure)
      {
//      qDebug("unwind %d-measure section starting %p through %p", sectionEndMeasure->no()+1, sectionStartMeasure, sectionEndMeasure);

      if (!sectionStartMeasure || !sectionEndMeasure) {
            qDebug("invalid section start/end");
            return;
            }

      // both of these trackers possibly should be private members to allow tracking accross all sections?
      // Especially when jumping to a different section and having playRepeats enabled could suffer from this
      std::map<Volta*, Measure*> voltaRangeEnds; // open volta possibly ends past the end of its spanner
      std::set<Jump*> jumpsTaken; // take the jumps only once, so store them

      rs = nullptr; // no measures to be played yet

      Measure* prevMeasure = nullptr; // the last processed measure that is part of this RepeatSegment
      Measure* currentMeasure = sectionStartMeasure; // the measure to be processed/evaluated

      Measure* startFrom = sectionStartMeasure; //the last StartRepeat encountered in this loop; we should return here upon hitting a repeat
      int startFromRepeatStartCount = findStartFromRepeatCount(startFrom, sectionEndMeasure);

      Volta* volta = nullptr;

      Measure* playUntilMeasure = nullptr;      // used during jumping
      Measure* continueAtMeasure = nullptr;     // used during jumping

      while (currentMeasure && (currentMeasure != sectionEndMeasure->nextMeasure())) {
            if (volta && (currentMeasure == voltaRangeEnds.at(volta)->nextMeasure())) { // volta was active, is it still?
                  volta = nullptr;
                  }
            // Should we play or skip this measure: --> look for volta
            if (!volta) {
                  volta = _score->searchVolta(currentMeasure->tick());
                  if (volta) {
                        auto voltaRangeEnd = voltaRangeEnds.find(volta);
                        if (voltaRangeEnd == voltaRangeEnds.end()) { // not yet determined the real endpoint
                              // start by assuming the end of the spanner == the end of this volta (closed volta)
                              Measure* voltaEndMeasure = volta->endMeasure();

                              // open volta may end past its spanner
                              if (volta->getProperty(P_ID::END_HOOK_TYPE).value<HookType>() == HookType::NONE) {
                                    Measure* nextMeasureToInspect = voltaEndMeasure->nextMeasure();
                                    // open volta ends:
                                    while (  (nextMeasureToInspect)     // end of score
                                          && (nextMeasureToInspect != sectionEndMeasure->nextMeasure()) // or end of section
                                          && (!voltaEndMeasure->repeatEnd())                       // hitting an endRepeat
                                          && (!_score->searchVolta(nextMeasureToInspect->tick()))  // or if another volta starts
                                          && (!voltaEndMeasure->repeatJump()) //or hitting a jump, otherwise the part after the jump might be considered under this volta as well…
                                          ) { // nextMeasureToInspect is still part of this volta
                                          voltaEndMeasure = nextMeasureToInspect;
                                          nextMeasureToInspect = voltaEndMeasure->nextMeasure();
                                          }
                                    }

                              // found the real ending of this volta, store it to minimize search efforts
                              voltaRangeEnd = voltaRangeEnds.insert(std::pair<Volta*, Measure*>(volta, voltaEndMeasure)).first;
                              }

                        if (!volta->hasEnding(startFrom->playbackCount())) {
                              // volta does not apply for expected playbackCount --> skip it
                              // but first finalize the current RepeatSegment
                              if (rs) {
                                    rs->len = prevMeasure->endTick() - rs->tick;
                                    append(rs);
                                    rs = nullptr;
                                    }

                              // now skip the volta
                              currentMeasure = voltaRangeEnd->second->nextMeasure();
                              volta = nullptr;

                              // restart processing for the new measure
                              prevMeasure = nullptr;
                              continue;
                              }
                        }
                  }

            // include this measure into the current RepeatSegment
            currentMeasure->setPlaybackCount(currentMeasure->playbackCount() + 1);
            if (nullptr == rs) {
                  rs = new RepeatSegment;
                  rs->tick = currentMeasure->tick();
                  }
            prevMeasure = currentMeasure;

            if (currentMeasure->repeatStart()) {
                   // always start from the last encountered repeat
                  startFrom = currentMeasure;
                  startFromRepeatStartCount = findStartFromRepeatCount(startFrom, sectionEndMeasure);
                  }

            if (    (currentMeasure->repeatEnd())
                 && (currentMeasure->playbackCount() < currentMeasure->repeatCount())    // not yet exhausted our number of repeats
                 ) {
                  // finalize this RepeatSegment
                  rs->len = currentMeasure->endTick() - rs->tick;
                  append(rs);
                  rs = nullptr;
                  // we already know where to start from now, so continue right away with the new reference
                  currentMeasure = startFrom;
                  prevMeasure = nullptr;
                  volta = nullptr;
                  continue;
                  }

            // we will now check for jumps, these should only be followed upon the last pass through a measure
            if (   (startFrom->playbackCount() == startFromRepeatStartCount) // means last pass through this set of repeats
                || (volta && (startFrom->playbackCount() == volta->lastEnding()))) { // or last pass through this volta
                  if (currentMeasure->repeatJump()) { // found a jump, should we follow it?
                        // fetch the jump
                        Jump* jump = nullptr;
                        for (Element* e : currentMeasure->el()) {
                              if (e->isJump()) {
                                    jump = toJump(e);
                                    break;
                                    }
                              }
                        // have we processed it already?
                        if (jumpsTaken.find(jump) == jumpsTaken.end()) { // not yet processed
                              // processing it now
                              jumpsTaken.insert(jump);
                              // validate the jump
                              Measure* jumpToMeasure     = _score->searchLabelWithinSectionFirst(jump->jumpTo()    , sectionStartMeasure, sectionEndMeasure);
                              playUntilMeasure  = _score->searchLabelWithinSectionFirst(jump->playUntil() , sectionStartMeasure, sectionEndMeasure);
                              continueAtMeasure = _score->searchLabelWithinSectionFirst(jump->continueAt(), sectionStartMeasure, sectionEndMeasure);
                              if (jumpToMeasure && playUntilMeasure && (continueAtMeasure || jump->continueAt().isEmpty())) {
                                    // we will jump, but first finalize the current RepeatSegment
                                    rs->len = prevMeasure->endTick() - rs->tick;
                                    append(rs);
                                    rs = nullptr;
                                    // now jump
                                    if (jump->playRepeats()) { // reset playbackCounts will retrigger all repeats
                                          for (Measure* m = _score->firstMeasure(); m; m = m->nextMeasure())
                                                m->setPlaybackCount(0);
                                          }
                                    else { // set each measure to have it play it's final time, but only from our jumptarget on until the current measure
                                          for (Measure* m = jumpToMeasure; (m && (m != currentMeasure->nextMeasure())); m = m->nextMeasure()) {
                                                if (m->playbackCount() != 0)
                                                      m->setPlaybackCount(m->playbackCount() - 1);
                                                }
                                          }
                                    currentMeasure = jumpToMeasure;
                                    startFrom = findStartRepeat(currentMeasure); // not yet happy with these, but not worse than before
                                    startFromRepeatStartCount = findStartFromRepeatCount(startFrom, sectionEndMeasure);
                                    // restart processing
                                    prevMeasure = nullptr;
                                    volta = nullptr;
                                    continue;
                                    }
                              }
                        }

                  if (currentMeasure == playUntilMeasure) {
                        // end of processing this jump
                        playUntilMeasure = nullptr;
                        // finalize the current RepeatSegment
                        rs->len = prevMeasure->endTick() - rs->tick;
                        append(rs);
                        rs = nullptr;
                        // we know where to continue, so jump there
                        currentMeasure = continueAtMeasure;
                        continueAtMeasure = nullptr;
                        // restart processing
                        prevMeasure = nullptr;
                        volta = nullptr;
                        continue;
                        }
                  }

            // keep looping until reach end of score or end of the section
            currentMeasure = currentMeasure->nextMeasure();
            }

      // append the final repeat segment of that section
      if (rs) {
            if (prevMeasure) {
                  rs->len = prevMeasure->endTick() - rs->tick;
                  if (rs->len) {
                        append(rs);
                        rs = nullptr;
                        }
                  else
                        delete rs;
                  }
            else // not even a single measure included in the segment -> it is empty
                  delete rs;
            }
      }

//---------------------------------------------------------
//   findStartRepeat
//    search backwards starting at a given measure to find a repeatStart
//    @return the measure having the repeatStart or start of section
//---------------------------------------------------------

Measure* RepeatList::findStartRepeat(Measure* m)
      {
      while ((!m->repeatStart())
          && (m != _score->firstMeasure())
          && (!m->prevMeasure()->sectionBreak()))
            {
            m = m->prevMeasure();
            }
      return m;
      }

//---------------------------------------------------------
//   findStartFromRepeatCount
//    @param startFrom starting measure for this repeat subsection
//    @param sectionEndMeasure final measure of the section in which to look for possible endRepeats
//    @return number of times playback passes the start repeat barline (not accounting for jumps)
//---------------------------------------------------------

int RepeatList::findStartFromRepeatCount(Measure * const startFrom, Measure * const sectionEndMeasure)
      {
      Measure * m = startFrom;
      int startFromRepeatCount = (m->repeatEnd())? (m->repeatCount()) : 1;
      m = m->nextMeasure();
      while (m && m != sectionEndMeasure->nextMeasure() && !m->repeatStart()) {
            if (m->repeatEnd()) {
                  startFromRepeatCount += m->repeatCount() - 1;
                  }
            m = m->nextMeasure();
            }
      return startFromRepeatCount;
      }
}
