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

#ifndef MU_PROJECT_SAVEPROJECTSCENARIO_H
#define MU_PROJECT_SAVEPROJECTSCENARIO_H

#include "isaveprojectscenario.h"

#include "modularity/ioc.h"
#include "iprojectconfiguration.h"
#include "global/iinteractive.h"

namespace mu::project {
class SaveProjectScenario : public ISaveProjectScenario
{
    INJECT(project, IProjectConfiguration, configuration)
    INJECT(project, framework::IInteractive, interactive)

public:
    SaveProjectScenario() = default;

    RetVal<SaveLocation> askSaveLocation(INotationProjectPtr project, SaveMode mode,
                                         SaveLocationType preselectedType = SaveLocationType::Undefined) const override;

    RetVal<io::path> askLocalPath(INotationProjectPtr project, SaveMode mode) const override;
    RetVal<SaveLocation::CloudInfo> askCloudLocation(INotationProjectPtr project,
                                                     CloudProjectVisibility defaultVisibility = CloudProjectVisibility::Private) const
    override;

private:
    RetVal<SaveLocationType> saveLocationType() const;
    RetVal<SaveLocationType> askSaveLocationType() const;

    RetVal<SaveLocation::CloudInfo> doAskCloudLocation(INotationProjectPtr project, bool canSaveLocallyInstead = true,
                                                       CloudProjectVisibility defaultVisibility = CloudProjectVisibility::Private) const;

    bool warnBeforePublishing() const;
};

class QMLSaveLocationType
{
    Q_GADGET

public:
    enum SaveLocationType {
        Undefined = int(project::SaveLocationType::Undefined),
        Local = int(project::SaveLocationType::Local),
        Cloud = int(project::SaveLocationType::Cloud)
    };
    Q_ENUM(SaveLocationType);
};

class QMLCloudVisibility
{
    Q_GADGET

public:
    enum CloudVisibility {
        Private = int(CloudProjectVisibility::Private),
        Public = int(CloudProjectVisibility::Public)
    };
    Q_ENUM(CloudVisibility);
};

class QMLSaveToCloudResponse
{
    Q_GADGET

public:
    enum SaveToCloudResponse {
        Cancel,
        Ok,
        SaveLocallyInstead
    };
    Q_ENUM(SaveToCloudResponse);
};
}

#endif // MU_PROJECT_SAVEPROJECTSCENARIO_H
