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

#ifndef __TUPLETMAP_H__
#define __TUPLETMAP_H__

#include <list>

namespace mu::engraving {
class Tuplet;

//---------------------------------------------------------
//   Tuplet2
//---------------------------------------------------------

struct Tuplet2 {
    Tuplet* o = nullptr;
    Tuplet* n = nullptr;
    Tuplet2(Tuplet* _o, Tuplet* _n)
        : o(_o), n(_n) {}
};

//---------------------------------------------------------
//   TupletMap
//---------------------------------------------------------

class TupletMap
{
    std::list<Tuplet2> map;

public:
    TupletMap() {}
    Tuplet* findNew(Tuplet* o);
    void add(Tuplet* _o, Tuplet* _n) { map.push_back(Tuplet2(_o, _n)); }
};
} // namespace mu::engraving
#endif
