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

#include "segmentlist.h"
#include "segment.h"
#include "score.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   clone
//---------------------------------------------------------

SegmentList SegmentList::clone() const
{
    SegmentList dl;
    Segment* s = m_first;
    for (int i = 0; i < m_size; ++i) {
        Segment* ns = s->clone();
        dl.push_back(ns);
        s = s->next();
    }
    dl.check();
    return dl;
}

Segment* SegmentList::at(int index) const
{
    Segment* s = m_first;
    for (int i = 0; i < m_size; ++i) {
        if (i == index) {
            return s;
        }
        s = s->next();
    }
    return nullptr;
}

Segment* SegmentList::firstActive() const
{
    if (Segment* segment = m_first) {
        return segment->isActive() ? segment : segment->nextActive();
    }
    return nullptr;
}

//---------------------------------------------------------
//   check
//---------------------------------------------------------

#ifndef NDEBUG
void SegmentList::check()
{
    int n = 0;
    Segment* f = 0;
    Segment* l = 0;
    for (Segment* s = m_first; s; s = s->next()) {
        if (f == 0) {
            f = s;
        }
        l = s;
        ++n;
    }
    for (Segment* s = m_first; s; s = s->next()) {
        switch (s->segmentType()) {
        case SegmentType::Invalid:
        case SegmentType::BeginBarLine:
        case SegmentType::HeaderClef:
        case SegmentType::Clef:
        case SegmentType::KeySig:
        case SegmentType::Ambitus:
        case SegmentType::TimeSig:
        case SegmentType::StartRepeatBarLine:
        case SegmentType::BarLine:
        case SegmentType::ChordRest:
        case SegmentType::Breath:
        case SegmentType::ClefRepeatAnnounce:
        case SegmentType::TimeSigRepeatAnnounce:
        case SegmentType::KeySigRepeatAnnounce:
        case SegmentType::ClefStartRepeatAnnounce:
        case SegmentType::TimeSigStartRepeatAnnounce:
        case SegmentType::KeySigStartRepeatAnnounce:
        case SegmentType::EndBarLine:
        case SegmentType::TimeSigAnnounce:
        case SegmentType::KeySigAnnounce:
        case SegmentType::TimeTick:
            break;
        default:
            ASSERT_X(String(u"SegmentList::check: invalid segment type: %1").arg(int(s->segmentType())));
            break;
        }
        Segment* ss = s->next();
        while (ss) {
            if (s == ss) {
                ASSERT_X("SegmentList::check: segment twice in list");
            }
            ss = ss->next();
        }
    }
    if (f != m_first) {
        ASSERT_X("SegmentList::check: bad first");
    }
    if (l != m_last) {
        ASSERT_X("SegmentList::check: bad last");
    }
    if (f && f->prev()) {
        ASSERT_X("SegmentList::check: first has prev");
    }
    if (l && l->next()) {
        ASSERT_X("SegmentList::check: last has next");
    }
    if (n != m_size) {
        ASSERT_X(String(u"SegmentList::check: counted %1 but _size is %d2").arg(n, m_size));
        m_size = n;
    }
}

#endif

//---------------------------------------------------------
//   insert
///   Insert Segment \a e before Segment \a el.
//---------------------------------------------------------

void SegmentList::insert(Segment* e, Segment* el)
{
    if (el == 0) {
        push_back(e);
    } else if (el == first()) {
        push_front(e);
    } else {
        ++m_size;
        e->setNext(el);
        e->setPrev(el->prev());
        el->prev()->setNext(e);
        el->setPrev(e);
    }
    check();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SegmentList::remove(Segment* e)
{
#ifndef NDEBUG
    check();
    bool found = false;
    for (Segment* s = m_first; s; s = s->next()) {
        if (e == s) {
            found = true;
            break;
        }
    }
    if (!found) {
        ASSERT_X(String(u"segment %1 not in list").arg(String::fromAscii(e->subTypeName())));
    }
#endif
    --m_size;
    if (e == m_first) {
        m_first = m_first->next();
        if (m_first) {
            m_first->setPrev(0);
        }
        if (e == m_last) {
            m_last = 0;
        }
    } else if (e == m_last) {
        m_last = m_last->prev();
        if (m_last) {
            m_last->setNext(0);
        }
    } else {
        e->prev()->setNext(e->next());
        e->next()->setPrev(e->prev());
    }
}

//---------------------------------------------------------
//   push_back
//---------------------------------------------------------

void SegmentList::push_back(Segment* e)
{
    ++m_size;
    e->setNext(0);
    if (m_last) {
        m_last->setNext(e);
    } else {
        m_first = e;
    }
    e->setPrev(m_last);
    m_last = e;
    check();
}

//---------------------------------------------------------
//   push_front
//---------------------------------------------------------

void SegmentList::push_front(Segment* e)
{
    ++m_size;
    e->setPrev(0);
    if (m_first) {
        m_first->setPrev(e);
    } else {
        m_last = e;
    }
    e->setNext(m_first);
    m_first = e;
    check();
}

//---------------------------------------------------------
//   firstCRSegment
//---------------------------------------------------------

Segment* SegmentList::firstCRSegment() const
{
    return first(SegmentType::ChordRest);
}

//---------------------------------------------------------
//   first
//---------------------------------------------------------

Segment* SegmentList::first(SegmentType types) const
{
    for (Segment* s = m_first; s; s = s->next()) {
        if (s->segmentType() & types) {
            return s;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   first
//---------------------------------------------------------

Segment* SegmentList::first(ElementFlag flags) const
{
    for (Segment* s = m_first; s; s = s->next()) {
        if (s->flag(flags)) {
            return s;
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   last
//---------------------------------------------------------

Segment* SegmentList::last(ElementFlag flags) const
{
    for (Segment* s = m_last; s; s = s->prev()) {
        if (s->flag(flags)) {
            return s;
        }
    }
    return nullptr;
}

Segment* SegmentList::last(SegmentType types) const
{
    for (Segment* s = m_last; s; s = s->prev()) {
        if (s->segmentType() & types) {
            return s;
        }
    }
    return nullptr;
}
}
