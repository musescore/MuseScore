/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MU_PROJECT_ISAVEPROJECTSCENARIO_H
#define MU_PROJECT_ISAVEPROJECTSCENARIO_H

#include "modularity/imoduleexport.h"
#include "inotationproject.h"

#include "types/retval.h"

namespace mu::project {
class ISaveProjectScenario : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISaveProjectScenario)

public:
    virtual RetVal<SaveLocation> askSaveLocation(INotationProjectPtr project, SaveMode mode,
                                                 SaveLocationType preselectedType = SaveLocationType::Undefined) const = 0;

    virtual RetVal<io::path_t> askLocalPath(INotationProjectPtr project, SaveMode mode) const = 0;
    virtual RetVal<CloudProjectInfo> askCloudLocation(INotationProjectPtr project, SaveMode mode) const = 0;
    virtual RetVal<CloudProjectInfo> askPublishLocation(INotationProjectPtr project) const = 0;

    virtual bool warnBeforeSavingToExistingPubliclyVisibleCloudProject() const = 0;

    virtual void showCloudSaveError(const Ret& ret, bool publishMode, bool alreadyAttempted) const = 0;
};
}

#endif // MU_PROJECT_ISAVEPROJECTSCENARIO_H
