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
#ifndef MU_INSPECTOR_TREMOLOBARTYPES_H
#define MU_INSPECTOR_TREMOLOBARTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class TremoloBarTypes
{
    Q_GADGET

public:
    //! NOTE: must be in synch with mu::engraving::TremoloBarType
    enum class TremoloBarType {
        TYPE_DIP = 0,
        TYPE_DIVE,
        TYPE_RELEASE_UP,
        TYPE_INVERTED_DIP,
        TYPE_RETURN,
        TYPE_RELEASE_DOWN,
        TYPE_CUSTOM
    };

    Q_ENUM(TremoloBarType)
};
}

#endif // MU_INSPECTOR_TREMOLOBARTYPES_H
