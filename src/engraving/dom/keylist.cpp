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

#include "keylist.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   key
//
//    locates the key sig currently in effect at tick
//---------------------------------------------------------

KeySigEvent KeyList::key(int tick) const
{
    KeySigEvent ke;
    ke.setConcertKey(Key::C);

    if (empty()) {
        return ke;
    }
    auto i = upper_bound(tick);
    if (i == begin()) {
        return ke;
    }
    return (--i)->second;
}

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void KeyList::setKey(int tick, KeySigEvent k)
{
    auto i = find(tick);
    if (i == end()) {
        insert(std::pair<int, KeySigEvent>(tick, k));
    } else {
        i->second = k;
    }
}

//---------------------------------------------------------
//   nextKeyTick
//
//    return the tick at which the key sig after tick is located
//    return -1, if no such a key sig
//---------------------------------------------------------

int KeyList::nextKeyTick(int tick) const
{
    if (empty()) {
        return -1;
    }
    auto i = upper_bound(tick + 1);
    return i == end() ? -1 : i->first;
}

//---------------------------------------------------------
//   prevKey
//
//    returns the key before the current key for tick
//---------------------------------------------------------

KeySigEvent KeyList::prevKey(int tick) const
{
    KeySigEvent kc;
    kc.setConcertKey(Key::C);

    if (empty()) {
        return kc;
    }
    auto i = upper_bound(tick);
    if (i == begin()) {
        return kc;
    }
    --i;
    if (i == begin()) {
        return kc;
    }
    return (--i)->second;
}

//---------------------------------------------------------
//   currentKeyTick
//
//    return the tick position of the key currently
//    in effect at tick
//---------------------------------------------------------

int KeyList::currentKeyTick(int tick) const
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
