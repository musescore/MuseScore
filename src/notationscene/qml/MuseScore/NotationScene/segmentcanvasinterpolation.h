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

#include <optional>

#include "notation/notationtypes.h"

// Shared canvasX<->tick interpolation used by overlay controllers (automation, note offsets) to
// translate between a mouse/canvas X position and a musical tick, and back. Both directions
// interpolate linearly between the nearest Duration/barline segments on either side of the point,
// so a caller that uses one direction to interpret input and the other to render output gets
// values that round-trip exactly.

namespace mu::notation {
std::optional<int> tickFromCanvasX(const System* system, double canvasX);
std::optional<double> canvasXFromTick(const System* system, int tick);
}
