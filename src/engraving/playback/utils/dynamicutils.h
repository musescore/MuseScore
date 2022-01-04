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

#ifndef MU_ENGRAVING_DYNAMICUTILS_H
#define MU_ENGRAVING_DYNAMICUTILS_H

#include "mpe/mpetypes.h"

#include "types/types.h"

namespace mu::engraving {
inline mpe::DynamicTypeList dynamicType(const DynamicType type)
{
    switch (type) {
    case DynamicType::OTHER: return { mpe::DynamicType::Undefined };
    case DynamicType::PPPPPP: return { mpe::DynamicType::pppppp };
    case DynamicType::PPPPP: return { mpe::DynamicType::ppppp };
    case DynamicType::PPPP: return { mpe::DynamicType::pppp };
    case DynamicType::PPP: return { mpe::DynamicType::ppp };
    case DynamicType::PP: return { mpe::DynamicType::pp };
    case DynamicType::P: return { mpe::DynamicType::p };
    case DynamicType::MP: return { mpe::DynamicType::mp };
    case DynamicType::MF: return { mpe::DynamicType::mf };
    case DynamicType::F: return { mpe::DynamicType::f };
    case DynamicType::FF: return { mpe::DynamicType::ff };
    case DynamicType::FFF: return { mpe::DynamicType::fff };
    case DynamicType::FFFF: return { mpe::DynamicType::ffff };
    case DynamicType::FFFFF: return { mpe::DynamicType::fffff };
    case DynamicType::FFFFFF: return { mpe::DynamicType::ffffff };

    case DynamicType::SFP:
    case DynamicType::FP:
        return { mpe::DynamicType::f, mpe::DynamicType::p };

    case DynamicType::SF:
    case DynamicType::SFZ:
    case DynamicType::RFZ:
    case DynamicType::RF:
    case DynamicType::FZ:
        return { mpe::DynamicType::f };

    case DynamicType::SFF:
    case DynamicType::SFFZ:
        return { mpe::DynamicType::ff };

    case DynamicType::SFPP:
        return { mpe::DynamicType::f, mpe::DynamicType::pp };

    default: return { mpe::DynamicType::Natural };
    }
}
}

#endif // MU_ENGRAVING_DYNAMICUTILS_H
