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
#ifndef MU_INSPECTOR_DYNAMICTYPES_H
#define MU_INSPECTOR_DYNAMICTYPES_H

#include "qobjectdefs.h"

class DynamicTypes
{
    Q_GADGET

public:
    enum class Scope {
        SCOPE_STAFF = 0,
        SCOPE_SINGLE_INSTRUMENT,
        SCOPE_ALL_INSTRUMENTS
    };

    enum class VelocityChangeSpeed {
        VELOCITY_CHANGE_SPEED_SLOW = 0,
        VELOCITY_CHANGE_SPEED_NORMAL,
        VELOCITY_CHANGE_SPEED_FAST
    };

    Q_ENUM(Scope)
    Q_ENUM(VelocityChangeSpeed)
};

#endif // MU_INSPECTOR_DYNAMICTYPES_H
