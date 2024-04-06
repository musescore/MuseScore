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
#ifndef MU_UPDATE_UPDATESERVICE_H
#define MU_UPDATE_UPDATESERVICE_H

#include "async/asyncable.h"

#include "global/types/version.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "iinteractive.h"
#include "network/inetworkmanagercreator.h"
#include "io/ifilesystem.h"
#include "global/isysteminfo.h"

#include "../iupdateconfiguration.h"
#include "../iupdateservice.h"

namespace mu::update {
class UpdateService : public IUpdateService, public async::Asyncable
{
    INJECT(IInteractive, interactive)
    INJECT(muse::network::INetworkManagerCreator, networkManagerCreator)
    INJECT(IUpdateConfiguration, configuration)
    INJECT(io::IFileSystem, fileSystem)
    INJECT(ISystemInfo, systemInfo)
    INJECT(IApplication, application)

public:
    RetVal<ReleaseInfo> checkForUpdate() override;

    RetVal<io::path_t> downloadRelease() override;
    void cancelUpdate() override;
    mu::Progress updateProgress() override;

private:
    RetVal<ReleaseInfo> parseRelease(const QByteArray& json) const;

    std::string platformFileSuffix() const;
    ISystemInfo::CpuArchitecture assetArch(const QString& asset) const;
    QJsonObject resolveReleaseAsset(const QJsonObject& release) const;

    PrevReleasesNotesList previousReleasesNotes(const Version& updateVersion) const;
    PrevReleasesNotesList parsePreviousReleasesNotes(const QByteArray& json) const;

    void clear();

    ReleaseInfo m_lastCheckResult;
    io::path_t m_installatorPath;

    muse::network::INetworkManagerPtr m_networkManager;
    mu::Progress m_updateProgress;
};
}

#endif // MU_UPDATE_UPDATESERVICE_H
