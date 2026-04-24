/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include <cmath>
#include "number.h"

namespace muse {
using secs_t = number_t<double>;
using msecs_t = number_t<int64_t>;
using usecs_t = number_t<int64_t>;

inline secs_t msecs_to_secs(msecs_t msecs) { return secs_t(msecs.raw() / 1000.0); }
inline msecs_t secs_to_msecs(secs_t secs) { return msecs_t(static_cast<int64_t>(std::llround(secs.raw() * 1000.0))); }

inline secs_t usecs_to_secs(usecs_t usecs) { return secs_t(usecs.raw() / 1000000.0); }
inline usecs_t secs_to_usecs(secs_t secs) { return usecs_t(static_cast<int64_t>(std::llround(secs.raw() * 1000000.0))); }
}
