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
#include "cloud/iauthorizationservice.h"
#include "cloud/icloudprojectsservice.h"

namespace mu::project {
class SaveProjectScenario : public ISaveProjectScenario
{
    INJECT(project, IProjectConfiguration, configuration)
    INJECT(project, framework::IInteractive, interactive)
    INJECT(project, cloud::IAuthorizationService, authorizationService)
    INJECT(project, cloud::ICloudProjectsService, cloudProjectsService)

public:
    SaveProjectScenario() = default;

    RetVal<SaveLocation> askSaveLocation(INotationProjectPtr project, SaveMode mode,
                                         SaveLocationType preselectedType = SaveLocationType::Undefined) const override;

    RetVal<io::path_t> askLocalPath(INotationProjectPtr project, SaveMode mode) const override;
    RetVal<CloudProjectInfo> askCloudLocation(INotationProjectPtr project, SaveMode mode) const override;
    RetVal<CloudProjectInfo> askPublishLocation(INotationProjectPtr project) const override;

    bool warnBeforeSavingToExistingPubliclyVisibleCloudProject() const override;

    Ret showCloudSaveError(const Ret& ret, const CloudProjectInfo& info, bool isPublish, bool alreadyAttempted) const override;

private:
    RetVal<SaveLocationType> saveLocationType() const;
    RetVal<SaveLocationType> askSaveLocationType() const;

    /// \param isPublish:
    ///     false -> this is part of a "Save to cloud" action
    ///     true -> this is part of a "Publish" action
    RetVal<CloudProjectInfo> doAskCloudLocation(INotationProjectPtr project, SaveMode mode, bool isPublish) const;

    bool warnBeforePublishing(bool isPublish, cloud::Visibility visibility) const;

    Ret warnCloudIsNotAvailable(bool isPublish) const;
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
        Public = int(cloud::Visibility::Public),
        Unlisted = int(cloud::Visibility::Unlisted),
        Private = int(cloud::Visibility::Private)
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
