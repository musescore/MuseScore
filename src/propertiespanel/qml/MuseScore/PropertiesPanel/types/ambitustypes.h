/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <qqmlintegration.h>

#include "engraving/dom/pitchspelling.h"

namespace mu::propertiespanel {
namespace AmbitusTypes {
Q_NAMESPACE;
QML_ELEMENT;

enum class TpcType {
    TPC_INVALID = int(engraving::TPC_INVALID),
    TPC_F_BB = int(engraving::TPC_F_BB),
    TPC_C_BB = int(engraving::TPC_C_BB),
    TPC_G_BB = int(engraving::TPC_G_BB),
    TPC_D_BB = int(engraving::TPC_D_BB),
    TPC_A_BB = int(engraving::TPC_A_BB),
    TPC_E_BB = int(engraving::TPC_E_BB),
    TPC_B_BB = int(engraving::TPC_B_BB),
    TPC_F_B = int(engraving::TPC_F_B),
    TPC_C_B = int(engraving::TPC_C_B),
    TPC_G_B = int(engraving::TPC_G_B),
    TPC_D_B = int(engraving::TPC_D_B),
    TPC_A_B = int(engraving::TPC_A_B),
    TPC_E_B = int(engraving::TPC_E_B),
    TPC_B_B = int(engraving::TPC_B_B),
    TPC_F = int(engraving::TPC_F),
    TPC_C = int(engraving::TPC_C),
    TPC_G = int(engraving::TPC_G),
    TPC_D = int(engraving::TPC_D),
    TPC_A = int(engraving::TPC_A),
    TPC_E = int(engraving::TPC_E),
    TPC_B = int(engraving::TPC_B),
    TPC_F_S = int(engraving::TPC_F_S),
    TPC_C_S = int(engraving::TPC_C_S),
    TPC_G_S = int(engraving::TPC_G_S),
    TPC_D_S = int(engraving::TPC_D_S),
    TPC_A_S = int(engraving::TPC_A_S),
    TPC_E_S = int(engraving::TPC_E_S),
    TPC_B_S = int(engraving::TPC_B_S),
    TPC_F_SS = int(engraving::TPC_F_SS),
    TPC_C_SS = int(engraving::TPC_C_SS),
    TPC_G_SS = int(engraving::TPC_G_SS),
    TPC_D_SS = int(engraving::TPC_D_SS),
    TPC_A_SS = int(engraving::TPC_A_SS),
    TPC_E_SS = int(engraving::TPC_E_SS),
    TPC_B_SS = int(engraving::TPC_B_SS)
};

Q_ENUM_NS(TpcType)
}
}
