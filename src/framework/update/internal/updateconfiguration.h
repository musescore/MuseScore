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
#ifndef MUSE_UPDATE_UPDATECONFIGURATION_H
#define MUSE_UPDATE_UPDATECONFIGURATION_H

#include "../iupdateconfiguration.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "network/inetworkconfiguration.h"

#include "global/types/config.h"

namespace muse::update {
class UpdateConfiguration : public IUpdateConfiguration, public Injectable, public async::Asyncable
{
    Inject<IGlobalConfiguration> globalConfiguration = { this };
    Inject<network::INetworkConfiguration> networkConfiguration = { this };

public:
    UpdateConfiguration(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    bool isAppUpdatable() const override;

    bool allowUpdateOnPreRelease() const override;
    void setAllowUpdateOnPreRelease(bool allow) override;

    bool needCheckForUpdate() const override;
    void setNeedCheckForUpdate(bool needCheck) override;
    muse::async::Notification needCheckForUpdateChanged() const override;

    std::string skippedReleaseVersion() const override;
    void setSkippedReleaseVersion(const std::string& version) override;

    std::string checkForAppUpdateUrl() const override;
    std::string previousAppReleasesNotesUrl() const override;

    muse::network::RequestHeaders updateHeaders() const override;

    std::string museScoreUrl() const override;
    std::string museScorePrivacyPolicyUrl() const override;

    muse::io::path_t updateDataPath() const override;
    muse::io::path_t updateRequestHistoryJsonPath() const override;

private:
    muse::async::Notification m_needCheckForUpdateChanged;

    Config m_config;
};
}

#endif // MUSE_UPDATE_UPDATECONFIGURATION_H
