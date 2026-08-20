/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "notevelocitygeometry.h"

#include <algorithm>
#include <cmath>

#include "engraving/dom/note.h"
#include "engraving/dom/stafftype.h"

using namespace mu::notation;
using namespace mu::engraving;

// A standard 5-line staff spans 8 half-line units (4 gaps x 2 half-line units per gap), using the
// same half-line-step convention as Note::updateRelLine()/Note::line(). Anchoring the "virtual 5th
// line" this many half-line units above the staff's real bottom line is what lets a 1-line
// percussion staff (or any staff with fewer than 5 lines) get the same velocity range as a normal
// 5-line staff, without needing to special-case the line count anywhere else.
constexpr static int STANDARD_STAFF_HALF_LINE_SPAN = 8;

NoteVelocityYRange mu::notation::noteVelocityYRange(const Note* note)
{
    IF_ASSERT_FAILED(note && note->staffType()) {
        return NoteVelocityYRange();
    }

    const StaffType* st = note->staffType();
    const double halfLineStepPx = note->spatium() * 0.5 * st->lineDistance().val();
    const double noteCanvasY = note->canvasPos().y();
    const int noteLine = note->line();

    const int bottomLine = st->bottomLine();
    const int virtualTopLine = bottomLine - STANDARD_STAFF_HALF_LINE_SPAN;

    NoteVelocityYRange range;
    range.y0 = noteCanvasY + (bottomLine - noteLine) * halfLineStepPx;
    range.y127 = noteCanvasY + (virtualTopLine - noteLine) * halfLineStepPx;
    return range;
}

double mu::notation::canvasYFromVelocity(const NoteVelocityYRange& range, int velocity)
{
    const double v = std::clamp(velocity, 0, 127) / 127.0;
    return range.y0 + (range.y127 - range.y0) * v;
}

int mu::notation::velocityFromCanvasY(const NoteVelocityYRange& range, double canvasY)
{
    const double span = range.y127 - range.y0;
    if (std::abs(span) < 1e-9) {
        return 0;
    }

    const double v = (canvasY - range.y0) / span;
    return std::clamp(static_cast<int>(std::lround(v * 127.0)), 0, 127);
}
