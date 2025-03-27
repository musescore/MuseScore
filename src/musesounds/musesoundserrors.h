/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "types/ret.h"

namespace mu::musesounds {
enum class Err {
    Undefined       = int(muse::Ret::Code::Undefined),
    NoError         = int(muse::Ret::Code::Ok),
    UnknownError    = int(muse::Ret::Code::MuseSoundsFirst),

    NoUpdate
};

inline muse::Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return muse::Ret(retCode);
    case Err::NoError: return muse::Ret(retCode);
    case Err::UnknownError: return muse::Ret(retCode);
    case Err::NoUpdate: return muse::Ret(retCode, "No update");
        break;
    }

    return muse::Ret(retCode);
}
}
