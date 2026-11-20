/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <string>

#include "modularity/imoduleinterface.h"
#include "async/promise.h"
#include "types/ret.h"
#include "types/string.h"

#include "musesoundstypes.h"

namespace mu::musesounds {
class IMuseHubLauncher : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IMuseHubLauncher)

public:
    virtual ~IMuseHubLauncher() = default;

    //! Opens the link in MuseHub if possible, otherwise opens its web page
    //! Resolves once either has been opened
    virtual muse::async::Promise<muse::Ret> open(MuseHubPage page, const std::string& utmContent = std::string()) = 0;
    virtual muse::async::Promise<muse::Ret> openSoundLibrary(const muse::String& code, const std::string& utmContent = std::string()) = 0;
};
}
