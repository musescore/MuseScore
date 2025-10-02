/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "inserttime.h"

#include <set>

#include "../dom/score.h"
#include "../dom/spanner.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   InsertTime
//---------------------------------------------------------

void InsertTime::redo(EditData*)
{
    score->insertTime(tick, len);
}

void InsertTime::undo(EditData*)
{
    score->insertTime(tick, -len);
}

//---------------------------------------------------------
//   InsertTimeUnmanagedSpanner
//---------------------------------------------------------

void InsertTimeUnmanagedSpanner::flip(EditData*)
{
    for (Score* s : score->scoreList()) {
        std::set<Spanner*> spannersCopy = s->unmanagedSpanners();
        for (Spanner* sp : spannersCopy) {
            sp->insertTimeUnmanaged(tick, len);
        }
    }
    len = -len;
}
