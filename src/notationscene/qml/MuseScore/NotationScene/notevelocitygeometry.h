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

#pragma once

namespace mu::engraving {
class Note;
}

// Maps a note's velocity (0-127) to a canvas Y position between its staff's bottom line
// (velocity 0) and where a 5th staff line would sit if the staff had one (velocity 127), even on
// staves that don't actually have 5 lines (e.g. 1-line percussion staves).

namespace mu::notation {
struct NoteVelocityYRange {
    double y0 = 0.0;   // canvas Y of the staff's actual bottom line (velocity 0)
    double y127 = 0.0; // canvas Y of the (possibly virtual) 5th line from the bottom (velocity 127)
};

NoteVelocityYRange noteVelocityYRange(const mu::engraving::Note* note);

double canvasYFromVelocity(const NoteVelocityYRange& range, int velocity);
int velocityFromCanvasY(const NoteVelocityYRange& range, double canvasY);
}
