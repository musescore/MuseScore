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

#ifndef MU_ENGRAVING_STAFFTYPELIST_H
#define MU_ENGRAVING_STAFFTYPELIST_H

#include "stafftype.h"

namespace mu::engraving {
//---------------------------------------------------------
//   StaffTypeList
//    this list is instantiated for every staff
//    to keep track of staff type changes
//---------------------------------------------------------

class StaffTypeList
{
public:
    StaffType& staffType(const Fraction&);
    const StaffType& staffType(const Fraction& f) const;
    StaffType* setStaffType(const Fraction&, const StaffType&);
    bool removeStaffType(const Fraction&);
    bool isStaffTypeStartFrom(const Fraction&) const;
    void moveStaffType(const Fraction& from, const Fraction& to);

    bool uniqueStaffType() const { return m_staffTypeChanges.empty(); }
    std::pair<int, int> staffTypeRange(const Fraction&) const;

    const std::map<int, StaffType>& staffTypeChanges() const { return m_staffTypeChanges; }

private:
    StaffType m_firstStaffType;   // staff type at tick 0
    std::map<int, StaffType> m_staffTypeChanges;
};
}

#endif
