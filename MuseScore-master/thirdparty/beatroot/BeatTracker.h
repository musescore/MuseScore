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

#ifndef _BEAT_TRACKER_H_
#define _BEAT_TRACKER_H_

#include "Event.h"

#include <vector>


class AgentParameters;

namespace BeatTracker {

      /** Perform beat tracking.
       *  @param events The onsets or peaks in a feature list
       *  @param beats The initial beats which are given, if any
       *  @return The list of beats, or an empty list if beat tracking fails
       */
      std::vector<double> beatTrack(const AgentParameters &params,
                                    const EventList &events,
                                    const EventList &beats);

      /** Perform beat tracking.
       *  @param events The onsets or peaks in a feature list
       *  @return The list of beats, or an empty list if beat tracking fails
       */
      std::vector<double> beatTrack(const AgentParameters &params, const EventList &events);

      std::vector<double> beatTrack(const EventList &events);

} // namespace BeatTracker


#endif

