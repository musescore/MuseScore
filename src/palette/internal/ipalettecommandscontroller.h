/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore Limited and others
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

#include "global/modularity/imoduleinterface.h"

#include "global/async/notification.h"
#include "global/async/channel.h"

namespace mu::palette {
class IPaletteCommandsController : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IPaletteCommandsController)

public:
    virtual ~IPaletteCommandsController() = default;

    virtual muse::async::Notification paletteSearchRequested() const = 0;
    virtual muse::async::Notification applyCurrentPaletteElementRequested() const = 0;
    virtual muse::async::Channel<bool /* expand */> expandCollapseAllRequested() const = 0;
};
}
