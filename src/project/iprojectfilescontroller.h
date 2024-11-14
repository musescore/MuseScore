/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "types/projecttypes.h"

class QUrl;

namespace mu::project {
class IProjectFilesController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IProjectFilesController)

public:
    virtual ~IProjectFilesController() = default;

    virtual bool isUrlSupported(const QUrl& url) const = 0;
    virtual bool isFileSupported(const muse::io::path_t& path) const = 0;
    virtual muse::Ret openProject(const ProjectFile& file) = 0;
    virtual bool closeOpenedProject(bool quitApp = false) = 0;
    virtual bool saveProject(const muse::io::path_t& path = muse::io::path_t()) = 0;
    virtual bool saveProjectLocally(
        const muse::io::path_t& path = muse::io::path_t(), SaveMode saveMode = SaveMode::Save, bool createBackup = true) = 0;

    virtual const ProjectBeingDownloaded& projectBeingDownloaded() const = 0;
    virtual muse::async::Notification projectBeingDownloadedChanged() const = 0;
};
}

#endif // MU_PROJECT_IPROJECTFILESCONTROLLER_H
