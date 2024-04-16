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
#ifndef MU_UPDATE_APPUPDATESERVICE_H
#define MU_UPDATE_APPUPDATESERVICE_H

#include "async/asyncable.h"

#include "global/types/version.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "iinteractive.h"
#include "network/inetworkmanagercreator.h"
#include "io/ifilesystem.h"
#include "global/isysteminfo.h"

#include "../iupdateconfiguration.h"
#include "../iappupdateservice.h"

namespace mu::update {
class AppUpdateService : public IAppUpdateService, public async::Asyncable
{
    INJECT(network::INetworkManagerCreator, networkManagerCreator);
    INJECT(io::IFileSystem, fileSystem);
    INJECT(ISystemInfo, systemInfo);
    INJECT(framework::IApplication, application);
    INJECT(framework::IInteractive, interactive);
    INJECT(IUpdateConfiguration, configuration);

public:
    RetVal<ReleaseInfo> checkForUpdate() override;

    RetVal<io::path_t> downloadRelease() override;
    void cancelUpdate() override;
    framework::Progress updateProgress() override;

private:
    friend class AppUpdateServiceTests;

    RetVal<ReleaseInfo> parseRelease(const QByteArray& json) const;

    std::string platformFileSuffix() const;
    ISystemInfo::CpuArchitecture assetArch(const QString& asset) const;
    QJsonObject resolveReleaseAsset(const QJsonObject& release) const;

    PrevReleasesNotesList previousReleasesNotes(const framework::Version& updateVersion) const;
    PrevReleasesNotesList parsePreviousReleasesNotes(const QByteArray& json) const;

    void clear();

    ReleaseInfo m_lastCheckResult;
    io::path_t m_installatorPath;

    network::INetworkManagerPtr m_networkManager;
    framework::Progress m_updateProgress;
};
}

#endif // MU_UPDATE_APPUPDATESERVICE_H
