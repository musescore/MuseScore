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

#ifndef MU_PROJECT_OPENSAVEPROJECTSCENARIO_H
#define MU_PROJECT_OPENSAVEPROJECTSCENARIO_H

#include "iopensaveprojectscenario.h"

#include "modularity/ioc.h"
#include "iprojectconfiguration.h"
#include "global/iinteractive.h"

#include "cloud/musescorecom/imusescorecomservice.h"
#include "cloud/audiocom/iaudiocomservice.h"

namespace mu::project {
class OpenSaveProjectScenario : public IOpenSaveProjectScenario
{
    INJECT(IProjectConfiguration, configuration)
    INJECT(framework::IInteractive, interactive)
    INJECT(cloud::IMuseScoreComService, museScoreComService)
    INJECT(cloud::IAudioComService, audioComService)

public:
    OpenSaveProjectScenario() = default;

    RetVal<SaveLocation> askSaveLocation(INotationProjectPtr project, SaveMode mode,
                                         SaveLocationType preselectedType = SaveLocationType::Undefined) const override;

    RetVal<io::path_t> askLocalPath(INotationProjectPtr project, SaveMode mode) const override;
    RetVal<CloudProjectInfo> askCloudLocation(INotationProjectPtr project, SaveMode mode) const override;
    RetVal<CloudProjectInfo> askPublishLocation(INotationProjectPtr project) const override;
    RetVal<CloudAudioInfo> askShareAudioLocation(INotationProjectPtr project) const override;

    bool warnBeforeSavingToExistingPubliclyVisibleCloudProject() const override;

    void showCloudOpenError(const Ret& ret) const override;
    Ret showCloudSaveError(const Ret& ret, const CloudProjectInfo& info, bool isPublish, bool alreadyAttempted) const override;
    Ret showAudioCloudShareError(const Ret& ret) const override;

private:
    RetVal<SaveLocationType> saveLocationType() const;
    RetVal<SaveLocationType> askSaveLocationType() const;

    /// \param isPublish:
    ///     false -> this is part of a "Save to cloud" action
    ///     true -> this is part of a "Publish" action
    RetVal<CloudProjectInfo> doAskCloudLocation(INotationProjectPtr project, SaveMode mode, bool isPublish) const;

    bool warnBeforePublishing(bool isPublish, cloud::Visibility visibility) const;

    Ret warnCloudNotAvailableForUploading(bool isPublish) const;
    Ret warnCloudNotAvailableForSharingAudio() const;
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

#endif // MU_PROJECT_OPENSAVEPROJECTSCENARIO_H
