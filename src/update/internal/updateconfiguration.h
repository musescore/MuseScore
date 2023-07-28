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
#ifndef MU_UPDATE_UPDATECONFIGURATION_H
#define MU_UPDATE_UPDATECONFIGURATION_H

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

#include "../iupdateconfiguration.h"

namespace mu::update {
class UpdateConfiguration : public IUpdateConfiguration, public async::Asyncable
{
    INJECT(framework::IGlobalConfiguration, globalConfiguration)

public:
    void init();

    bool isAppUpdatable() const override;

    bool allowUpdateOnPreRelease() const override;
    void setAllowUpdateOnPreRelease(bool allow) override;

    bool needCheckForUpdate() const override;
    void setNeedCheckForUpdate(bool needCheck) override;

    std::string skippedReleaseVersion() const override;
    void setSkippedReleaseVersion(const std::string& version) const override;

    std::string checkForUpdateUrl() const override;
    network::RequestHeaders checkForUpdateHeaders() const override;

    std::string museScoreUrl() const override;
    std::string museScorePrivacyPolicyUrl() const override;

    io::path_t updateDataPath() const override;
};
}

#endif // MU_UPDATE_UPDATECONFIGURATION_H
