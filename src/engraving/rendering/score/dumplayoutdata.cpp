/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "dumplayoutdata.h"

#include <sstream>

#include "dom/score.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

static void dumpLayoutData(const EngravingObject* obj, std::stringstream& ss)
{
    for (EngravingObject* ch : obj->children()) {
        if (ch->isEngravingItem()) {
            toEngravingItem(ch)->ldata()->dump(ss);
        }

        dumpLayoutData(ch, ss);
    }
}

std::string DumpLayoutData::dump(const Score* s)
{
    std::stringstream ss;
    // Start from the score, which owns the pages, the systems and the measures; the
    // root item only leads to the dummy.
    dumpLayoutData(s, ss);
    return ss.str();
}
