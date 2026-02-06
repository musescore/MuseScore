/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#pragma once

#include "update/iappupdateservice.h"
#include "global/modularity/ioc.h"
#include "global/async/asyncable.h"

#include "global/io/ifilesystem.h"
#include "global/iapplication.h"
#include "global/isysteminfo.h"

#include "network/inetworkmanagercreator.h"
#include "update/iupdateconfiguration.h"

namespace muse::update {
class AppUpdateService : public IAppUpdateService, public Contextable, public async::Asyncable
{
    GlobalInject<io::IFileSystem> fileSystem;
    GlobalInject<ISystemInfo> systemInfo;
    GlobalInject<IUpdateConfiguration> configuration;
    GlobalInject<network::INetworkManagerCreator> networkManagerCreator;
    GlobalInject<IApplication> application;

public:
    AppUpdateService(const modularity::ContextPtr& iocCtx)
        : Contextable(iocCtx) {}

    void init();

    async::Promise<RetVal<ReleaseInfo> > checkForUpdate() override;
    const RetVal<ReleaseInfo>& lastCheckResult() const override;
    RetVal<Progress> downloadRelease() override;

private:
    friend class AppUpdateServiceTests;

    struct UpdateRequestHistory {
        QDate installedWeekBeginning;
        QDate previousRequestDay;
        bool isValid() const { return installedWeekBeginning.isValid() && previousRequestDay.isValid(); }
    };

    network::RequestHeaders prepareHeaders(const UpdateRequestHistory& history) const;
    RetVal<UpdateRequestHistory> readUpdateRequestHistory(const io::path_t& path) const;
    Ret writeUpdateRequestHistory(const io::path_t& path, const UpdateRequestHistory& updateRequestHistory);

    RetVal<ReleaseInfo> parseRelease(const QByteArray& json) const;

    std::string platformFileSuffix() const;
    QJsonObject resolveReleaseAsset(const QJsonObject& release) const;

    using PrevReleaseNotesCallback = std::function<void (const PrevReleasesNotesList&)>;
    void downloadPreviousReleasesNotes(const Version& updateVersion, const PrevReleaseNotesCallback& finished);

    void clear();

    RetVal<ReleaseInfo> m_lastCheckResult;
    network::INetworkManagerPtr m_networkManager;
    Progress m_updateProgress;
};
}
