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
#ifndef MU_USERSCORES_IFILESCORECONTROLLER_H
#define MU_USERSCORES_IFILESCORECONTROLLER_H

#include "modularity/imoduleexport.h"
#include "ret.h"
#include "io/path.h"

namespace mu::userscores {
class IFileScoreController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IFileScoreController)

public:
    virtual ~IFileScoreController() = default;

    virtual Ret openProject(const io::path& scorePath) = 0;
    virtual bool closeOpenedProject() = 0;
    virtual bool isProjectOpened(const io::path& scorePath) const = 0;
};
}

#endif // MU_USERSCORES_IFILESCORECONTROLLER_H
