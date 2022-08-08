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

#ifndef __SPANNERMAP_H__
#define __SPANNERMAP_H__

#include <map>
#include "thirdparty/intervaltree/IntervalTree.h"

namespace mu::engraving {
class Spanner;

//---------------------------------------------------------
//   SpannerMap
//---------------------------------------------------------

class SpannerMap : std::multimap<int, Spanner*>
{
    mutable bool dirty;
    mutable interval_tree::IntervalTree<Spanner*> tree;
    mutable std::vector<interval_tree::Interval<Spanner*> > results;

public:
    typedef typename std::multimap<int, Spanner*>::const_reverse_iterator const_reverse_it;
    typedef typename std::multimap<int, Spanner*>::const_iterator const_it;

    SpannerMap();

    const std::vector<interval_tree::Interval<Spanner*> >& findContained(int start, int stop) const;
    const std::vector<interval_tree::Interval<Spanner*> >& findOverlapping(int start, int stop) const;
    const std::multimap<int, Spanner*>& map() const { return *this; }
    const_reverse_it crbegin() const { return std::multimap<int, Spanner*>::crbegin(); }
    const_reverse_it crend() const { return std::multimap<int, Spanner*>::crend(); }
    const_it cbegin() const { return std::multimap<int, Spanner*>::cbegin(); }
    const_it cend() const { return std::multimap<int, Spanner*>::cend(); }
    void addSpanner(Spanner* s);
    bool removeSpanner(Spanner* s);
    void clear() { std::multimap<int, Spanner*>::clear(); dirty = true; }
    bool empty() const { return std::multimap<int, Spanner*>::empty(); }
    void update() const;
    void setDirty() const { dirty = true; }     // must be called if a spanner changes start/length
#ifndef NDEBUG
    void dump() const;
#endif
};
} // namespace mu::engraving

#endif
