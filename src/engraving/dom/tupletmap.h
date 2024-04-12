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

#ifndef MU_ENGRAVING_TUPLETMAP_H
#define MU_ENGRAVING_TUPLETMAP_H

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
public:
    TupletMap() {}
    Tuplet* findNew(Tuplet* o);
    void add(Tuplet* _o, Tuplet* _n) { m_map.push_back(Tuplet2(_o, _n)); }

private:
    std::list<Tuplet2> m_map;
};
} // namespace mu::engraving
#endif
