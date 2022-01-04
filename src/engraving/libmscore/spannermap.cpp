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

using namespace mu;

namespace Ms {
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

const std::vector<interval_tree::Interval<Spanner*> >& SpannerMap::findContained(int start, int stop)
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

const std::vector<interval_tree::Interval<Spanner*> >& SpannerMap::findOverlapping(int start, int stop)
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
    qDebug("%s (%p) not found", s->name(), s);
    return false;
}

#ifndef NDEBUG
//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void SpannerMap::dump() const
{
    qDebug("SpannerMap::dump");
    for (auto i = begin(); i != end(); ++i) {
        qDebug("   %5d: %s %p", i->first, i->second->name(), i->second);
    }
}

#endif
}     // namespace Ms
