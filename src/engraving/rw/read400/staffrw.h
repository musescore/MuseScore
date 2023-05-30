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
#ifndef MU_ENGRAVING_STAFFRW_H
#define MU_ENGRAVING_STAFFRW_H

#include "readcontext.h"
#include "../write/writecontext.h"

namespace mu::engraving {
class XmlReader;
class XmlWriter;
class MeasureBase;
}

namespace mu::engraving::read400 {
class StaffRW
{
public:

    static void readStaff(Score* score, XmlReader&, ReadContext& ctx);
    static void writeStaff(const Staff* staff, XmlWriter& xml, write::WriteContext& ctx, MeasureBase* measureStart, MeasureBase* measureEnd,
                           staff_idx_t staffStart, staff_idx_t staffIdx, bool selectionOnly);
};
}

#endif // MU_ENGRAVING_STAFFRW_H
