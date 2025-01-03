/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
  Vamp feature extraction plugin for the BeatRoot beat tracker.

  Centre for Digital Music, Queen Mary, University of London.
  This file copyright 2011 Simon Dixon, Chris Cannam and QMUL.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.  See the file
  COPYING included with this distribution for more information.
*/

#include "BeatTracker.h"
#include "Agent.h"
#include "AgentList.h"
#include "Induction.h"


namespace BeatTracker {

std::vector<double> beatTrack(const AgentParameters &params,
                              const EventList &events,
                              const EventList &beats)
      {
      AgentList agents;
      int count = 0;
      double beatTime = -1;
      if (!beats.empty()) {
            count = beats.size() - 1;
            EventList::const_iterator itr = beats.end();
            --itr;
            beatTime = itr->time;
            }
      if (count > 0) {        // tempo given by mean of initial beats
            double ioi = (beatTime - beats.begin()->time) / count;
            agents.push_back(new Agent(params, ioi));
            }
      else            // tempo not given; use tempo induction
            agents = Induction::beatInduction(params, events);

      if (!beats.empty()) {
            for (AgentList::iterator itr = agents.begin(); itr != agents.end(); ++itr) {
                  (*itr)->beatTime = beatTime;
                  (*itr)->beatCount = count;
                  (*itr)->events = beats;
                  }
            }
      agents.beatTrack(events, params, -1);

      Agent *best = agents.bestAgent();
      std::vector<double> resultBeatTimes;

      if (best) {
            best->fillBeats(beatTime);
            for (EventList::const_iterator itr = best->events.begin();
                        itr != best->events.end(); ++itr) {
                  resultBeatTimes.push_back(itr->time);
                  }
            }
      for (AgentList::iterator ai = agents.begin(); ai != agents.end(); ++ai) {
            delete *ai;
            }

      return resultBeatTimes;
      }

std::vector<double> beatTrack(const AgentParameters &params, const EventList &events)
      {
      return beatTrack(params, events, EventList());
      }

std::vector<double> beatTrack(const EventList &events)
      {
      return beatTrack(AgentParameters(), events);
      }

} // namespace BeatTracker


