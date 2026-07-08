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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "engraving/dom/score.h"
#include "engraving/types/fraction.h"

namespace mu::notation {
using engraving::LoopBoundaryType;

struct LoopBoundaries
{
    engraving::Fraction loopInTick;
    engraving::Fraction loopOutTick;
    bool enabled = false;

    bool isNull() const
    {
        return loopInTick.isZero() && loopOutTick.isZero();
    }

    bool operator==(const LoopBoundaries& boundaries) const
    {
        bool equals = true;

        equals &= loopInTick == boundaries.loopInTick;
        equals &= loopOutTick == boundaries.loopOutTick;
        equals &= enabled == boundaries.enabled;

        return equals;
    }

    bool operator!=(const LoopBoundaries& boundaries) const
    {
        return !(*this == boundaries);
    }
};
}
