/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

    enum CenterBetweenStaves {
        CENTER_STAVES_AUTO,
        CENTER_STAVES_ON,
        CENTER_STAVES_OFF
    };

    Q_ENUM(VerticalDirection)
    Q_ENUM(HorizontalDirection)
    Q_ENUM(CenterBetweenStaves)
};
}

#endif // MU_INSPECTOR_DIRECTIONTYPES_H
