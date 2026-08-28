/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited and others
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
#include "iprojectfilescontroller.h"
#include "types/projectfile.h"

namespace mu::project {
class IOpenProjectScenario : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IOpenProjectScenario)

public:
    virtual ~IOpenProjectScenario() = default;

    virtual muse::Ret openProject(const ProjectFile& file) = 0;
    virtual muse::Ret openProject(const muse::io::path_t& path, const QString& displayNameOverride = QString()) = 0;
    virtual muse::Ret openProject(const muse::rcommand::Params& params) = 0;

    virtual void revertToLastSaved() = 0;

    virtual muse::Ret finishOpening() = 0;

    virtual const ProjectBeingDownloaded& projectBeingDownloaded() const = 0;
    virtual muse::async::Notification projectBeingDownloadedChanged() const = 0;

    virtual bool isBusy(IProjectCommandsController::BusyStatus status) const = 0;
    virtual muse::async::Notification busyChanged() const = 0;
};
}
