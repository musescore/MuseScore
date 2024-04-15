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
#ifndef MU_INSPECTOR_MARKERTYPES_H
#define MU_INSPECTOR_MARKERTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class MarkerTypes
{
    Q_GADGET

public:
    enum class Type {
        TYPE_SEGNO = 0,
        TYPE_VARSEGNO,
        TYPE_CODA,
        TYPE_VARCODA,
        TYPE_CODETTA,
        TYPE_FINE,
        TYPE_TOCODA,
        TYPE_USER
    };

    Q_ENUM(Type)
};
}

#endif // MU_INSPECTOR_MARKERTYPES_H
