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

#include "Agent.h"
#include "BeatTracker.h"
#include "AgentList.h"

#include <cmath>


const double AgentParameters::DEFAULT_POST_MARGIN_FACTOR = 0.3;
const double AgentParameters::DEFAULT_PRE_MARGIN_FACTOR = 0.15;
const double AgentParameters::DEFAULT_MAX_CHANGE = 0.2;
const double AgentParameters::DEFAULT_EXPIRY_TIME = 10.0;

const double Agent::INNER_MARGIN = 0.040;
const double Agent::CONF_FACTOR = 0.5;
const double Agent::DEFAULT_CORRECTION_FACTOR = 50.0;

int Agent::idCounter = 0;


Agent::Agent(const AgentParameters &params, double ibi)
      : idNumber(idCounter++)
      , phaseScore(0.0)
      , beatCount(0)
      , beatInterval(ibi)
      , initialBeatInterval(ibi)
      , beatTime(-1.0)
      , maxChange(params.maxChange)
      , preMargin(ibi * params.preMarginFactor)
      , postMargin(ibi * params.postMarginFactor)
      , innerMargin(INNER_MARGIN)
      , correctionFactor(DEFAULT_CORRECTION_FACTOR)
      , expiryTime(params.expiryTime)
      {
      }

Agent *Agent::clone() const
      {
      Agent *a = new Agent(*this);
      a->idNumber = idCounter++;
      return a;
      }

void Agent::accept(const BeatTracker::Event &e, double err, int beats)
      {
      beatTime = e.time;
      events.push_back(e);
      if (std::fabs(initialBeatInterval - beatInterval - err / correctionFactor)
                  < maxChange * initialBeatInterval) {
            beatInterval += err / correctionFactor;         // Adjust tempo
            }
      beatCount += beats;
      double conFactor = 1.0 - CONF_FACTOR * err
                  / (err > 0 ? postMargin: -preMargin);
      phaseScore += conFactor * e.salience;
      }

bool Agent::considerAsBeat(const BeatTracker::Event &e, AgentList &a)
      {
      if (beatTime < 0) {	// first event
            accept(e, 0, 1);
            return true;
            }
      else {			// subsequent events
            BeatTracker::EventList::iterator last = events.end();
            --last;
            if (e.time - last->time > expiryTime) {
                  phaseScore = -1.0;	// flag agent to be deleted
                  return false;
                  }
            double beats = nearbyint((e.time - beatTime) / beatInterval);
            double err = e.time - beatTime - beats * beatInterval;
            if ((beats > 0) && (-preMargin <= err) && (err <= postMargin)) {
                  if (std::fabs(err) > innerMargin) {
                                    // Create new agent that skips this event (avoids
                                    // large phase jump)
                        a.add(clone());
                        }
                  accept(e, err, (int)beats);
                  return true;
                  }
            }
      return false;
      }

void Agent::fillBeats(double start)
      {
      BeatTracker::EventList::iterator it = events.begin();
      if (it == events.end())
            return;
      double prevBeat = it->time;
      for (++it; it != events.end(); ++it) {
            double nextBeat = it->time;
            double beats = nearbyint((nextBeat - prevBeat) / beatInterval - 0.01);   // prefer slow
            double currentInterval = (nextBeat - prevBeat) / beats;
            for ( ; (nextBeat > start) && (beats > 1.5); --beats) {
                  prevBeat += currentInterval;
                  events.insert(it, BeatTracker::Event(prevBeat, 0));
                  }
            prevBeat = nextBeat;
            }
      }
