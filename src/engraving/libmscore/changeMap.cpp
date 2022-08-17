/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 \file
 Implementation of class ChangeMap.
*/

#include "changeMap.h"

#include <cmath>
#include <cassert>

#include "containers.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   interpolateVelocity
///   the maths looks complex, but is just a series of graph transformations.
///   You can see these graphically at: https://www.desmos.com/calculator/kk89ficmjk
//---------------------------------------------------------

int ChangeMap::interpolate(Fraction& eventTick, ChangeEvent& event, Fraction& tick)
{
    assert(event.type == ChangeEventType::RAMP);

    // Prevent zero-division error
    if (event.cachedStartVal == event.cachedEndVal || event.length.isZero()) {
        return event.cachedStartVal;
    }

    // Ticks to change expression over
    int exprTicks = event.length.ticks();
    int exprDiff = event.cachedEndVal - event.cachedStartVal;

    std::function<int(int)> valueFunction;
    switch (event.method) {
    case ChangeMethod::EXPONENTIAL:
        // Due to the nth-root, exponential functions do not flip with negative values, and cause errors,
        // so treat it as a piecewise function.
        if (exprDiff > 0) {
            valueFunction = [&](int ct) {
                return int(
                    pow(
                        pow((exprDiff + 1), 1.0 / double(exprTicks)),                 // the exprTicks root of d+1
                        double(ct)                    // to the power of the current tick (exponential)
                        ) - 1
                    );
            };
        } else {
            valueFunction = [&](int ct) {
                return -int(
                    pow(
                        pow((-exprDiff + 1), 1.0 / double(exprTicks)),                 // the exprTicks root of 1-d
                        double(ct)                    // again to the power of ct
                        ) + 1
                    );
            };
        }
        break;
    // Uses sin x transformed, which _does_ flip with negative numbers
    case ChangeMethod::EASE_IN_OUT:
        valueFunction = [&](int ct) {
            return int(
                (double(exprDiff) / 2.0) * (
                    sin(
                        double(ct) * (
                            double(M_PI / double(exprTicks))
                            ) - double(M_PI / 2.0)
                        ) + 1
                    )
                );
        };
        break;
    case ChangeMethod::EASE_IN:
        valueFunction = [&](int ct) {
            return int(
                double(exprDiff) * (
                    sin(
                        double(ct - double(exprTicks)) * (
                            double(M_PI / double(2 * exprTicks))
                            )
                        ) + 1
                    )
                );
        };
        break;
    case ChangeMethod::EASE_OUT:
        valueFunction = [&](int ct) {
            return int(
                double(exprDiff) * sin(
                    double(ct) * (
                        double(M_PI / double(2 * exprTicks))
                        )
                    )
                );
        };
        break;
    case ChangeMethod::NORMAL:
    default:
        valueFunction = [&](int ct) {
            return int(
                double(exprDiff) * (double(ct) / double(exprTicks))
                );
        };
        break;
    }

    return event.cachedStartVal + valueFunction(tick.ticks() - eventTick.ticks());
}

//---------------------------------------------------------
//   val
///   return value at tick position.
//---------------------------------------------------------

int ChangeMap::val(Fraction tick)
{
    if (!cleanedUp) {
        cleanup();
    }

    auto eventIter = upper_bound(tick);
    if (eventIter == begin()) {
        return DEFAULT_VALUE;
    }

    eventIter--;
    bool foundRamp = false;
    ChangeEvent& rampFound = eventIter->second;         // only used to init
    Fraction rampFoundStartTick = eventIter->first;
    auto values = this->equal_range(rampFoundStartTick);
    for (auto it = values.first; it != values.second; ++it) {
        auto& event = it->second;
        if (event.type == ChangeEventType::RAMP) {
            foundRamp = true;
            rampFound = event;
        }
    }

    if (!foundRamp) {
        // Last event must be a fix, since there are max two events at one tick
        return eventIter->second.value;
    }

    if (tick >= (rampFoundStartTick + rampFound.length)) {
        return rampFound.cachedEndVal;
    } else {
        // Do some maths!
        return interpolate(rampFoundStartTick, rampFound, tick);        // NOTE:JT check should rampFound be eventIter.value()
    }
}

//---------------------------------------------------------
//   addFixed
//---------------------------------------------------------

void ChangeMap::addFixed(Fraction tick, int value)
{
    insert({ tick, ChangeEvent(value) });
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
    insert({ stick, ChangeEvent(stick, etick, change, method, direction) });
    cleanedUp = false;
}

//---------------------------------------------------------
//   cleanupStage0
///   put the ramps in size order if they start at the same point
//---------------------------------------------------------

