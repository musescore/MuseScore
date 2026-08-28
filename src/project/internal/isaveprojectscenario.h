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
#include "types/retval.h"

#include "iprojectcommandscontroller.h"
#include "types/projecttypes.h"
#include "types/savelocation.h"

namespace mu::project {
//! NOTE Owns the whole "where does this score go, and how" flow: it asks the user whatever it needs
//! to, writes the file, and uploads it. Callers hand over the intent and get back the outcome.
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

    //! NOTE Raised when the user answered a corruption dialog with "Revert to last saved". Reopening
    //! the file belongs to the open flow, so performing it is left to the listener.
    virtual muse::async::Notification revertToLastSavedRequested() const = 0;
};
}
