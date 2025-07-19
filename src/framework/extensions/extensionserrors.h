/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include "global/types/ret.h"
#include "global/translation.h"

namespace muse::extensions {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::ExtensionsFirst),

    ExtNotFound,
    ExtLoadError,
    ExtBadFormat
};

inline Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::NoError: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::ExtNotFound: return Ret(retCode, muse::trc("extensions", "Plugin not found"));
    case Err::ExtLoadError: return Ret(retCode, muse::trc("extensions", "Could not load plugin"));
    case Err::ExtBadFormat: return Ret(retCode, muse::trc("extensions", "Plugin is not in the correct format"));
    }

    return Ret(static_cast<int>(e));
}
}
