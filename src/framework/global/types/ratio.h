/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
inline float db_to_linear(float v)
{
    return std::pow(10.0, v / 20.0);
}

inline float linear_to_db(float v)
{
    return 20.0 * std::log10(std::abs(v));
}

//! NOTE Just linear ratio
using ratio_t = number_t<float>;

//! NOTE logarithmic ratio (decibel)
using db_t = number_t<float>;

inline ratio_t db_to_linear(db_t v)
{
    return muse::db_to_linear(v.raw());
}

inline db_t linear_to_db(ratio_t v)
{
    return muse::linear_to_db(v.raw());
}

//! NOTE Percent
using percent_t = number_t<float>;
}
