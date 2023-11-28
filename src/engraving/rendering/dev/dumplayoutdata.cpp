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
#include "dumplayoutdata.h"

#include <sstream>

#include "dom/score.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

static void dumpLayoutData(const EngravingItem* item, std::stringstream& ss)
{
    item->ldata()->dump(ss);

    for (EngravingItem* ch : item->childrenItems()) {
        dumpLayoutData(ch, ss);
    }
}

std::string DumpLayoutData::dump(const Score* s)
{
    std::stringstream ss;
    dumpLayoutData(s->rootItem(), ss);
    return ss.str();
}
