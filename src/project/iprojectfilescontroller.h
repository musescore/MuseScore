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
#ifndef MU_PROJECT_IPROJECTFILESCONTROLLER_H
#define MU_PROJECT_IPROJECTFILESCONTROLLER_H

#include "modularity/imoduleinterface.h"
#include "types/ret.h"
#include "io/path.h"

#include "projecttypes.h"

namespace mu::project {
class IProjectFilesController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IProjectFilesController)

public:
    virtual ~IProjectFilesController() = default;

    virtual bool isFileSupported(const io::path_t& path) const = 0;
    virtual Ret openProject(const io::path_t& path) = 0;
    virtual bool closeOpenedProject(bool quitApp = false) = 0;
    virtual bool isProjectOpened(const io::path_t& path) const = 0;
    virtual bool isAnyProjectOpened() const = 0;
    virtual bool saveProject(const io::path_t& path = io::path_t()) = 0;

    virtual ProjectBeingDownloaded projectBeingDownloaded() const = 0;
    virtual async::Notification projectBeingDownloadedChanged() const = 0;
};
}

#endif // MU_PROJECT_IPROJECTFILESCONTROLLER_H