void ChangeMap::cleanupStage0()
{
    for (auto& tick : mu::uniqueKeys(*this)) {
        // rampEvents will contain all the ramps at this tick
        std::vector<ChangeEvent> rampEvents;
        auto values = this->equal_range(tick);
        for (auto it = values.first; it != values.second; ++it) {
            auto& event = it->second;
            if (event.type == ChangeEventType::FIX) {
                continue;
            }
            rampEvents.push_back(event);
        }

        if (int(rampEvents.size()) > 1) {
            // Sort rampEvents so that the longest ramps come first -
            // this is important for when we remove ramps/fixes enclosed within other
            // ramps during stage 1.
            std::sort(rampEvents.begin(), rampEvents.end(), ChangeMap::compareRampEvents);
            for (auto& event : rampEvents) {
                insert({ tick, event });
            }
        }
    }
}

//---------------------------------------------------------
//   cleanupStage1
///   remove any ramps or fixes that are completely enclosed within other ramps
//---------------------------------------------------------

void ChangeMap::cleanupStage1()
{
    Fraction currentRampStart = Fraction(-1, 1);      // start point of ramp we're in
    Fraction currentRampEnd = Fraction(-1, 1);        // end point of ramp we're in
    Fraction lastFix = Fraction(-1, 1);               // the position of the last fix event
    bool inRamp = false;                              // whether we're in a ramp or not

    // Keep a record of the endpoints
    EndPointsVector endPoints;
    std::vector<bool> startsInRamp;

    auto i = begin();
    while (i != end()) {
        Fraction tick = i->first;
        ChangeEvent& event = i->second;
        Fraction etick = tick + event.length;

        // Reset if we've left the ramp we were in
        if (currentRampEnd < tick) {
            inRamp = false;
        }

        if (event.type == ChangeEventType::RAMP) {
            if (inRamp) {
                if (etick <= currentRampEnd) {
                    // delete, this event is enveloped
                    i = erase(i);
                    // don't add to the end points
                    continue;
                } else {
                    currentRampStart = tick;
                    currentRampEnd = etick;
                    startsInRamp.push_back(true);
                }
            } else {
                currentRampStart = tick;
                currentRampEnd = etick;
                inRamp = true;
                startsInRamp.push_back(false);
            }

            endPoints.push_back(std::make_pair(tick, etick));
        } else if (event.type == ChangeEventType::FIX) {
            if (inRamp) {
                if (tick != currentRampStart && tick != currentRampEnd && lastFix != tick) {
                    // delete, this event is enveloped or at the same point as another fix
                    i = erase(i);
                    continue;
                }
            }

            lastFix = tick;
        }

        i++;
    }

    cleanupStage2(startsInRamp, endPoints);
}

//---------------------------------------------------------
//   cleanupStage2
///   readjust lengths of any colliding ramps
//---------------------------------------------------------

void ChangeMap::cleanupStage2(std::vector<bool>& startsInRamp, EndPointsVector& endPoints)
{
    // moveTo stores the events that need to be moved to a Fraction position
    std::map<Fraction, ChangeEvent> moveTo;
    auto i = begin();
    int j = -1;
    while (i != end()) {
        Fraction tick = i->first;
        ChangeEvent& event = i->second;
        if (event.type != ChangeEventType::RAMP) {
            i++;
            continue;
        }

        j++;
        if (!startsInRamp[j]) {
            i++;
            continue;
        }

        // Take a copy of the event and remove it
        Fraction newTick = endPoints[j - 1].second;
        event.length -= (newTick - tick);
        moveTo[newTick] = event;
        i = erase(i);
    }

    // Re-insert the events that we need to move in their new positions
    for (auto k = moveTo.begin(); k != moveTo.end(); k++) {
        insert({ k->first, k->second });
    }
}

//---------------------------------------------------------
//   cleanupStage3
///   cache start and end values for each ramp
//---------------------------------------------------------

