/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#pragma once

#include "segment.h"

namespace mu::engraving {
class Segment;

//---------------------------------------------------------
//   SegmentList
//---------------------------------------------------------

class SegmentList
{
public:
    SegmentList() { clear(); }
    void clear() { m_first = m_last = 0; m_size = 0; }
#ifndef NDEBUG
    void check();
#else
    void check() {}
#endif
    SegmentList clone() const;
    int size() const { return m_size; }

    Segment* first() const { return m_first; }
    Segment* firstActive() const;
    Segment* first(SegmentType) const;
    Segment* first(ElementFlag) const;

    Segment* last() const { return m_last; }
    Segment* last(ElementFlag) const;
    Segment* last(SegmentType) const;
    Segment* firstCRSegment() const;
    void remove(Segment*);
    void push_back(Segment*);
    void push_front(Segment*);
    void insert(Segment* e, Segment* el);    // insert e before el

    class iterator
    {
        Segment* p;
    public:
        iterator(Segment* s) { p = s; }
        iterator operator++() { iterator i(p); p = p->next(); return i; }
        bool operator !=(const iterator& i) const { return p != i.p; }
        Segment& operator*() { return *p; }
    };
    class const_iterator
    {
        const Segment* p;
    public:
        const_iterator(const Segment* s) { p = s; }
        const_iterator operator++() { const_iterator i(p); p = p->next(); return i; }
        bool operator !=(const const_iterator& i) const { return p != i.p; }
        const Segment& operator*() const { return *p; }
    };

    iterator begin() { return m_first; }
    iterator end() { return 0; }
    const_iterator begin() const { return m_first; }
    const_iterator end() const { return 0; }

private:

    Segment* m_first = nullptr;          // First item of segment list
    Segment* m_last = nullptr;           // Last item of segment list
    int m_size = 0;                      // Number of items in segment list
};

// Segment* begin(SegmentList& l) { return l.first(); }
// Segment* end(SegmentList&) { return 0; }
} // namespace mu::engraving
