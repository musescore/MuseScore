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

#include "modularity/imoduleexport.h"
#include "ret.h"
#include "io/path.h"

namespace mu::project {
class IProjectFilesController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IProjectFilesController)

public:
    virtual ~IProjectFilesController() = default;

    virtual bool isFileSupported(const io::path& path) const = 0;
    virtual Ret openProject(const io::path& path) = 0;
    virtual bool closeOpenedProject(bool quitApp = false) = 0;
    virtual bool isProjectOpened(const io::path& path) const = 0;
    virtual bool isAnyProjectOpened() const = 0;
    virtual bool saveProject(const io::path& path = io::path()) = 0;
};
}

#endif // MU_PROJECT_IPROJECTFILESCONTROLLER_H