void ChangeMap::cleanupStage3()
{
    for (auto i = begin(); i != end(); i++) {
        Fraction tick = i->first;
        auto& event = i->second;
        if (event.type != ChangeEventType::RAMP) {
            continue;
        }

        // Phase 1: cache a start value for the ramp
        // Try and get a fix at the tick of this ramp
        bool foundFix = false;
        auto values = this->equal_range(tick);
        for (auto it = values.first; it != values.second; ++it) {
            auto& currentChangeEvent = it->second;
            if (currentChangeEvent.type == ChangeEventType::FIX) {
                event.cachedStartVal = currentChangeEvent.value;
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
                auto prevValues = this->equal_range(prevChangeEventIter->first);
                for (auto it = prevValues.first; it != prevValues.second; ++it) {
                    auto& prevChangeEvent = it->second;
                    if (prevChangeEvent.type == ChangeEventType::RAMP) {
                        event.cachedStartVal = prevChangeEvent.cachedEndVal;
                        foundRamp = true;
                        break;
                    }
                }

                if (!foundRamp) {
                    // prevChangeEventIter must point to a fix in this case
                    event.cachedStartVal = prevChangeEventIter->second.value;
                }
            } else {
                event.cachedStartVal = DEFAULT_VALUE;
            }
        }

        // Phase 2: cache an end value for the ramp
        // If there's no set velocity change:
        if (event.value == 0) {
            auto nextChangeEventIter = i;
            nextChangeEventIter++;
            // There's a chance that the next event is a fix at the same tick as the
            // start of the current ramp. If so, get the next event, which is assured
            // to be a different (larger) tick
            if (nextChangeEventIter != end() && nextChangeEventIter->first == tick) {
                nextChangeEventIter++;
            }

            // If this is the last event, there is no change
            if (nextChangeEventIter == end()) {
                event.cachedEndVal = event.cachedStartVal;
            } else {
                // Search for a fixed event at the next event point
                bool foundFix2 = false;
                auto nextValues = this->equal_range(nextChangeEventIter->first);
                for (auto it = nextValues.first; it != nextValues.second; ++it) {
                    auto& nextChangeEvent = it->second;
                    if (nextChangeEvent.type == ChangeEventType::FIX) {
                        event.cachedEndVal = nextChangeEvent.value;
                        foundFix2 = true;
                        break;
                    }
                }

                // We haven't found a fix, so there must be a ramp. What does the user want?
                // A good guess would to be to interpolate, but that might get complex, so just ignore
                // this ramp and set the ending to be the same as the start.
                // TODO: implementing some form of smart interpolation would be nice.
                if (!foundFix2) {
                    event.cachedEndVal = event.cachedStartVal;
                }
            }
        } else {
            event.cachedEndVal = event.cachedStartVal + event.value;
        }

        // And finally... if something's wrong, make it not wrong
        if ((event.cachedStartVal > event.cachedEndVal && event.direction == ChangeDirection::INCREASING)
            || (event.cachedStartVal < event.cachedEndVal && event.direction == ChangeDirection::DECREASING)) {
            event.cachedEndVal = event.cachedStartVal;
        }
    }
}

//---------------------------------------------------------
//   cleanup
//---------------------------------------------------------

void ChangeMap::cleanup()
{
    if (cleanedUp) {
        return;
    }

    // LOGD() << "Before cleanup:";
    // dump();

    cleanupStage0();
    cleanupStage1();
    cleanupStage3();
    cleanedUp = true;

    // LOGD() << "After cleanup:";
    // dump();
}

//---------------------------------------------------------
//   changesInRange
///   returns a list of changes in a range, and their start and end points
//---------------------------------------------------------

std::vector<std::pair<Fraction, Fraction> > ChangeMap::changesInRange(Fraction stick, Fraction etick)
{
    if (!cleanedUp) {
        cleanup();
    }

    std::vector<std::pair<Fraction, Fraction> > tempChanges;

    // Force a new event on every noteon, in case the velocity has changed
    tempChanges.push_back(std::make_pair(stick, stick));
    for (auto iter = lower_bound(stick); iter != end(); iter++) {
        Fraction tick = iter->first;
        if (tick > etick) {
            break;
        }

        auto& event = iter->second;
        if (event.type == ChangeEventType::FIX) {
            tempChanges.push_back(std::make_pair(tick, tick));
        } else if (event.type == ChangeEventType::RAMP) {
            Fraction eventEtick = tick + event.length;
            Fraction useEtick = eventEtick > etick ? etick : eventEtick;
            tempChanges.push_back(std::make_pair(tick, useEtick));
        }
    }

    // And also go back one and try to find ramp coming into this range
    auto iter = lower_bound(stick);
    if (iter != begin()) {
        iter--;
        auto& event = iter->second;
        if (event.type == ChangeEventType::RAMP) {
            Fraction eventEtick = iter->first + event.length;
            if (eventEtick > stick) {
                tempChanges.push_back(std::make_pair(stick, eventEtick));
            }
        }
    }
    return tempChanges;
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool ChangeEvent::operator==(const ChangeEvent& event) const
{
    return
        value == event.value
        && type == event.type
        && length == event.length
        && method == event.method
        && direction == event.direction
    ;
}
}
