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

#include "global/types/number.h"

#include "engraving/types/bps.h"
#include "engraving/types/constants.h"

namespace mu::engraving {
//! NOTE: AutomationPoint values are normalized [0, 1] - Tempo has no such native unit,
//! so it's normalized as a fraction of the score's own max valid tempo
constexpr muse::real_t normalizeTempo(const BeatsPerSecond& bps)
{
    return muse::real_t::make(bps.val / Constants::MAX_TEMPO.val);
}

constexpr BeatsPerSecond denormalizeTempo(const muse::real_t normalized)
{
    return BeatsPerSecond(normalized.raw() * Constants::MAX_TEMPO.val);
}

inline constexpr muse::real_t DEFAULT_NORMALIZED_TEMPO = normalizeTempo(Constants::DEFAULT_TEMPO);
inline constexpr muse::real_t MIN_NORMALIZED_TEMPO = normalizeTempo(Constants::MIN_TEMPO);
inline constexpr muse::real_t MAX_NORMALIZED_TEMPO = normalizeTempo(Constants::MAX_TEMPO);
}
