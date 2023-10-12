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
#ifndef MU_ENGRAVING_DISTANCES_DEV_H
#define MU_ENGRAVING_DISTANCES_DEV_H

#include "infrastructure/shape.h"

namespace mu::engraving {
class Segment;
}

namespace mu::engraving::rendering::dev::distances {
double minHorizontalDistance(const Shape& f, const Shape& s, double spatium, double squeezeFactor = 1.0);
double minVerticalDistance(const Shape& f, const Shape& a);
double verticalClearance(const Shape& f, const Shape& a);
double shapeSpatium(const Shape& s);

double minHorizontalDistance(const Segment* f, const Segment* ns, bool systemHeaderGap, double squeezeFactor);
double minHorizontalCollidingDistance(const Segment* f, const Segment* ns, double squeezeFactor);
}

#endif // MU_ENGRAVING_DISTANCES_DEV_H
