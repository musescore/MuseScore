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

#include "stafftypelist.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

const StaffType& StaffTypeList::staffType(const Fraction& tick) const
{
    if (tick.negative()) {
        static const StaffType st;
        return st;
    }

    if (staffTypeChanges.empty()) {
        return firstStaffType;
    }

    auto i = staffTypeChanges.upper_bound(tick.ticks());
    if (i == staffTypeChanges.begin()) {
        return firstStaffType;
    }

    return (--i)->second;
}

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

StaffType& StaffTypeList::staffType(const Fraction& tick)
{
    assert(!tick.negative());

    if (staffTypeChanges.empty()) {
        return firstStaffType;
    }

    auto i = staffTypeChanges.upper_bound(tick.ticks());
    if (i == staffTypeChanges.begin()) {
        return firstStaffType;
    }
    return (--i)->second;
}

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

StaffType* StaffTypeList::setStaffType(const Fraction& tick, const StaffType& st)
{
    assert(!tick.negative());

    if (tick.isZero()) {
        firstStaffType = st;
        return &firstStaffType;
    }

    auto i = staffTypeChanges.find(tick.ticks());
    StaffType* nst;
    if (i == staffTypeChanges.end()) {
        auto k = staffTypeChanges.insert(std::pair<int, StaffType>(tick.ticks(), st));
        nst = &(k.first->second);
    } else {
        i->second = st;
        nst = &i->second;
    }
    return nst;
}

//---------------------------------------------------------
//   removeStaffType
//---------------------------------------------------------

bool StaffTypeList::removeStaffType(const Fraction& tick)
{
    assert(!tick.negative());
    if (tick.isZero()) {
        firstStaffType = StaffType();
        return true;
    }

    auto i = staffTypeChanges.find(tick.ticks());

    if (i == staffTypeChanges.end()) {
        return false;
    }

    staffTypeChanges.erase(i);
    return true;
}

bool StaffTypeList::isStaffTypeStartFrom(const Fraction& tick) const
{
    return staffTypeChanges.find(tick.ticks()) != staffTypeChanges.end();
}

void StaffTypeList::moveStaffType(const Fraction& from, const Fraction& to)
{
    StaffType tmp = staffType(from);
    removeStaffType(from);
    setStaffType(to, tmp);
}

//---------------------------------------------------------
//   staffTypeRange
//---------------------------------------------------------

std::pair<int, int> StaffTypeList::staffTypeRange(const Fraction& tick) const
{
    if (tick.negative()) {
        return { -1, -1 };
    }

    auto i = staffTypeChanges.upper_bound(tick.ticks());
    const int end = (i == staffTypeChanges.end()) ? -1 : i->first;
    const int start = (i == staffTypeChanges.begin()) ? 0 : (--i)->first;

    return { start, end };
}
}
