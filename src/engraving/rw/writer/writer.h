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
#ifndef MU_ENGRAVING_WRITE400_H
#define MU_ENGRAVING_WRITE400_H

#include "../iwriter.h"

#include "writecontext.h"
#include "../compat/writescorehook.h"

namespace mu::engraving::write {
class Writer : public rw::IWriter
{
public:

    bool writeScore(Score* score, io::IODevice* device, bool onlySelection, rw::WriteInOutData* out) override;

    static void write(Score* score, XmlWriter& xml, WriteContext& ctx, bool selectionOnly, compat::WriteScoreHook& hook);
};
}

#endif // MU_ENGRAVING_WRITE400_H
