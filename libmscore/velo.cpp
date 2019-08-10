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

/**
 \file
 Implementation of class VeloList.
*/

#include "velo.h"

namespace Ms {

//---------------------------------------------------------
//   interpolateVelocity
///   the maths looks complex, but is just a series of graph transformations.
///   You can see these graphically at: https://www.desmos.com/calculator/kk89ficmjk
//---------------------------------------------------------

int VeloList::interpolateVelocity(VeloEvent& event, Fraction& tick)
      {
      Q_ASSERT(event.type == VeloType::RAMP);

      // Prevent zero-division error
      if (event.cachedStart == event.cachedEnd || event.tick == event.etick) {
            return event.cachedStart;
            }

      // Ticks to change expression over
      int exprTicks = (event.etick - event.tick).ticks();
      int exprDiff = event.cachedEnd - event.cachedStart;

      std::function<int(int)> valueFunction;
      switch (event.method) {
            case VeloChangeMethod::EXPONENTIAL:
                  // Due to the nth-root, exponential functions do not flip with negative values, and cause errors,
                  // so treat it as a piecewise function.
                  if (exprDiff > 0) {
                        valueFunction = [&](int ct) { return int(
                              pow(
                                    pow((exprDiff + 1), 1.0 / double(exprTicks)), // the exprTicks root of d+1
                                    double(ct)        // to the power of the current tick (exponential)
                                    ) - 1
                              ); };
                        }
                  else {
                        valueFunction = [&](int ct) { return -int(
                              pow(
                                    pow((-exprDiff + 1), 1.0 / double(exprTicks)), // the exprTicks root of 1-d
                                    double(ct)        // again to the power of ct
                                    ) + 1
                              ); };
                        }
                  break;
            // Uses sin x transformed, which _does_ flip with negative numbers
            case VeloChangeMethod::EASE_IN_OUT:
                  valueFunction = [&](int ct) { return int(
                        (double(exprDiff) / 2.0) * (
                              sin(
                                    double(ct) * (
                                          double(M_PI / double(exprTicks))
                                          ) - double(M_PI / 2.0)
                                    ) + 1
                              )
                        ); };
                  break;
            case VeloChangeMethod::EASE_IN:
                  valueFunction = [&](int ct) { return int(
                        double(exprDiff) * (
                              sin(
                                    double(ct - double(exprTicks)) * (
                                          double(M_PI / double(2 * exprTicks))
                                          )
                                    ) + 1
                              )
                        ); };
                  break;
            case VeloChangeMethod::EASE_OUT:
                  valueFunction = [&](int ct) { return int(
                        double(exprDiff) * sin(
                              double(ct) * (
                                    double(M_PI / double(2 * exprTicks))
                                    )
                              )
                        ); };
                  break;
            case VeloChangeMethod::NORMAL:
            default:
                  valueFunction = [&](int ct) { return int(
                        double(exprDiff) * (double(ct) / double(exprTicks))
                        ); };
                  break;
            }

      return event.cachedStart + valueFunction(tick.ticks() - event.tick.ticks());
      }

//---------------------------------------------------------
//   velo
//    return velocity at tick position
//---------------------------------------------------------

int VeloList::velo(Fraction tick)
      {
      if (!cleanedUp)
            cleanup();

      auto eventIter = upperBound(tick);
      if (eventIter == begin()) {
            return DEFAULT_VELOCITY;
            }

      eventIter--;
      bool foundRamp = false;
      VeloEvent& rampFound = eventIter.value();       // only used to init
      for (auto& event : values(eventIter.key())) {
            if (event.type == VeloType::RAMP) {
                  foundRamp = true;
                  rampFound = event;
                  }
            }

      if (!foundRamp) {
            // Last event must be a fix, since there are max two events at one tick
            return eventIter.value().value;
            }

      if (tick >= rampFound.etick) {
            return rampFound.cachedEnd;
            }
      else {
            // Do some maths!
            return interpolateVelocity(eventIter.value(), tick);
            }
      }

//---------------------------------------------------------
//   addFixed
//---------------------------------------------------------

void VeloList::addFixed(Fraction tick, int velocity)
      {
      insert(tick, VeloEvent(tick, velocity));
      cleanedUp = false;
      }

//---------------------------------------------------------
//   addRamp
///   the direction of the ramp *must* be explicitly passed, and the user-defined velocity change,
///   if any, must always be positive.
///   A `velChange` of 0 means that the change in velocity should be calculated from the next fixed
///   velocity event.
//---------------------------------------------------------

void VeloList::addRamp(Fraction stick, Fraction etick, int velChange, VeloChangeMethod method, VeloDirection direction)
      {
      int sign = direction == VeloDirection::CRESCENDO ? +1 : -1;
      insert(stick, VeloEvent(stick, etick, sign * velChange, method, direction));
      cleanedUp = false;
      }

//---------------------------------------------------------
//   cleanup
//---------------------------------------------------------

void VeloList::cleanup()
      {
      //
      // Stage 0: put the ramps in size order if they start at the same point
      //
      for (auto& tick : uniqueKeys()) {
            // `events` will contain all the ramps at this tick
            std::vector<VeloEvent> events;
            for (auto& event : values(tick)) {
                  if (event.type == VeloType::FIX)
                        continue;
                  events.push_back(event);
                  }

            if (int(events.size()) > 1) {
                  // new order is a map of the NEGATIVE length of a hairpin to a copy of its event
                  QMultiMap<Fraction, VeloEvent> newOrder;
                  for (auto& event : events) {
                        newOrder.insert(event.tick - event.etick, event);
                        // And clear out the events at this tick
                        remove(tick, event);
                        }

                  // newOrder's values are implicitally ordered by the key, which is the
                  // NEGATIVE length of the hairpin.
                  for (auto& event : newOrder.values()) {
                        insert(tick, event);
                        }
                  }
            }

      //
      // Stage 1: remove any ramps or fixes that are completely enclosed within other ramps
      //
      Fraction currentRampStart = Fraction(-1, 1);    // start point of ramp we're in
      Fraction currentRampEnd = Fraction(-1, 1);      // end point of ramp we're in
      Fraction lastFix = Fraction(-1, 1);             // the position of the last fix event
      bool inRamp = false;                            // whether we're in a ramp or not

      // Keep a record of the endpoints
      std::vector<std::pair<Fraction, Fraction>> endPoints;
      std::vector<bool> startsInHairpin;

      for (VeloEvent& event : values()) {
            // Reset if we've left the hairpin we were in
            if (currentRampEnd < event.tick)
                  inRamp = false;

            if (event.type == VeloType::RAMP) {
                  if (inRamp) {
                        if (event.etick <= currentRampEnd) {
                              // delete, this event is enveloped
                              remove(event.tick, event);
                              // don't add to the end points
                              continue;
                              }
                        else {
                              currentRampStart = event.tick;
                              currentRampEnd = event.etick;
                              startsInHairpin.push_back(true);
                              }
                        }
                  else {
                        currentRampStart = event.tick;
                        currentRampEnd = event.etick;
                        inRamp = true;
                        startsInHairpin.push_back(false);
                        }

                  endPoints.push_back(std::make_pair(event.tick, event.etick));
                  }
            else if (event.type == VeloType::FIX) {
                  if (inRamp) {
                        if (event.tick != currentRampStart && event.tick != currentRampEnd && lastFix != event.tick) {
                              // delete, this event is enveloped or at the same point as another fix
                              remove(event.tick, event);
                              continue;
                              }
                        }

                  lastFix = event.tick;
                  }
            }

      //
      // Stage 2: readjust lengths of any colliding ramps:
      //
      int j = -1;
      for (auto& event : values()) {
            if (event.type != VeloType::RAMP)
                  continue;

            j++;
            if (!startsInHairpin[j])
                  continue;

            // Take a copy of the event...
            auto tempEvent = event;
            remove(event.tick, event);
            tempEvent.tick = endPoints[j-1].second;
            // ...and move it to its new tick position
            insert(tempEvent.tick, tempEvent);
            }

      //
      // Stage 3: cache start and end values for each ramp
      //
      for (auto i = begin(); i != end(); i++) {
            auto& event = i.value();
            if (event.type != VeloType::RAMP)
                  continue;

            // Phase 1: cache a start value for the ramp
            // Try and get a fix at the tick of this ramp
            bool foundFix = false;
            for (auto& currentEvent : values(event.tick)) {
                  if (currentEvent.type == VeloType::FIX) {
                        event.cachedStart = currentEvent.value;
                        foundFix = true;
                        break;
                        }
                  }

            // If there isn't a fix, use from the last event:
            //  - the cached end value if it's a ramp
            //  - the value if it's a fix
            if (!foundFix) {
                  if (i != begin()) {
                        auto prevEventIter = i;
                        prevEventIter--;

                        // Look for a ramp first
                        bool foundRamp = false;
                        for (auto& prevEvent : values(prevEventIter.key())) {
                              if (prevEvent.type == VeloType::RAMP) {
                                    event.cachedStart = prevEvent.cachedEnd;
                                    foundRamp = true;
                                    break;
                                    }
                              }

                        if (!foundRamp) {
                              // prevEventIter must point to a fix in this case
                              event.cachedStart = prevEventIter.value().value;
                              }
                        }
                  else {
                        event.cachedStart = DEFAULT_VELOCITY;
                        }
                  }

            // Phase 2: cache an end value for the ramp
            // If there's no set velocity change:
            if (event.value == 0) {
                  auto nextEventIter = upperBound(event.tick);

                  // If this is the last event, there is no change
                  if (nextEventIter == end()) {
                        event.cachedEnd = event.cachedStart;
                        }
                  else {
                        // Search for a fixed event at the next event point
                        bool foundFix = false;
                        for (auto& nextEvent : values(nextEventIter.key())) {
                              if (nextEvent.type == VeloType::FIX) {
                                    event.cachedEnd = nextEvent.value;
                                    foundFix = true;
                                    break;
                                    }
                              }

                        // We haven't found a fix, so there must be a ramp. What does the user want?
                        // A good guess would to be to interpolate, but that might get complex, so just ignore
                        // this hairpin and set the ending to be the same as the start.
                        // TODO: implementing some form of smart interpolation would be nice.
                        if (!foundFix) {
                              event.cachedEnd = event.cachedStart;
                              }
                        }
                  }
            else {
                  event.cachedEnd = event.cachedStart + event.value;
                  }

            // And finally... if something's wrong, make it not wrong
            if ((event.cachedStart > event.cachedEnd && event.direction == VeloDirection::CRESCENDO) ||
                (event.cachedStart < event.cachedEnd && event.direction == VeloDirection::DIMINUENDO)) {
                  event.cachedEnd = event.cachedStart;
                  }
            }

      cleanedUp = true;
      }

//---------------------------------------------------------
//   changesInRange
///   returns a list of changes in a range, and their start and end points
//---------------------------------------------------------

std::vector<std::pair<Fraction, Fraction>> VeloList::changesInRange(Fraction stick, Fraction etick)
      {
      if (!cleanedUp)
            cleanup();

      std::vector<std::pair<Fraction, Fraction>> tempChanges;

      // Force a new event on every noteon, in case the velocity has changed
      tempChanges.push_back(std::make_pair(stick, stick));
      for (auto iter = lowerBound(stick); iter != end(); iter++) {
            if (iter.value().tick > etick)
                  break;

            auto& event = iter.value();
            if (event.type == VeloType::FIX)
                  tempChanges.push_back(std::make_pair(event.tick, event.tick));
            else if (event.type == VeloType::RAMP) {
                  Fraction useEtick = event.etick > etick ? etick : event.etick;
                  tempChanges.push_back(std::make_pair(event.tick, useEtick));
                  }
            }

      // And also go back one and try to find hairpin coming into this range
      auto iter = lowerBound(stick);
      if (iter != begin()) {
            iter--;
            auto& event = iter.value();
            if (event.type == VeloType::RAMP) {
                  if (event.etick > stick) {
                        tempChanges.push_back(std::make_pair(stick, event.etick));
                        }
                  }
            }
      return tempChanges;
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void VeloList::dump()
      {
      qDebug("\n\n=== VeloList: dump ===");
      for (auto& event : values()) {
            if (event.type == VeloType::FIX) {
                  qDebug().nospace() << "===" << event.tick.ticks() << " : FIX " << event.value;
                  }
            else if (event.type == VeloType::RAMP) {
                  qDebug().nospace() << "===" << event.tick.ticks() << " to " << event.etick.ticks() << " : RAMP diff " << event.value << " " << Hairpin::veloChangeMethodToName(event.method) << " (" << event.cachedStart << ", " << event.cachedEnd << ")";
                  }
            }
      qDebug("=== VeloList: dump end ===\n\n");
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool VeloEvent::operator==(const VeloEvent& event) const
      {
      return (
            tick == event.tick &&
            value == event.value &&
            type == event.type &&
            etick == event.etick &&
            method == event.method &&
            direction == event.direction
      );
      }

bool VeloEvent::operator!=(const VeloEvent& event) const
      {
      return !(operator==(event));
      }

}

