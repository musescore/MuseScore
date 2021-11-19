/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include <utility>

#include <QString>
#include "infrastructure/draw/color.h"
#include "infrastructure/draw/geometry.h"
#include "infrastructure/draw/painterpath.h"
#include "spatium.h"

#ifndef MU_ENGRAVING_TYPES_H
#define MU_ENGRAVING_TYPES_H

namespace mu::engraving {
// ========================================
// PropertyValue
// ========================================

// --- Base ---
// bool (std)                           // P_TYPE::BOOL
// int  (std)                           // P_TYPE::INT
// qreal (Qt)                           // P_TYPE::REAL
// QString (Qt)                         // P_TYPE::STRING

// --- Geometry ---
using PointF = mu::PointF;              // P_TYPE::POINT
using SizeF = mu::SizeF;                // P_TYPE::SIZE
using PainterPath = mu::PainterPath;    // P_TYPE::PATH
using ScaleF = mu::ScaleF;              // P_TYPE::SCALE
// Spatium (spatium.h)                  // P_TYPE::SPATIUM
using PairF = mu::PairF;                // P_TYPE::PAIR_REAL

using Color = draw::Color;
}

#endif // MU_ENGRAVING_TYPES_H
