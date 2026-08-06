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

#include "global/modularity/imoduleinterface.h"
#include "global/async/notification.h"

namespace mu::project {
class IProjectCommandsController : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IProjectCommandsController)

public:
    virtual ~IProjectCommandsController() = default;

    enum class BusyStatus {
        Opening,
        Saving,
        Closing,
        Downloading,
        Uploading,
        Publishing,
        AudioSharing
    };

    virtual bool hasProject() const = 0;
    virtual muse::async::Notification hasProjectChanged() const = 0;

    virtual bool needSave() const = 0;
    virtual muse::async::Notification needSaveChanged() const = 0;

    virtual bool isBusy(BusyStatus status) const = 0;
    virtual muse::async::Notification busyChanged() const = 0;

    virtual bool hasSelection() const = 0;
    virtual muse::async::Notification hasSelectionChanged() const = 0;
};
}
