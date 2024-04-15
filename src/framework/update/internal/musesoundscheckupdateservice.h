/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MUSE_UPDATE_MUSESAMPLERUPDATESERVICE_H
#define MUSE_UPDATE_MUSESAMPLERUPDATESERVICE_H

#include "async/asyncable.h"

#include "global/types/version.h"

#include "modularity/ioc.h"
#include "network/inetworkmanagercreator.h"
#include "io/ifilesystem.h"
#include "global/isysteminfo.h"
#include "languages/ilanguagesconfiguration.h"
#include "iinteractive.h"

#include "../iupdateconfiguration.h"
#include "../imusesoundscheckupdateservice.h"

namespace muse::update {
class MuseSoundsCheckUpdateService : public IMuseSoundsCheckUpdateService, public async::Asyncable
{
    Inject<network::INetworkManagerCreator> networkManagerCreator;
    Inject<IUpdateConfiguration> configuration;
    Inject<io::IFileSystem> fileSystem;
    Inject<ISystemInfo> systemInfo;
    Inject<languages::ILanguagesConfiguration> languagesConfiguration;
    Inject<IInteractive> interactive;

public:
    Ret needCheckForUpdate() const override;

    RetVal<ReleaseInfo> checkForUpdate() override;
    RetVal<ReleaseInfo> lastCheckResult() override;

    Progress updateProgress() override;

    void openMuseHub() override;

private:
    RetVal<ReleaseInfo> parseRelease(const QByteArray& json) const;

    void clear();

    RetVal<ReleaseInfo> m_lastCheckResult;
    io::path_t m_installatorPath;

    network::INetworkManagerPtr m_networkManager;
    Progress m_updateProgress;
};
}

#endif // MUSE_UPDATE_MUSESAMPLERUPDATESERVICE_H
