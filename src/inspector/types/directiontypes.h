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
#ifndef MU_INSPECTOR_DIRECTIONTYPES_H
#define MU_INSPECTOR_DIRECTIONTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class DirectionTypes
{
    Q_GADGET

public:
    enum VerticalDirection {
        VERTICAL_AUTO,
        VERTICAL_UP,
        VERTICAL_DOWN
    };

    enum HorizontalDirection {
        HORIZONTAL_AUTO,
        HORIZONTAL_LEFT,
        HORIZONTAL_RIGHT
    };

    enum TiePlacement {
        TIE_PLACEMENT_AUTO,
        TIE_PLACEMENT_INSIDE,
        TIE_PLACEMENT_OUTSIDE,
    };

    Q_ENUM(VerticalDirection)
    Q_ENUM(HorizontalDirection)
    Q_ENUM(TiePlacement)
};
}

#endif // MU_INSPECTOR_DIRECTIONTYPES_H
