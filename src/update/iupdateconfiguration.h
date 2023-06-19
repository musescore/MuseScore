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
#ifndef MU_UPDATE_IUPDATECONFIGURATION_H
#define MU_UPDATE_IUPDATECONFIGURATION_H

#include "io/path.h"
#include "network/networktypes.h"

#include "modularity/imoduleinterface.h"

namespace mu::update {
class IUpdateConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IUpdateConfiguration)

public:
    virtual ~IUpdateConfiguration() = default;

    virtual bool isAppUpdatable() const = 0;

    virtual bool isTestingMode() const = 0;
    virtual void setIsTestingMode(bool isTesting) = 0;

    virtual bool needCheckForUpdate() const = 0;
    virtual void setNeedCheckForUpdate(bool needCheck) = 0;

    virtual std::string skippedReleaseVersion() const = 0;
    virtual void setSkippedReleaseVersion(const std::string& version) const = 0;

    virtual std::string checkForUpdateUrl() const = 0;
    virtual network::RequestHeaders checkForUpdateHeaders() const = 0;

    virtual std::string museScoreUrl() const = 0;
    virtual std::string museScorePrivacyPolicyUrl() const = 0;

    virtual io::path_t updateDataPath() const = 0;
};
}

#endif // MU_UPDATE_IUPDATECONFIGURATION_H
