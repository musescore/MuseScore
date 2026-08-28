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

#include "modularity/imoduleinterface.h"
#include "async/notification.h"
#include "io/path.h"
#include "rcommand/commandtypes.h"

#include "iprojectcommandscontroller.h"
#include "types/projecttypes.h"
#include "types/savelocation.h"

namespace mu::project {
class ISaveProjectScenario : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(ISaveProjectScenario)

public:
    virtual ~ISaveProjectScenario() = default;

    virtual muse::Ret saveProject(SaveMode saveMode, SaveLocationType saveLocationType = SaveLocationType::Undefined,
                                  bool force = false) = 0;
    virtual bool saveProject(const muse::io::path_t& path = muse::io::path_t()) = 0;
    virtual bool saveProjectLocally(const muse::io::path_t& path, SaveMode saveMode = SaveMode::Save, bool createBackup = true) = 0;
    virtual muse::Ret saveProjectAt(const muse::rcommand::Params& params) = 0;

    virtual muse::Ret publish() = 0;
    virtual muse::Ret shareAudio() = 0;

    virtual bool isBusy(IProjectCommandsController::BusyStatus status) const = 0;
    virtual muse::async::Notification busyChanged() const = 0;
};
}
