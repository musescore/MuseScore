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
#ifndef MUSE_UPDATE_APPUPDATESERVICE_H
#define MUSE_UPDATE_APPUPDATESERVICE_H

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

namespace muse::update {
class AppUpdateService : public IAppUpdateService, public Injectable, public async::Asyncable
{
    Inject<network::INetworkManagerCreator> networkManagerCreator = { this };
    Inject<io::IFileSystem> fileSystem = { this };
    Inject<ISystemInfo> systemInfo = { this };
    Inject<IApplication> application = { this };
    Inject<IInteractive> interactive = { this };
    Inject<IUpdateConfiguration> configuration = { this };

public:
    AppUpdateService(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    RetVal<ReleaseInfo> checkForUpdate() override;

    RetVal<io::path_t> downloadRelease() override;
    void cancelUpdate() override;
    Progress updateProgress() override;

private:
    friend class AppUpdateServiceTests;

    struct UpdateRequestHistory {
        QDate installedWeekBeginning;
        QDate previousRequestDay;
        bool isValid() const { return installedWeekBeginning.isValid() && previousRequestDay.isValid(); }
    };

    RetVal<UpdateRequestHistory> readUpdateRequestHistory(const io::path_t& path) const;
    Ret writeUpdateRequestHistory(const io::path_t& path, const UpdateRequestHistory& updateRequestHistory);

    RetVal<ReleaseInfo> parseRelease(const QByteArray& json) const;

    std::string platformFileSuffix() const;
    ISystemInfo::CpuArchitecture assetArch(const QString& asset) const;
    QJsonObject resolveReleaseAsset(const QJsonObject& release) const;

    PrevReleasesNotesList previousReleasesNotes(const Version& updateVersion) const;
    PrevReleasesNotesList parsePreviousReleasesNotes(const QByteArray& json) const;

    void clear();

    ReleaseInfo m_lastCheckResult;
    io::path_t m_installatorPath;

    network::INetworkManagerPtr m_networkManager;
    Progress m_updateProgress;
};
}

#endif // MUSE_UPDATE_APPUPDATESERVICE_H
