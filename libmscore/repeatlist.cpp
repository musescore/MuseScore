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
      utick      = 0;
      utime      = 0.0;
      timeOffset = 0.0;
      }

RepeatSegment::RepeatSegment(RepeatSegment * const rs, Measure * const fromMeasure, Measure * const untilMeasure)
      {
      tick       = 0;
      utick      = 0;
      utime      = 0.0;
      timeOffset = 0.0;
      //copy the measure list
      auto it = rs->measureList.cbegin();
      if (nullptr != fromMeasure) {
            //skip until fromMeasure
            while ((it->first != fromMeasure) && ((++it) != rs->measureList.cend()));
            }
      while (it != rs->measureList.cend()) {
            if ((nullptr != untilMeasure) && (it->first->no() > untilMeasure->no())) {
                  break;
                  }
            measureList.push_back(std::make_pair(it->first, it->second));
            ++it;
            }
      if (!measureList.empty()) {
            tick = measureList.cbegin()->first->tick();
            }
      }

void RepeatSegment::addMeasure(Measure * const m)
      {
      if (measureList.empty()) {
            tick = m->tick();
            }
      measureList.push_back(std::make_pair(m, m->playbackCount()));
      }

bool RepeatSegment::containsMeasure(Measure const * const m) const
      {
      for (std::pair<Measure*, int> measure : measureList)
            {
            if (measure.first == m)
                  return true;
            }
      return false;
      }

int RepeatSegment::len() const
      {
      return (measureList.empty()) ? 0 : (measureList.last().first->endTick() - tick);
      }

//---------------------------------------------------------
//   playbackCount
//    returns the playbackCount of this measure at the time it was inserted into the repeatSegment
//    returns 0 if the measure is not part of this repeatSegment
//---------------------------------------------------------

