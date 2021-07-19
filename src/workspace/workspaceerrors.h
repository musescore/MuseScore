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
#ifndef MU_WORKSPACE_WORKSPACEERRORS_H
#define MU_WORKSPACE_WORKSPACEERRORS_H

#include "ret.h"

namespace mu::workspace {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::WorkspaceFirst),

    NotLoaded       = 1502,
    NoData          = 1503,

    FailedPack      = 1510,
    FailedUnPack    = 1511,
};

inline mu::Ret make_ret(Err e)
{
    return Ret(static_cast<int>(e));
}
}

#endif // MU_WORKSPACE_WORKSPACEERRORS_H
