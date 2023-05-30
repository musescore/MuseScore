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

#ifndef MU_ENGRAVING_INOUTDATA_H
#define MU_ENGRAVING_INOUTDATA_H

#include <map>
#include <vector>

#include "linksindexer.h"

#include "writer/writecontext.h"

namespace mu::engraving {
class LinkedObjects;
}

namespace mu::engraving::rw {
struct ReadLinks
{
    std::map<int /*staffIndex*/, std::vector<std::pair<LinkedObjects*, Location> > > staffLinkedElements; // one list per staff
    LinksIndexer linksIndexer;
};

struct ReadInOutData {
    // for master - out
    // for except - in
    ReadLinks links;

    // out
    SettingsCompat settingsCompat;
};

struct WriteInOutData {
    rw400::WriteContext ctx;
};
}

#endif // MU_ENGRAVING_INOUTDATA_H
