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

#include "keylist.h"
#include "xml.h"
#include "score.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   key
//
//    locates the key sig currently in effect at tick
//---------------------------------------------------------

KeySigEvent KeyList::key(int tick) const
{
    KeySigEvent ke;
    ke.setKey(Key::C);

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
    kc.setKey(Key::C);

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

//---------------------------------------------------------
//   KeyList::read
//---------------------------------------------------------

void KeyList::read(XmlReader& e, Score* cs)
{
    while (e.readNextStartElement()) {
        if (e.name() == "key") {
            Key k;
            int tick = e.intAttribute("tick", 0);
            if (e.hasAttribute("custom")) {
                k = Key::C;              // ke.setCustomType(e.intAttribute("custom"));
            } else {
                k = Key(e.intAttribute("idx"));
            }
            KeySigEvent ke;
            ke.setKey(k);
            (*this)[cs->fileDivision(tick)] = ke;
            e.readNext();
        } else {
            e.unknown();
        }
    }
}
}
