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
#ifndef MU_INSPECTOR_ARTICULATIONTYPES_H
#define MU_INSPECTOR_ARTICULATIONTYPES_H

#include "qobjectdefs.h"

class ArticulationTypes
{
    Q_GADGET

public:
    enum class Placement {
        TYPE_TOP,
        TYPE_BOTTOM,
        TYPE_AUTO
    };

    enum class Style {
        STYLE_STANDART = 0,
        STYLE_BAROQUE
    };

    enum class ArticulationStemSideAlign {
        STEM,
        NOTEHEAD,
        AVERAGE
    };

    Q_ENUM(Placement)
    Q_ENUM(Style)
    Q_ENUM(ArticulationStemSideAlign)
};

#endif // MU_INSPECTOR_ARTICULATIONTYPES_H
