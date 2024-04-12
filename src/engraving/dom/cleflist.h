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

#ifndef MU_ENGRAVING_CLEFLIST_H
#define MU_ENGRAVING_CLEFLIST_H

#include "clef.h"

namespace mu::engraving {
//---------------------------------------------------------
//   ClefList
//---------------------------------------------------------

class ClefList : public std::map<int, ClefTypeList>
{
    OBJECT_ALLOCATOR(engraving, ClefList)
public:
    ClefList() {}
    ClefTypeList clef(int tick) const;
    void setClef(int tick, ClefTypeList);
    int nextClefTick(int tick) const;
    int currentClefTick(int tick) const;
};
} // namespace mu::engraving
#endif
