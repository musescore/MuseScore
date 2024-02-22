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
#ifndef MU_CONVERTER_CONVERTERCODES_H
#define MU_CONVERTER_CONVERTERCODES_H

#include "types/ret.h"

namespace mu::converter {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::ConverterFirst), // 1300

    BatchJobFileFailedOpen = 1301,
    BatchJobFileFailedParse = 1302,

    ConvertFailed = 1303,

    ConvertTypeUnknown = 1310,

    InFileFailedLoad = 1320,

    OutFileFailedOpen = 1330,
    OutFileFailedWrite = 1331,
};

inline Ret make_ret(Err e)
{
    return Ret(static_cast<int>(e));
}

inline Ret make_ret(Err e, const std::string& text)
{
    return Ret(static_cast<int>(e), text);
}
}

#endif // MU_CONVERTER_CONVERTERCODES_H
