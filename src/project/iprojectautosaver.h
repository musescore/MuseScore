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
#ifndef MU_PROJECT_IPROJECTAUTOSAVER_H
#define MU_PROJECT_IPROJECTAUTOSAVER_H

#include "io/path.h"

#include "modularity/imoduleexport.h"

namespace mu::project {
class IProjectAutoSaver : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IProjectAutoSaver)

public:
    virtual ~IProjectAutoSaver() = default;

    virtual bool projectHasUnsavedChanges(const io::path& projectPath) const = 0;
    virtual void removeProjectUnsavedChanges(const io::path& projectPath) = 0;

    virtual io::path projectOriginalPath(const io::path& projectAutoSavePath) const = 0;
    virtual io::path projectAutoSavePath(const io::path& projectPath) const = 0;
};
}

#endif // MU_PROJECT_IPROJECTAUTOSAVER_H
