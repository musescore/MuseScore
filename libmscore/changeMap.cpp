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
 Implementation of class ChangeMap.
*/

#include "changeMap.h"

namespace Ms {

//---------------------------------------------------------
//   interpolateVelocity
///   the maths looks complex, but is just a series of graph transformations.
///   You can see these graphically at: https://www.desmos.com/calculator/kk89ficmjk
//---------------------------------------------------------

int ChangeMap::interpolate(ChangeEvent& event, Fraction& tick)
      {
      Q_ASSERT(event.type == ChangeEventType::RAMP);

      // Prevent zero-division error
      if (event.cachedStart == event.cachedEnd || event.tick == event.etick) {
            return event.cachedStart;
            }

      // Ticks to change expression over
      int exprTicks = (event.etick - event.tick).ticks();
      int exprDiff = event.cachedEnd - event.cachedStart;

      std::function<int(int)> valueFunction;
      switch (event.method) {
            case ChangeMethod::EXPONENTIAL:
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
            case ChangeMethod::EASE_IN_OUT:
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
            case ChangeMethod::EASE_IN:
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
            case ChangeMethod::EASE_OUT:
                  valueFunction = [&](int ct) { return int(
                        double(exprDiff) * sin(
                              double(ct) * (
                                    double(M_PI / double(2 * exprTicks))
                                    )
                              )
                        ); };
                  break;
            case ChangeMethod::NORMAL:
            default:
                  valueFunction = [&](int ct) { return int(
                        double(exprDiff) * (double(ct) / double(exprTicks))
                        ); };
                  break;
            }

      return event.cachedStart + valueFunction(tick.ticks() - event.tick.ticks());
      }

//---------------------------------------------------------
//   val
///   return value at tick position. Do not confuse with
///   `value`, which is a method of QMultiMap.
//---------------------------------------------------------

int ChangeMap::val(Fraction tick)
      {
      if (!cleanedUp)
            cleanup();

      auto eventIter = upperBound(tick);
      if (eventIter == begin()) {
            return DEFAULT_VALUE;
            }

      eventIter--;
      bool foundRamp = false;
      ChangeEvent& rampFound = eventIter.value();       // only used to init
      for (auto& event : values(eventIter.key())) {
            if (event.type == ChangeEventType::RAMP) {
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
            return interpolate(eventIter.value(), tick);
            }
      }

//---------------------------------------------------------
//   addFixed
//---------------------------------------------------------

void ChangeMap::addFixed(Fraction tick, int value)
      {
      insert(tick, ChangeEvent(tick, value));
      cleanedUp = false;
      }

//---------------------------------------------------------
//   addRamp
///   A `change` of 0 means that the change in velocity should be calculated from the next fixed
///   velocity event.
//---------------------------------------------------------

void ChangeMap::addRamp(Fraction stick, Fraction etick, int change, ChangeMethod method, ChangeDirection direction)
      {
      change = abs(change);
      change *= (direction == ChangeDirection::INCREASING) ? 1 : -1;
      insert(stick, ChangeEvent(stick, etick, change, method, direction));
      cleanedUp = false;
      }

//---------------------------------------------------------
//   cleanup
//---------------------------------------------------------

void ChangeMap::cleanup()
      {
      //
      // Stage 0: put the ramps in size order if they start at the same point
      //
      for (auto& tick : uniqueKeys()) {
            // `events` will contain all the ramps at this tick
            std::vector<ChangeEvent> events;
            for (auto& event : values(tick)) {
                  if (event.type == ChangeEventType::FIX)
                        continue;
                  events.push_back(event);
                  }

            if (int(events.size()) > 1) {
                  // new order is a map of the NEGATIVE length of a ramp to a copy of its event
                  QMultiMap<Fraction, ChangeEvent> newOrder;
                  for (auto& event : events) {
                        newOrder.insert(event.tick - event.etick, event);
                        // And clear out the events at this tick
                        remove(tick, event);
                        }

                  // newOrder's values are implicitally ordered by the key, which is the
                  // NEGATIVE length of the ramp.
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
      std::vector<bool> startsInRamp;

      for (ChangeEvent& event : values()) {
            // Reset if we've left the ramp we were in
            if (currentRampEnd < event.tick)
                  inRamp = false;

            if (event.type == ChangeEventType::RAMP) {
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
                              startsInRamp.push_back(true);
                              }
                        }
                  else {
                        currentRampStart = event.tick;
                        currentRampEnd = event.etick;
                        inRamp = true;
                        startsInRamp.push_back(false);
                        }

                  endPoints.push_back(std::make_pair(event.tick, event.etick));
                  }
            else if (event.type == ChangeEventType::FIX) {
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
            if (event.type != ChangeEventType::RAMP)
                  continue;

            j++;
            if (!startsInRamp[j])
                  continue;

            // Take a copy of the event...
            auto tempChangeEvent = event;
            remove(event.tick, event);
            tempChangeEvent.tick = endPoints[j-1].second;
            // ...and move it to its new tick position
            insert(tempChangeEvent.tick, tempChangeEvent);
            }

      //
      // Stage 3: cache start and end values for each ramp
      //
      for (auto i = begin(); i != end(); i++) {
            auto& event = i.value();
            if (event.type != ChangeEventType::RAMP)
                  continue;

            // Phase 1: cache a start value for the ramp
            // Try and get a fix at the tick of this ramp
            bool foundFix = false;
            for (auto& currentChangeEvent : values(event.tick)) {
                  if (currentChangeEvent.type == ChangeEventType::FIX) {
                        event.cachedStart = currentChangeEvent.value;
                        foundFix = true;
                        break;
                        }
                  }

            // If there isn't a fix, use from the last event:
            //  - the cached end value if it's a ramp
            //  - the value if it's a fix
            if (!foundFix) {
                  if (i != begin()) {
                        auto prevChangeEventIter = i;
                        prevChangeEventIter--;

                        // Look for a ramp first
                        bool foundRamp = false;
                        for (auto& prevChangeEvent : values(prevChangeEventIter.key())) {
                              if (prevChangeEvent.type == ChangeEventType::RAMP) {
                                    event.cachedStart = prevChangeEvent.cachedEnd;
                                    foundRamp = true;
                                    break;
                                    }
                              }

                        if (!foundRamp) {
                              // prevChangeEventIter must point to a fix in this case
                              event.cachedStart = prevChangeEventIter.value().value;
                              }
                        }
                  else {
                        event.cachedStart = DEFAULT_VALUE;
                        }
                  }

            // Phase 2: cache an end value for the ramp
            // If there's no set velocity change:
            if (event.value == 0) {
                  auto nextChangeEventIter = upperBound(event.tick);

                  // If this is the last event, there is no change
                  if (nextChangeEventIter == end()) {
                        event.cachedEnd = event.cachedStart;
                        }
                  else {
                        // Search for a fixed event at the next event point
                        bool foundFix = false;
                        for (auto& nextChangeEvent : values(nextChangeEventIter.key())) {
                              if (nextChangeEvent.type == ChangeEventType::FIX) {
                                    event.cachedEnd = nextChangeEvent.value;
                                    foundFix = true;
                                    break;
                                    }
                              }

                        // We haven't found a fix, so there must be a ramp. What does the user want?
                        // A good guess would to be to interpolate, but that might get complex, so just ignore
                        // this ramp and set the ending to be the same as the start.
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
            if ((event.cachedStart > event.cachedEnd && event.direction == ChangeDirection::INCREASING) ||
                (event.cachedStart < event.cachedEnd && event.direction == ChangeDirection::DECREASING)) {
                  event.cachedEnd = event.cachedStart;
                  }
            }

      cleanedUp = true;
      }

//---------------------------------------------------------
//   changesInRange
///   returns a list of changes in a range, and their start and end points
//---------------------------------------------------------

std::vector<std::pair<Fraction, Fraction>> ChangeMap::changesInRange(Fraction stick, Fraction etick)
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
            if (event.type == ChangeEventType::FIX)
                  tempChanges.push_back(std::make_pair(event.tick, event.tick));
            else if (event.type == ChangeEventType::RAMP) {
                  Fraction useEtick = event.etick > etick ? etick : event.etick;
                  tempChanges.push_back(std::make_pair(event.tick, useEtick));
                  }
            }

      // And also go back one and try to find ramp coming into this range
      auto iter = lowerBound(stick);
      if (iter != begin()) {
            iter--;
            auto& event = iter.value();
            if (event.type == ChangeEventType::RAMP) {
                  if (event.etick > stick) {
                        tempChanges.push_back(std::make_pair(stick, event.etick));
                        }
                  }
            }
      return tempChanges;
      }

//---------------------------------------------------------
//   changeMethodTable
//---------------------------------------------------------

const std::vector<ChangeMap::ChangeMethodItem> ChangeMap::changeMethodTable {
      { ChangeMethod::NORMAL,           "normal"      },
      { ChangeMethod::EASE_IN,          "ease-in"     },
      { ChangeMethod::EASE_OUT,         "ease-out"    },
      { ChangeMethod::EASE_IN_OUT,      "ease-in-out" },
      { ChangeMethod::EXPONENTIAL,      "exponential" },
      };

//---------------------------------------------------------
//   changeMethodToName
//---------------------------------------------------------

QString ChangeMap::changeMethodToName(ChangeMethod method)
      {
      for (auto i : ChangeMap::changeMethodTable) {
            if (i.method == method)
                  return i.name;
            }
      qFatal("Unrecognised change method!");
      return "none"; // silence a compiler warning
      }

//---------------------------------------------------------
//   nameToChangeMethod
//---------------------------------------------------------

ChangeMethod ChangeMap::nameToChangeMethod(QString name)
      {
      for (auto i : ChangeMap::changeMethodTable) {
            if (i.name == name)
                  return i.method;
            }
      return ChangeMethod::NORMAL;   // default
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void ChangeMap::dump()
      {
      qDebug("\n\n=== ChangeMap: dump ===");
      for (auto& event : values()) {
            if (event.type == ChangeEventType::FIX) {
                  qDebug().nospace() << "===" << event.tick.ticks() << " : FIX " << event.value;
                  }
            else if (event.type == ChangeEventType::RAMP) {
                  qDebug().nospace() << "===" << event.tick.ticks() << " to " << event.etick.ticks() << " : RAMP diff " << event.value << " " /*<< Hairpin::veloChangeMethodToName(event.method)*/ << " (" << event.cachedStart << ", " << event.cachedEnd << ")";
                  }
            }
      qDebug("=== ChangeMap: dump end ===\n\n");
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool ChangeEvent::operator==(const ChangeEvent& event) const
      {
      return (
            tick == event.tick &&
            value == event.value &&
            type == event.type &&
            etick == event.etick &&
            method == event.method
      );
      }

bool ChangeEvent::operator!=(const ChangeEvent& event) const
      {
      return !(operator==(event));
      }

}

