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

#include "draw/types/geometry.h"

#include "notationautomationtypes.h"

namespace mu::notation {
muse::RectF regionCanvasRect(const System* system, staff_idx_t staffIdx, const Measure* startMeasure, const Measure* endMeasure);
int regionLastTick(const Measure* endMeasure);
int tickFromXInPolyline(const PolylineKey& key, const PolylineData& data, qreal x);
}
