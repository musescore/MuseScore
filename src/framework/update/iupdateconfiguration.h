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
#ifndef MUSE_UPDATE_IUPDATECONFIGURATION_H
#define MUSE_UPDATE_IUPDATECONFIGURATION_H

#include "async/notification.h"
#include "io/path.h"
#include "network/networktypes.h"

#include "modularity/imoduleinterface.h"

namespace muse::update {
class IUpdateConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IUpdateConfiguration)

public:
    virtual ~IUpdateConfiguration() = default;

    virtual bool isAppUpdatable() const = 0;

    virtual bool allowUpdateOnPreRelease() const = 0;
    virtual void setAllowUpdateOnPreRelease(bool allow) = 0;

    virtual bool needCheckForUpdate() const = 0;
    virtual void setNeedCheckForUpdate(bool needCheck) = 0;
    virtual muse::async::Notification needCheckForUpdateChanged() const = 0;

    virtual std::string skippedReleaseVersion() const = 0;
    virtual void setSkippedReleaseVersion(const std::string& version) = 0;

    virtual std::string checkForAppUpdateUrl() const = 0;
    virtual std::string previousAppReleasesNotesUrl() const = 0;

    virtual muse::network::RequestHeaders updateHeaders() const = 0;

    virtual std::string museScoreUrl() const = 0;
    virtual std::string museScorePrivacyPolicyUrl() const = 0;

    virtual muse::io::path_t updateDataPath() const = 0;
    virtual muse::io::path_t updateRequestHistoryJsonPath() const = 0;
};
}

#endif // MUSE_UPDATE_IUPDATECONFIGURATION_H
