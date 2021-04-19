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
#ifndef MU_INSPECTOR_ARTICULATIONTYPES_H
#define MU_INSPECTOR_ARTICULATIONTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class ArticulationTypes
{
    Q_GADGET

public:
    enum class Placement {
        TYPE_ABOVE_STAFF,
        TYPE_BELOW_STAFF,
        TYPE_CHORD_AUTO,
        TYPE_ABOVE_CHORD,
        TYPE_BELOW_CHORD
    };

    enum class Direction {
        AUTO,
        DOWN,
        UP
    };

    enum class Style {
        STYLE_STANDART = 0,
        STYLE_BAROQUE
    };

    Q_ENUM(Placement)
    Q_ENUM(Direction)
    Q_ENUM(Style)
};
}

#endif // MU_INSPECTOR_ARTICULATIONTYPES_H
