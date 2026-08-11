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

#include "global/async/channel.h"
#include "rcommand/commandtypes.h"

#include "../appshelltypes.h"

namespace mu::appshell {
class IAppshellCommandsController : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IAppshellCommandsController)
public:
    virtual ~IAppshellCommandsController() = default;

    virtual muse::rcommand::Command dockToggleCommand(const DockName& dockName) const = 0;
    virtual DockName commandDockName(const muse::rcommand::Command& command) const = 0;
    virtual muse::async::Channel<DockName> dockToggleRequested() const = 0;
};
}
