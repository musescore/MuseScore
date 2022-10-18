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

#include "spannermap.h"
#include "spanner.h"
#include "part.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   SpannerMap
//---------------------------------------------------------

SpannerMap::SpannerMap()
    : std::multimap<int, Spanner*>()
{
    dirty = true;
}

//---------------------------------------------------------
//   update
//   updates the internal lookup tree, not the map itself
//---------------------------------------------------------

void SpannerMap::update() const
{
    IntervalList regularIntervals;
    IntervalList collisionFreeIntervals;

    collectIntervals(regularIntervals, collisionFreeIntervals);

    tree = interval_tree::IntervalTree<Spanner*>(regularIntervals);
    collisionFreeTree = interval_tree::IntervalTree<Spanner*>(collisionFreeIntervals);
    dirty = false;
}

//---------------------------------------------------------
//   findContained
//---------------------------------------------------------

const SpannerMap::IntervalList& SpannerMap::findContained(int start, int stop, bool excludeCollisions) const
{
    if (dirty) {
        update();
    }

    results.clear();

    if (excludeCollisions) {
        collisionFreeTree.findContained(start, stop, results);
    } else {
        tree.findContained(start, stop, results);
    }

    return results;
}

//---------------------------------------------------------
//   findOverlapping
//---------------------------------------------------------

const SpannerMap::IntervalList& SpannerMap::findOverlapping(int start, int stop, bool excludeCollisions) const
{
    if (dirty) {
        update();
    }

    results.clear();

    if (excludeCollisions) {
        collisionFreeTree.findOverlapping(start, stop, results);
    } else {
        tree.findOverlapping(start, stop, results);
    }

    return results;
}

void SpannerMap::collectIntervals(IntervalList& regularIntervals, IntervalList& collisionFreeIntervals) const
{
    using IntervalsByType = std::map<ElementType, IntervalList>;
    using IntervalsByPart = std::map<ID, IntervalsByType>;

    IntervalsByPart intervalsByPart;

    //!Note Because of the current UX of spanners adjustments spanners collision is a regular thing,
    //!     so we have to manage those cases when two similar spanners (e.g. Pedal line) are overlapping
    //!     with each other.
    constexpr int collidingSpannersPadding = 1;

    for (const auto& pair : *this) {
        int newSpannerStartTick = pair.second->tick().ticks();
        int newSpannerEndTick = pair.second->tick2().ticks();

        IntervalsByType& intervalsByType = intervalsByPart[pair.second->part()->id()];
        IntervalList& intervalList = intervalsByType[pair.second->type()];

        if (!intervalList.empty()) {
            auto lastIntervalIt = intervalList.rbegin();
            if (lastIntervalIt->stop >= newSpannerStartTick) {
                lastIntervalIt->stop = newSpannerStartTick - collidingSpannersPadding;
            }
        }

        intervalList.emplace_back(interval_tree::Interval<Spanner*>(newSpannerStartTick,
                                                                    newSpannerEndTick,
                                                                    pair.second));

        regularIntervals.push_back(interval_tree::Interval(newSpannerStartTick,
                                                           newSpannerEndTick,
                                                           pair.second));
    }

    for (const auto& pair : intervalsByPart) {
        for (const auto& intervals : pair.second) {
            collisionFreeIntervals.insert(collisionFreeIntervals.end(),
                                          intervals.second.begin(),
                                          intervals.second.end());
        }
    }
}

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void SpannerMap::addSpanner(Spanner* s)
{
    insert(std::pair<int, Spanner*>(s->tick().ticks(), s));
    dirty = true;
}

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

bool SpannerMap::removeSpanner(Spanner* s)
{
    for (auto i = begin(); i != end(); ++i) {
        if (i->second == s) {
            erase(i);
            dirty = true;
            return true;
        }
    }
    LOGD("%s (%p) not found", s->typeName(), s);
    return false;
}

#ifndef NDEBUG
//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void SpannerMap::dump() const
{
    LOGD("SpannerMap::dump");
    for (auto i = begin(); i != end(); ++i) {
        LOGD("   %5d: %s %p", i->first, i->second->typeName(), i->second);
    }
}

#endif
} // namespace mu::engraving
