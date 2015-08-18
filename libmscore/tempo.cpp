//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "tempo.h"
#include "xml.h"
#include "repeatlist.h"


namespace Ms {

//---------------------------------------------------------
//   TEvent
//---------------------------------------------------------

TEvent::TEvent()
      {
      type              = TempoType::INVALID;
      tempo             = 0.0;
      time              = 0.0;
      pauseBeforeTick   = 0.0;
      pauseThroughTick  = 0.0;
      }

TEvent::TEvent(const TEvent& e)
      {
      type  = e.type;
      tempo = e.tempo;
      time  = e.time;
      pauseBeforeTick = e.pauseBeforeTick;
      pauseThroughTick = e.pauseThroughTick;
      }

TEvent::TEvent(qreal tempo, qreal pauseBeforeTick, qreal pauseThroughTick, TempoType type)
      {
      this->type  = type;
      this->tempo = tempo;
      this->time  = 0.0;
      this->pauseBeforeTick = pauseBeforeTick;
      this->pauseThroughTick = pauseThroughTick;
      }

bool TEvent::valid() const
      {
      return !(!type);
      }

void TEvent::dump() const
      {
      qDebug("TempoEvent:\tTime=%f, Type=%2d, Tempo=%f, PauseBefore=%f, PauseThrough=%f", time, static_cast<char>(type), tempo, pauseBeforeTick, pauseThroughTick );
      }

//---------------------------------------------------------
//   TempoMap
//---------------------------------------------------------

TempoMap::TempoMap()
      {
      _tempo    = 2.0;        // default fixed tempo in beat per second
      _tempoSN  = 1;
      _relTempo = 1.0;
      }

//---------------------------------------------------------
//   TempoMap constructor creating an unrolled TempoMap from a RepeatList and a score's graphicalTempoMap.
//    Inserts the adjusted unrolled TempoEvents into this new TempoMap.
//---------------------------------------------------------

TempoMap::TempoMap( const RepeatList* repeatList, const TempoMap* graphicalTempoMap )
      {
      _tempo    = graphicalTempoMap->_tempo;          // default initial fixed tempo in beat per second
      _tempoSN  = 1;                                  // restart serial number for tracking changes (although since never adding entries after constructing unrolled map, SN will never change

      static const TEvent zeroTEvent; // should be filled with all zeroes.  Used only for code clarity as a default to refer to TEvents that don't exist
      TEvent mergedInitialEvent;

      graphicalTempoMap->dump();
      qDebug("STARTING RepeatList::update() using above graphicalTempoMap ---------------");

      // pointers to keep track of the event at the ending tick of the previous segment and the starting tick of the current segment
      const TEvent* prevEndEvent = &zeroTEvent;
      const TEvent* currStartEvent = &zeroTEvent;

      auto hintPositionInsert = begin();

      // unroll each repeat segment starting from s->tick to s->tick+s->len
      for (RepeatSegment* s : *repeatList) {

            int utickOfEventToInsert = s->utick;

            qDebug("START unrolling segment:"); s->dump();

            auto nextGraphicalEvent = graphicalTempoMap->lower_bound(s->tick);

            // check if an event exists exactly at the starting tick of this segment
            if (nextGraphicalEvent != graphicalTempoMap->end() && nextGraphicalEvent->first == s->tick) {
                  currStartEvent = &nextGraphicalEvent->second;
                  nextGraphicalEvent++;

                  qDebug("currStartEvent:"); currStartEvent->dump();
            }
            else
                  currStartEvent = &zeroTEvent;      // indicates that there is no event at start of this segment

            //---------------------------------------------------------
            //    Now will merge the TEvents at edges of previous RepeatSegment and current RepeatSegment:
            //            prevEndEvent, the TEvent at the ending tick of the previous repeat segment
            //            currStartEvent, the TEvent at the starting tick of the current segment
            //
            //    Create mergedInitialEvent for the utick at the junction between the two concatenated repeat segments according to these rules:
            //
            //          always incur pause from breaths/caesuras prior to jumping:
            //                -Keep PAUSE_BEFORE_TICK at end of previous segment.
            //                -Ignore PAUSE_BEFORE_TICK at start of current segment.
            //
            //          never incur a pause from section breaks when jumping prior to encountering them:
            //                -Ignore PAUSE_THROUGH_TICK at end of previous segment.
            //                -Ignore PAUSE_THROUGH_TICK at start of current segment.
            //
            //          only use tempo from current segment:
            //                -Ignore tempo FIX at end of previous segment.
            //                -Keep tempo FIX at start of current segment.
            //
            //           if want to add support for fermatas (not implemented yet):
            //                -not implemented yet: Ignore PAUSE_AFTER_TICK at end of previous segment.
            //                -not implemented yet: Keep PAUSE_AFTER_TICK at start of current segment.
            //
            //           if want to add support for tempo RAMPs (not implemented yet):
            //                -not implemented yet: Ignore tempo RAMP at end of previous segment.
            //                -not implemented yet: Keep tempo RAMP at start of current segment.
            //---------------------------------------------------------
            mergedInitialEvent.type = (prevEndEvent->type & TempoType::PAUSE_BEFORE_TICK)
                                  | (currStartEvent->type & (TempoType::FIX|TempoType::RAMP));
            mergedInitialEvent.tempo = (currStartEvent->type & TempoType::FIX)? currStartEvent->tempo : 0.0;
            mergedInitialEvent.pauseBeforeTick = (prevEndEvent->type & TempoType::PAUSE_BEFORE_TICK)? prevEndEvent->pauseBeforeTick : 0.0;
            mergedInitialEvent.pauseThroughTick = 0.0;

            // insert this merged initial starting event at beginning tick of this segment
            // only insert if type is valid, since no point in inserting an event of TempoType::INVALID
            if (mergedInitialEvent.type) {
                  hintPositionInsert = insert(hintPositionInsert, std::pair<const int, TEvent> (utickOfEventToInsert, mergedInitialEvent));
                  qDebug("Inserted initial event at start of repeat segment."); mergedInitialEvent.dump();
            }

            // now handle rest of events in repeat segment
            while ( nextGraphicalEvent != graphicalTempoMap->lower_bound(s->tick + s->len) ) {

                  qDebug("Inserting nextGraphicalEvent:"); nextGraphicalEvent->second.dump();

                  // append the graphical event into unrolled tempomap
                  utickOfEventToInsert = s->utick + (nextGraphicalEvent->first - s->tick);
                  hintPositionInsert = insert(hintPositionInsert, std::pair<const int, TEvent> (utickOfEventToInsert, nextGraphicalEvent->second));

                  // if want to apply tempos that are triggered on specific repeats, based on some condition, then here would be the place to do it

                  qDebug("Inserted event."); //insertedUnrolledEvent->second.dump();

                  nextGraphicalEvent++;
                  }

            qDebug("FINISHED segment.\n");

            // check if an event exists exactly at the ending tick of this segment
            if (nextGraphicalEvent != graphicalTempoMap->end() && nextGraphicalEvent->first == s->tick+s->len) {
                  prevEndEvent = &nextGraphicalEvent->second; // needed to handle situation when need to apply pause just before the next repeat segment
                  qDebug("prevEndEvent:"); prevEndEvent->dump();
                  }
            else
                  prevEndEvent = &zeroTEvent;      // indicates that there is no event at end of segment

            }

      setRelTempo( graphicalTempoMap->relTempo() );      // use same relative tempo as graphical score & normalize

      qDebug("FINISHED unrolling TempoMap with repeatList & graphicalTempoMap. State of normalized unrolled tempomap:"); dump();
      }

//---------------------------------------------------------
//   setPauseBeforeTick
//    a PAUSE_BEFORE_TICK occurs just before reaching a tick.
//    (e.g. breaths/caesuras before a jump or repeat sign will pause playback
//    both when the jump/repeat is taken and when passing through the jump/repeat)
//---------------------------------------------------------

void TempoMap::setPauseBeforeTick(int tick, qreal seconds)
      {
      auto e = find(tick);
      if (e != end()) {
            e->second.pauseBeforeTick = seconds;
            e->second.type |= TempoType::PAUSE_BEFORE_TICK;
            }
      else {
            insert(std::pair<const int, TEvent> (tick, TEvent(tempo(tick), seconds, 0.0, TempoType::PAUSE_BEFORE_TICK)));
            }
      normalize();

 //     dump();
      }

//---------------------------------------------------------
//   setPauseThroughTick
//    a PAUSE_THROUGH_TICK only gets triggered when passing through a tick.
//    (e.g. a section break at a repeat sign will only pause playback when passing through the repeat sign)
//---------------------------------------------------------

void TempoMap::setPauseThroughTick(int tick, qreal seconds )
      {
      auto e = find(tick);
      if (e != end()) {
            e->second.pauseThroughTick = seconds;
            e->second.type |= TempoType::PAUSE_THROUGH_TICK;
            }
      else {
            insert(std::pair<const int, TEvent> (tick, TEvent(tempo(tick), 0.0, seconds, TempoType::PAUSE_THROUGH_TICK)));
            }
      normalize();

 //     dump();
      }

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void TempoMap::setTempo(int tick, qreal tempo)
      {
      auto e = find(tick);
      if (e != end()) {
            e->second.tempo = tempo;
            e->second.type |= TempoType::FIX;
            }
      else
            insert(std::pair<const int, TEvent> (tick, TEvent(tempo, 0.0, 0.0, TempoType::FIX)));
      normalize();

      qDebug( "setTempo( @tick %d, %f bps) finished. State of graphical tempomap:", tick, tempo);  dump();
      }

//---------------------------------------------------------
//   TempoMap::normalize
//---------------------------------------------------------

void TempoMap::normalize()
      {
      qreal time  = 0;
      int tick    = 0;
      qreal tempo = 2.0;
      for (auto e = begin(); e != end(); ++e) {

            // entries that represent a pause *only* without a tempo change (FIX or RAMP) need to continue previous tempo
            if ((e->second.type&(TempoType::PAUSE_ANY)) &&
               !(e->second.type&(TempoType::FIX|TempoType::RAMP)))
                  e->second.tempo = tempo;

            int delta = e->first - tick;
            time += qreal(delta) / (MScore::division * tempo * _relTempo);
            time += e->second.pauseBeforeTick + e->second.pauseThroughTick;
            e->second.time = time;
            tick  = e->first;
            tempo = e->second.tempo;

            }
      ++_tempoSN;
      }

//---------------------------------------------------------
//   TempoMap::dump
//---------------------------------------------------------

void TempoMap::dump() const
      {
      qDebug("TempoMap:   Tick | Time     | Type | Tempo    | PauseBefore | PauseThrough");
      for (auto i = begin(); i != end(); ++i) {

            char F = (i->second.type & TempoType::FIX)?   'F' : ' ';
            char R = (i->second.type & TempoType::RAMP)?  'R' : ' ';
            char B = (i->second.type & TempoType::PAUSE_BEFORE_TICK)?   'B' : ' ';
            char T = (i->second.type & TempoType::PAUSE_THROUGH_TICK)?  'T' : ' ';

            qDebug("          %6d | %8.2f | %c%c%c%c | %8.2f | %8.2f    | %8.2f",
               i->first, i->second.time, F, R, B, T, i->second.tempo, i->second.pauseBeforeTick, i->second.pauseThroughTick);
            }
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void TempoMap::clear()
      {
      std::map<int,TEvent>::clear();
      ++_tempoSN;
      }


//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

qreal TempoMap::tempo(int tick) const
      {
      if (empty())
            return 2.0;
      auto i = lower_bound(tick);
      if (i == end()) {
            --i;
            return i->second.tempo;
            }
      if (i->first == tick)
            return i->second.tempo;
      if (i == begin())
            return 2.0;
      --i;
      return i->second.tempo;
      }

//---------------------------------------------------------
//   del
//---------------------------------------------------------

void TempoMap::del(int tick)
      {
      auto e = find(tick);
      if (e == end()) {
            qDebug("TempoMap::del event at (%d): not found", tick);
            // abort();
            return;
            }

      // don't delete event if still being used for pause
      e->second.type = (e->second.type & (TempoType::PAUSE_BEFORE_TICK|TempoType::PAUSE_THROUGH_TICK));
      if (!e->second.type )
            erase(e);

      normalize();
      }

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void TempoMap::setRelTempo(qreal val)
      {
      _relTempo = val;
      normalize();
      }

//---------------------------------------------------------
//   delTempo
//---------------------------------------------------------

void TempoMap::delTempo(int tick)
      {
      del(tick);
      ++_tempoSN;
      }

//---------------------------------------------------------
//   tick2time
//---------------------------------------------------------

qreal TempoMap::tick2time(int tick, qreal time, int* sn) const
      {
      return (*sn == _tempoSN) ? time : tick2time(tick, sn);
      }

//---------------------------------------------------------
//   time2tick
//    return cached value t if list did not change
//---------------------------------------------------------

int TempoMap::time2tick(qreal time, int t, int* sn) const
      {
      return (*sn == _tempoSN) ? t : time2tick(time, sn);
      }

//---------------------------------------------------------
//   tick2time
//---------------------------------------------------------

qreal TempoMap::tick2time(int tick, int* sn) const
      {
      qreal time  = 0.0;
      qreal delta = qreal(tick);
      qreal tempo = 2.0;

      if (!empty()) {
            int ptick  = 0;
            auto e = lower_bound(tick);
            if (e == end()) {
                  auto pe = e;
                  --pe;
                  ptick = pe->first;
                  tempo = pe->second.tempo;
                  time  = pe->second.time;
                  }
            else if (e->first == tick) {
                  ptick = tick;
                  tempo = e->second.tempo;
                  time  = e->second.time;
                  }
            else if (e != begin()) {
                  auto pe = e;
                  --pe;
                  ptick = pe->first;
                  tempo = pe->second.tempo;
                  time  = pe->second.time;
                  }
            delta = qreal(tick - ptick);
            }
      else
            qDebug("TempoMap: empty");
      if (sn)
            *sn = _tempoSN;
      time += delta / (MScore::division * tempo * _relTempo);
      return time;
      }

//---------------------------------------------------------
//   time2tick
//---------------------------------------------------------

int TempoMap::time2tick(qreal time, int* sn) const
      {
      int   tickOfPreviousEvent = 0;
      qreal timeOfPreviousEvent = 0.0;
      qreal tempoOfPreviousEvent = 2.0;  // Why was _tempo overwritten with = 2.0?  _tempo is supposed to hold initial tempo, which should default to 2.0
      for (auto nextEvent = begin(); nextEvent != end(); ++nextEvent) {

            qreal timeOfNextEvent  = nextEvent->second.time;
            int   tickOfNextEvent  = nextEvent->first;
            qreal tempoOfNextEvent = nextEvent->second.tempo;
            qreal pauseOfNextEvent = nextEvent->second.pauseBeforeTick + nextEvent->second.pauseThroughTick;

            // done if found time inside a pause
            qreal timePriorToPause = timeOfNextEvent - pauseOfNextEvent;
            if ( (timePriorToPause < time) && (time <= timeOfNextEvent)) {
                  timeOfPreviousEvent += (time - timePriorToPause);
                  break;
                  }

            // done if found time before next event
            if (time <= timeOfNextEvent)
                  break;

            timeOfPreviousEvent  = timeOfNextEvent;
            tickOfPreviousEvent  = tickOfNextEvent;
            tempoOfPreviousEvent = tempoOfNextEvent;
            }

      qreal timeDelta = time - timeOfPreviousEvent;
      int tick = tickOfPreviousEvent + lrint(timeDelta * _relTempo * MScore::division * tempoOfPreviousEvent);

      if (sn)
            *sn = _tempoSN;

      return tick;
      }

}
