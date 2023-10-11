/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_SHAPEUTILS_DEV_H
#define MU_ENGRAVING_SHAPEUTILS_DEV_H

#include "infrastructure/shape.h"

namespace mu::engraving::rendering::dev {
double minHorizontalDistance(const Shape& f, const Shape& s, double spatium, double squeezeFactor = 1.0);
}

#endif // MU_ENGRAVING_SHAPEUTILS_DEV_H
