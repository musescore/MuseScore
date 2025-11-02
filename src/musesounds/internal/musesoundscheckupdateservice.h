/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "network/inetworkmanagercreator.h"
#include "io/ifilesystem.h"
#include "global/isysteminfo.h"
#include "languages/ilanguagesconfiguration.h"
#include "iinteractive.h"

#include "../global/iglobalconfiguration.h"
#include "../imusesoundsconfiguration.h"
#include "../imusesoundscheckupdateservice.h"

namespace mu::musesounds {
class MuseSoundsCheckUpdateService : public IMuseSoundsCheckUpdateService, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<muse::network::INetworkManagerCreator> networkManagerCreator = { this };
    muse::Inject<muse::IGlobalConfiguration> globalConfiguration = { this };
    muse::Inject<muse::io::IFileSystem> fileSystem = { this };
    muse::Inject<muse::ISystemInfo> systemInfo = { this };
    muse::Inject<muse::languages::ILanguagesConfiguration> languagesConfiguration = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<IMuseSoundsConfiguration> configuration = { this };

public:

    MuseSoundsCheckUpdateService(const muse::modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    muse::Ret needCheckForUpdate() const override;

    muse::RetVal<muse::update::ReleaseInfo> checkForUpdate() override;
    muse::RetVal<muse::update::ReleaseInfo> lastCheckResult() override;

    muse::Progress updateProgress() override;

private:
    muse::RetVal<muse::update::ReleaseInfo> parseRelease(const QByteArray& json) const;

    void clear();

    muse::RetVal<muse::update::ReleaseInfo> m_lastCheckResult;
    muse::io::path_t m_installatorPath;
    muse::Progress m_updateProgress;
};
}
