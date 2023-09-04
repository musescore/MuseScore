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

#ifndef __STAFFTYPELIST_H__
#define __STAFFTYPELIST_H__

#include "stafftype.h"

namespace mu::engraving {
//---------------------------------------------------------
//   StaffTypeList
//    this list is instantiated for every staff
//    to keep track of staff type changes
//---------------------------------------------------------

class StaffTypeList
{
    StaffType firstStaffType;   ///< staff type at tick 0
    std::map<int, StaffType> staffTypeChanges;

public:
    StaffTypeList() {}
    StaffType& staffType(const Fraction&);
    const StaffType& staffType(const Fraction& f) const;
    StaffType* setStaffType(const Fraction&, const StaffType&);
    bool removeStaffType(const Fraction&);
    bool isStaffTypeStartFrom(const Fraction&) const;
    void moveStaffType(const Fraction& from, const Fraction& to);

    bool uniqueStaffType() const { return staffTypeChanges.empty(); }
    std::pair<int, int> staffTypeRange(const Fraction&) const;
};
}

#endif
