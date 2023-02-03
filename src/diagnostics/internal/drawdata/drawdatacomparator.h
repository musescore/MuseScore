/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_DIAGNOSTICS_DRAWDATACOMPARATOR_H
#define MU_DIAGNOSTICS_DRAWDATACOMPARATOR_H

#include "global/types/ret.h"
#include "global/io/path.h"
#include "draw/types/drawdata.h"

namespace mu::diagnostics {
class DrawDataComparator
{
public:
    DrawDataComparator() = default;

    draw::Diff compare(const draw::DrawDataPtr& ref, const draw::DrawDataPtr& test);
    Ret compare(const io::path_t& ref, const io::path_t& test, const io::path_t& outdiff);
};
}

#endif // MU_DIAGNOSTICS_DRAWDATACOMPARATOR_H
