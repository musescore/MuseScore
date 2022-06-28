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

const SpannerMap::SpannerRange& SpannerMap::range(int tickFrom, int tickTo) const
{
    static SpannerRange result;

    if (empty()) {
        result.first = cend();
        result.last = cend();
        return result;
    }

    auto firstNotLess = std::lower_bound(cbegin(), cend(), tickFrom, [](const auto& pair, int tick) {
        int spannerFrom = pair.second->tick().ticks();
        int spannerTo = pair.second->tick().ticks() + pair.second->ticks().ticks();
        return spannerFrom < tick && spannerTo < tick;
    });

    auto firstGreater = upper_bound(tickTo);

    if (firstNotLess == firstGreater
        || firstNotLess->first >= tickTo) {
        result.first = cend();
        result.last = cend();
        return result;
    }

    result.first = firstNotLess;
    result.last = firstGreater;
    return result;
}

//---------------------------------------------------------
//   update
//   updates the internal lookup tree, not the map itself
//---------------------------------------------------------

void SpannerMap::update() const
{
    std::vector<interval_tree::Interval<Spanner*> > intervals;
    for (auto i : *this) {
        intervals.push_back(interval_tree::Interval<Spanner*>(i.second->tick().ticks(), i.second->tick2().ticks(), i.second));
    }
    tree = interval_tree::IntervalTree<Spanner*>(intervals);
    dirty = false;
}

//---------------------------------------------------------
//   findContained
//---------------------------------------------------------

const std::vector<interval_tree::Interval<Spanner*> >& SpannerMap::findContained(int start, int stop) const
{
    if (dirty) {
        update();
    }
    results.clear();
    tree.findContained(start, stop, results);
    return results;
}

//---------------------------------------------------------
//   findOverlapping
//---------------------------------------------------------

const std::vector<interval_tree::Interval<Spanner*> >& SpannerMap::findOverlapping(int start, int stop) const
{
    if (dirty) {
        update();
    }
    results.clear();
    tree.findOverlapping(start, stop, results);
    return results;
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