int RepeatSegment::playbackCount(Measure * const m) const
      {
      for (std::pair<Measure*, int> measure : measureList)
            {
            if (measure.first == m)
                  return measure.second;
            }
      return 0;
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
            return s->utick + s->len();
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
            utick        += s->len();
            t            += tl->tick2time(s->tick + s->len()) - ct;
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
            if (tick >= s->tick && tick < (s->tick + s->len() ))
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
      _voltaRanges.clear();
      _jumpsTaken.clear();
      Measure* fm = _score->firstMeasure();
      if (!fm)
            return;

 //qDebug("unwind===================");

      for (Measure* m = fm; m; m = m->nextMeasure())
            m->setPlaybackCount(0);

      preProcessVoltas();

      MeasureBase* sectionStartMeasureBase = NULL; // NULL indicates haven't discovered starting Measure of section
      MeasureBase* sectionEndMeasureBase = NULL;

      // partition score by section breaks and unwind individual sections separately
      // note: section breaks may occur on non-Measure frames, so must search list of all MeasureBases
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
//   preProcessVoltas
//    determine the real effective endpoint of voltas
//    because an open volta doesn't necessarily end on its end anchorpoint
//    MUST be called before unwinding(Section) logic!
//---------------------------------------------------------

void RepeatList::preProcessVoltas()
      {
      Volta* nextVolta = nullptr; //work backwards as a volta can end because the next one starts
      for (auto rit = _score->spanner().crbegin(); rit != _score->spanner().crend(); ++rit)
            {
            Spanner* s = (*rit).second;
            if (!s->isVolta())
                  continue;
            Volta* volta = toVolta(s);
            if (volta) {
                  if (_voltaRanges.find(volta) == _voltaRanges.end()) { // not yet determined the real endpoint, should always be true
                        // start by assuming the end of the spanner == the end of this volta (closed volta)
                        Measure* voltaEndMeasure = volta->endMeasure();
                        // open volta may end past its spanner
                        if (volta->getProperty(P_ID::END_HOOK_TYPE).value<HookType>() == HookType::NONE) {
                              Measure* nextMeasureToInspect = voltaEndMeasure->nextMeasure();
                              // open volta ends:
                              while (  (nextMeasureToInspect)     // end of score
                                    && (!voltaEndMeasure->sectionBreak()) // or end of section
                                    && (!voltaEndMeasure->repeatEnd())    // hitting an endRepeat
                                    && (!nextMeasureToInspect->repeatStart()) // or starting a new repeat
                                    && (!nextVolta || (nextMeasureToInspect != nextVolta->startMeasure()))  // or if another volta starts
                                    && (!voltaEndMeasure->repeatJump()) //or hitting a jump, otherwise the part after the jump might be considered under this volta as well…
                                    ) { // nextMeasureToInspect is still part of this volta
                                    voltaEndMeasure = nextMeasureToInspect;
                                    nextMeasureToInspect = voltaEndMeasure->nextMeasure();
                                    }
                              }
                        // found the real ending of this volta, store it to minimize search efforts
                        _voltaRanges.insert(std::pair<Volta*, Measure*>(volta, voltaEndMeasure));
                        nextVolta = volta; // this volta might indicate the end of the previous one
                        }
                  else {
                        qDebug("Found same volta twice in the spannermap");
                        }
                  }
            } // end spannerloop
      }

//---------------------------------------------------------
//   searchVolta
//    return the iterator in _voltaRanges for the volta spanning the given measure
//---------------------------------------------------------

std::map<Volta*, Measure*>::const_iterator RepeatList::searchVolta(Measure * const measure) const
      {
      std::map<Volta*, Measure*>::const_iterator voltaRange;
      for (voltaRange = _voltaRanges.cbegin(); voltaRange != _voltaRanges.cend(); ++voltaRange) {
            if (   (voltaRange->first->startMeasure()->tick() <= measure->tick())
                && (measure->tick() <= voltaRange->second->tick())
                ) {
                  break;
                  }
            }
      return voltaRange;
      }

//---------------------------------------------------------
//   unwindSection
//    unwinds from sectionStartMeasure through sectionEndMeasure
//    appends repeat segments using rs
//---------------------------------------------------------

void RepeatList::unwindSection(Measure* const sectionStartMeasure, Measure* const sectionEndMeasure)
      {
//      qDebug("unwind %d-measure section starting %p through %p", sectionEndMeasure->no()+1, sectionStartMeasure, sectionEndMeasure);

      if (!sectionStartMeasure || !sectionEndMeasure) {
            qDebug("invalid section start/end");
            return;
            }

      rs = nullptr; // no measures to be played yet

      Measure* prevMeasure = nullptr; // the last processed measure that is part of this RepeatSegment
      Measure* currentMeasure = sectionStartMeasure; // the measure to be processed/evaluated

      Measure* startFrom = sectionStartMeasure; //the last StartRepeat encountered in this loop; we should return here upon hitting a repeat
      int startFromRepeatStartCount = findStartFromRepeatCount(startFrom);

      std::map<Volta*, Measure*>::const_iterator voltaRange = _voltaRanges.cend();

      Measure* playUntilMeasure = nullptr;      // used during jumping
      Measure* continueAtMeasure = nullptr;     // used during jumping

      while (currentMeasure && (currentMeasure != sectionEndMeasure->nextMeasure())) {
            if (   (voltaRange != _voltaRanges.cend())
                && (currentMeasure == voltaRange->second->nextMeasure())
                ) { // volta was active -> not anymore
                  voltaRange = _voltaRanges.cend();
                  }
            // Should we play or skip this measure: --> look for volta
            if (voltaRange == _voltaRanges.cend()) {
                  voltaRange = searchVolta(currentMeasure);
                  if (   (voltaRange != _voltaRanges.cend())
                      && !voltaRange->first->hasEnding(startFrom->playbackCount())
                      ) {
                        // volta does not apply for expected playbackCount --> skip it
                        // but first finalize the current RepeatSegment
                        if (rs) {
                              push_back(rs);
                              rs = nullptr;
                              }

                        // now skip the volta
                        currentMeasure = voltaRange->second->nextMeasure();
                        voltaRange = _voltaRanges.cend();

                        // restart processing for the new measure
                        prevMeasure = nullptr;
                        continue;
                        }
                  }

            // include this measure into the current RepeatSegment
            currentMeasure->setPlaybackCount(currentMeasure->playbackCount() + 1);
            if (nullptr == rs) {
                  rs = new RepeatSegment();
                  }
            rs->addMeasure(currentMeasure);
            prevMeasure = currentMeasure;

            if (currentMeasure->repeatStart()) {
                   // always start from the last encountered repeat
                  startFrom = currentMeasure;
                  startFromRepeatStartCount = findStartFromRepeatCount(startFrom);
                  }

            if (    (currentMeasure->repeatEnd())
                 && (currentMeasure->playbackCount() < currentMeasure->repeatCount())    // not yet exhausted our number of repeats
                 ) {
                  // finalize this RepeatSegment
                  push_back(rs);
                  rs = nullptr;
                  // we already know where to start from now, so continue right away with the new reference
                  currentMeasure = startFrom;
                  prevMeasure = nullptr;
                  voltaRange = _voltaRanges.cend();
                  continue;
                  }

            // we will now check for jumps, these should only be followed upon the last pass through a measure
            if (   (startFrom->playbackCount() == startFromRepeatStartCount) // means last pass through this set of repeats
                || (   (voltaRange != _voltaRanges.cend())
                    && (startFrom->playbackCount() == voltaRange->first->lastEnding()) // or last pass through this volta
                    )
                ) {
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
                        if (_jumpsTaken.find(jump) == _jumpsTaken.end()) { // not yet processed
                              // processing it now
                              _jumpsTaken.insert(jump);
                              // validate the jump
                              Measure* jumpToMeasure = _score->searchLabelWithinSectionFirst(jump->jumpTo(), sectionStartMeasure, sectionEndMeasure);
                              playUntilMeasure  = _score->searchLabelWithinSectionFirst(jump->playUntil(), sectionStartMeasure, sectionEndMeasure);
                              continueAtMeasure = _score->searchLabelWithinSectionFirst(jump->continueAt(), sectionStartMeasure, sectionEndMeasure);
                              if (jumpToMeasure && playUntilMeasure && (continueAtMeasure || jump->continueAt().isEmpty())) {
                                    // we will jump, but first finalize the current RepeatSegment
                                    push_back(rs);
                                    rs = nullptr;
                                    // now jump
                                    // basically, replay the score from jumpToMeasure all the way up the to last occurence of playUntilMeasure
                                    // this replay lookup uses mostly indexes over iterators as push_back of new segments might invalidate an interator
                                    // step 1: find the last occurence of playUntilMeasure
                                    Measure* copyUntilMeasure = playUntilMeasure; // assume we can rewind back to the playUntilMeasure
                                    int copyUntilIdx = this->size() - 1; // assume we will find the copyUntilMeasure in the (currently) last repeatSegment
                                    int playUntilIdx = this->size();
                                    do {
                                          --playUntilIdx;
                                          }
                                    while (   (!(this->at(playUntilIdx)->containsMeasure(playUntilMeasure)))
                                           && (playUntilIdx > 0)
                                           );
                                    if (isFinalPlaythrough(playUntilMeasure, (this->cbegin() + playUntilIdx))) {
                                          copyUntilIdx = playUntilIdx;
                                          }
                                    else { // not found or not final playthrough
                                          // -> currentMeasure is the most recent parsed measure that should be included into the replay
                                          copyUntilMeasure = currentMeasure;
                                          }

                                    // step 2: find jumpToMeasure
                                    auto copyFromIdx = -1;
                                    Measure * copyFromMeasure = jumpToMeasure;
                                    if (jump->playRepeats()) {
                                          // we want to replay as much from the score as possible
                                          // => find most recent occurence of first playBack of jumpToMeasure #270332
                                          for (int jumpToIdx = playUntilIdx; jumpToIdx >= 0; --jumpToIdx) {
                                                if (this->at(jumpToIdx)->playbackCount(jumpToMeasure) == 1) {
                                                      copyFromIdx = jumpToIdx;
                                                      break;
                                                      }
                                                }
                                          }
                                    else { // no repeats upon jumping => find final occurence of jumpToMeasure
                                          int jumpToIdx = this->size();
                                          do {
                                                --jumpToIdx;
                                                }
                                          while (   (!(this->at(jumpToIdx)->containsMeasure(jumpToMeasure)))
                                                 && (jumpToIdx > 0)
                                                 );
                                          if (isFinalPlaythrough(jumpToMeasure, (this->cbegin() + jumpToIdx))) {
                                                copyFromIdx = jumpToIdx;
                                                }
                                          }

                                    // step 3 : duplicate the part we would otherwise have to rewind
                                    if (copyFromIdx != -1) {
                                          if (jump->playRepeats()) {
                                                // all should be replayed, simply copy over
                                                for (int idx = copyFromIdx; idx <= copyUntilIdx; ++idx) {
                                                      this->push_back(new RepeatSegment(
                                                                            this->at(idx),
                                                                            (idx == copyFromIdx) ? copyFromMeasure : nullptr,
                                                                            (idx == copyUntilIdx) ? copyUntilMeasure : nullptr
                                                                            )
                                                                      );
                                                      }
                                                }
                                          else { // only most recent passes should be played
                                                // so we have to inspect each measure to be it's most recent playthrough
                                                int idx = copyFromIdx;
                                                while (idx <= copyUntilIdx) {
                                                      RepeatSegment* referenceSegment = this->at(idx);
                                                      auto referenceIt = referenceSegment->measureList.cbegin();
                                                      bool forwardToMoreRecentPlaythrough = false;
                                                      if (idx == copyFromIdx) {
                                                            // skip start of segment
                                                            while ((referenceIt->first != copyFromMeasure) && ((++referenceIt) != referenceSegment->measureList.cend()));
                                                            }
                                                      // start copy
                                                      while (   ((referenceIt != referenceSegment->measureList.cend()) && !forwardToMoreRecentPlaythrough)
                                                             && ((idx != copyUntilIdx) || (referenceIt->first->no() <= copyUntilMeasure->no()))
                                                             ) {
                                                            // find most recent occurence of this measure
                                                            auto mostRecentRepeatSegmentIdx = copyUntilIdx; // look backwards to find to most recent first
                                                            while ((mostRecentRepeatSegmentIdx > idx) && !forwardToMoreRecentPlaythrough) {
                                                                  RepeatSegment * mostRecentSegment = this->at(mostRecentRepeatSegmentIdx);
                                                                  if (   (mostRecentSegment->playbackCount(referenceIt->first) >= referenceIt->second)
                                                                      && (referenceIt->first->no() <= copyUntilMeasure->no())
                                                                      ) {
                                                                        // found a more recent playthrough => continue our copy from there
                                                                        forwardToMoreRecentPlaythrough = true;
                                                                        // save continueCopyPosition
                                                                        copyFromMeasure = referenceIt->first;
                                                                        idx = mostRecentRepeatSegmentIdx;
                                                                        continue;
                                                                        }
                                                                  --mostRecentRepeatSegmentIdx;
                                                                  }
                                                            if (!forwardToMoreRecentPlaythrough) {
                                                                  // this is the most recent playthrough of this measure -> copy it
                                                                  if (nullptr == rs) {
                                                                        rs = new RepeatSegment();
                                                                        rs->tick = referenceIt->first->tick();
                                                                        }
                                                                  rs->measureList.push_back(std::make_pair(referenceIt->first, referenceIt->second));
                                                                  }
                                                            // test & copy next measure
                                                            ++referenceIt;
                                                            }
                                                      // store what we've copied from this repeatSegment
                                                      if (rs) {
                                                            push_back(rs);
                                                            rs = nullptr;
                                                            }
                                                      // move to the next repeatSegment
                                                      if (!forwardToMoreRecentPlaythrough) {
                                                            ++idx;
                                                            }
                                                      }
                                                }
                                          }

                                    // step 4 : determine the next measure to evaluate
                                    if (copyFromIdx == -1) {
                                          // still everything to process, because we couldn't copy from the jumpToMeasure yet
                                          currentMeasure = jumpToMeasure; // so start there now
                                          }
                                    else {
                                          if (copyUntilMeasure == playUntilMeasure) {
                                                // fully processed this jump, we know where to continue, so jump there
                                                currentMeasure = continueAtMeasure;
                                                // end of processing this jump
                                                playUntilMeasure = nullptr;
                                                continueAtMeasure = nullptr;
                                                }
                                          else {
                                                // we have copied stuff, but not yet all of it
                                                currentMeasure = copyUntilMeasure->nextMeasure();
                                                }
                                          }
                                    if (currentMeasure) {
                                          startFrom = findStartRepeat(currentMeasure); // not yet happy with these, but not worse than before
                                          startFromRepeatStartCount = findStartFromRepeatCount(startFrom);
                                          }

                                    // restart processing
                                    prevMeasure = nullptr;
                                    voltaRange = _voltaRanges.cend();
                                    continue;
                                    }
                              }
                        }

                  if (currentMeasure == playUntilMeasure) {
                        // end of processing this jump
                        playUntilMeasure = nullptr;
                        // finalize the current RepeatSegment
                        push_back(rs);
                        rs = nullptr;
                        // we know where to continue, so jump there
                        currentMeasure = continueAtMeasure;
                        continueAtMeasure = nullptr;
                        // restart processing
                        prevMeasure = nullptr;
                        voltaRange = _voltaRanges.cend();
                        continue;
                        }
                  }

            // keep looping until reach end of score or end of the section
            currentMeasure = currentMeasure->nextMeasure();
            }

      // append the final repeat segment of that section
      if (rs) {
            if (prevMeasure) {
                  if (rs->len()) {
                        push_back(rs);
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

Measure* RepeatList::findStartRepeat(Measure * const measure) const
      {
      Measure* m = measure;
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
//    @return number of times playback passes the start repeat barline (not accounting for jumps)
//---------------------------------------------------------

int RepeatList::findStartFromRepeatCount(Measure * const startFrom) const
      {
      Measure * m = startFrom;
      int startFromRepeatCount = (m->repeatEnd())? (m->repeatCount()) : 1;
      m = m->nextMeasure();
      while (m && !m->repeatStart()) {
            if (m->repeatEnd()) {
                  startFromRepeatCount += m->repeatCount() - 1;
                  }
            m = (m->sectionBreak()) ? nullptr : m->nextMeasure();
            }
      return startFromRepeatCount;
      }

//---------------------------------------------------------
//   isFinalPlaythrough
//    @param measure the measure to verify
//    @param repeatSegment consider measure with its playbackCount in this specific RepeatSegment
//    @return true if that measure has its final playthrough in the given repeatSegment
//---------------------------------------------------------

bool RepeatList::isFinalPlaythrough(Measure * const measure, QList<RepeatSegment*>::const_iterator repeatSegmentIt) const
      {
      bool finalPlaythrough = false;

      if (   (repeatSegmentIt != this->cend())
          && ((*repeatSegmentIt)->containsMeasure(measure))
          ) {
            // step 1 : so go and look back for the relevant startRepeatMeasure
            Measure * startRepeatMeasure = findStartRepeat(measure);
            auto startRepeatIt = repeatSegmentIt;
            while (  (startRepeatIt != this->cbegin())
                  && (!(*startRepeatIt)->containsMeasure(startRepeatMeasure))
                  ){
                  --startRepeatIt;
                  }
            int startRepeatPlaybackCount = (*startRepeatIt)->playbackCount(startRepeatMeasure);
            // step 2 :  does this measure belong to a volta?
            auto voltaRange = searchVolta(measure);
            // step 3 : is it the final playThrough of this measure?
            if (   (startRepeatPlaybackCount == findStartFromRepeatCount(startRepeatMeasure))
                || (   (voltaRange != _voltaRanges.cend())
                    && (startRepeatPlaybackCount == voltaRange->first->lastEnding())
                    )
                ) {
                  finalPlaythrough = true;
                  }
            //else: found, but not yet final playthrough -> nothing to do
            }
      //else: measure not found == not part of this RepeatSegment -> nothing to do

      return finalPlaythrough;
      }
}
