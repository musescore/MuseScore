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
#ifndef MU_INSPECTOR_AMBITUSTYPES_H
#define MU_INSPECTOR_AMBITUSTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class AmbitusTypes
{
    Q_GADGET

public:
    enum class TpcType {
        TPC_INVALID = -2,
        TPC_F_BB,
        TPC_C_BB,
        TPC_G_BB,
        TPC_D_BB,
        TPC_A_BB,
        TPC_E_BB,
        TPC_B_BB,
        TPC_F_B,
        TPC_C_B,
        TPC_G_B,
        TPC_D_B,
        TPC_A_B,
        TPC_E_B,
        TPC_B_B,
        TPC_F,
        TPC_C,
        TPC_G,
        TPC_D,
        TPC_A,
        TPC_E,
        TPC_B,
        TPC_F_S,
        TPC_C_S,
        TPC_G_S,
        TPC_D_S,
        TPC_A_S,
        TPC_E_S,
        TPC_B_S,
        TPC_F_SS,
        TPC_C_SS,
        TPC_G_SS,
        TPC_D_SS,
        TPC_A_SS,
        TPC_E_SS,
        TPC_B_SS
    };

    Q_ENUM(TpcType)
};
}

#endif // MU_INSPECTOR_AMBITUSTYPES_H
