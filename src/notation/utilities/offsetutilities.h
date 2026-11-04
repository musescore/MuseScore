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

namespace mu::notation {
class OffsetUtils
{
public:
    /**
     * Converts a vertical offset between the internal convention (negative = up, positive = down)
     * and the displayed one (positive = up, negative = down).
     */
    static double flipSign(double v)
    {
        // Uses 0.0 - v rather than -v so that zero doesn't become -0
        return 0.0 - v;
    }

    static muse::PointF flipYSign(const muse::PointF& p)
    {
        return muse::PointF(p.x(), flipSign(p.y()));
    }
};
}
