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
#ifndef MU_CLOUD_ICLOUDCONFIGURATION_H
#define MU_CLOUD_ICLOUDCONFIGURATION_H

#include "modularity/imoduleinterface.h"
#include "io/path.h"
#include "network/networktypes.h"

namespace mu::cloud {
class ICloudConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ICloudConfiguration)

public:
    virtual ~ICloudConfiguration() = default;

    virtual network::RequestHeaders headers() const = 0;
    virtual QByteArray uploadingLicense() const = 0;

    virtual QUrl cloudUrl() const = 0;
    virtual QUrl authorizationUrl() const = 0;
    virtual QUrl signUpUrl() const = 0;
    virtual QUrl signInSuccessUrl() const = 0;
    virtual QUrl scoreManagerUrl() const = 0;
    virtual QUrl accessTokenUrl() const = 0;

    virtual QUrl refreshApiUrl() const = 0;
    virtual QUrl userInfoApiUrl() const = 0;
    virtual QUrl logoutApiUrl() const = 0;
    virtual QUrl scoreInfoApiUrl() const = 0;
    virtual QUrl uploadScoreApiUrl() const = 0;
    virtual QUrl uploadAudioApiUrl() const = 0;

    virtual io::path_t tokensFilePath() const = 0;
};
}

#endif // MU_CLOUD_ICLOUDCONFIGURATION_H
