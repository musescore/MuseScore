/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "modularity/imoduleinterface.h"

#include "types/retval.h"
#include "async/promise.h"

#include "update/updatetypes.h"

namespace mu::musesounds {
class IMuseSoundsCheckUpdateService : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IMuseSamplerUpdateService)

public:
    virtual ~IMuseSoundsCheckUpdateService() = default;

    virtual muse::Ret needCheckForUpdate() const = 0;

    virtual muse::async::Promise<muse::RetVal<muse::update::ReleaseInfo> > checkForUpdate() = 0;
    virtual const muse::RetVal<muse::update::ReleaseInfo>& lastCheckResult() const = 0;
};
}
