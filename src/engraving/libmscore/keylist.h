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

#ifndef __KEYLIST_H__
#define __KEYLIST_H__

#include <map>

#include "global/allocator.h"
#include "key.h"

namespace mu::engraving {
//---------------------------------------------------------
//   KeyList
//    this list is instantiated for every staff
//    to keep track of key signature changes
//---------------------------------------------------------

class KeyList : public std::map<const int, KeySigEvent>
{
    OBJECT_ALLOCATOR(engraving, KeyList)
public:
    KeyList() {}
    KeySigEvent key(int tick) const;
    KeySigEvent prevKey(int tick) const;
    void setKey(int tick, KeySigEvent);
    int nextKeyTick(int tick) const;
    int currentKeyTick(int tick) const;
};
}

#endif
