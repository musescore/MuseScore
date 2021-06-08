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

#include "cleflist.h"
#include "clef.h"
#include "score.h"
#include "xml.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   ClefTypeList::operator==
//---------------------------------------------------------

bool ClefTypeList::operator==(const ClefTypeList& t) const
{
    return t._concertClef == _concertClef && t._transposingClef == _transposingClef;
}

//---------------------------------------------------------
//   ClefTypeList::operator!=
//---------------------------------------------------------

bool ClefTypeList::operator!=(const ClefTypeList& t) const
{
    return t._concertClef != _concertClef || t._transposingClef != _transposingClef;
}

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

ClefTypeList ClefList::clef(int tick) const
{
    if (empty()) {
        return ClefTypeList(ClefType::INVALID, ClefType::INVALID);
    }
    auto i = upper_bound(tick);
    if (i == begin()) {
        return ClefTypeList(ClefType::INVALID, ClefType::INVALID);
    }
    return (--i)->second;
}

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void ClefList::setClef(int tick, ClefTypeList ctl)
{
    auto i = find(tick);
    if (i == end()) {
        insert(std::pair<int, ClefTypeList>(tick, ctl));
    } else {
        i->second = ctl;
    }
}

//---------------------------------------------------------
//   nextClefTick
//
//    return the tick at which the clef after tick is located
//    return -1, if no such clef
//---------------------------------------------------------

int ClefList::nextClefTick(int tick) const
{
    if (empty()) {
        return -1;
    }
    auto i = upper_bound(tick + 1);
    if (i == end()) {
        return -1;
    }
    return i->first;
}

//---------------------------------------------------------
//   currentClefTick
//
//    return the tick position of the clef currently
//    in effect at tick
//---------------------------------------------------------

int ClefList::currentClefTick(int tick) const
{
    if (empty()) {
        return 0;
    }
    auto i = upper_bound(tick);
    if (i == begin()) {
        return 0;
    }
    --i;
    return i->first;
}
}
