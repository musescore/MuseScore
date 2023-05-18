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
#ifndef MU_CLOUD_CLOUDCONFIGURATION_H
#define MU_CLOUD_CLOUDCONFIGURATION_H

#include "icloudconfiguration.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

namespace mu::cloud {
class CloudConfiguration : public ICloudConfiguration
{
    INJECT(framework::IGlobalConfiguration, globalConfiguration)

public:
    void init();

    network::RequestHeaders headers() const override;
    QByteArray uploadingLicense() const override;

    QUrl cloudUrl() const override;
    QUrl authorizationUrl() const override;
    QUrl signUpUrl() const override;
    QUrl signInSuccessUrl() const override;
    QUrl scoreManagerUrl() const override;
    QUrl accessTokenUrl() const override;

    QUrl refreshApiUrl() const override;
    QUrl userInfoApiUrl() const override;
    QUrl logoutApiUrl() const override;
    QUrl scoreInfoApiUrl() const override;
    QUrl uploadScoreApiUrl() const override;
    QUrl uploadAudioApiUrl() const override;

    io::path_t tokensFilePath() const override;

private:
    QString apiRootUrl() const;
};
}

#endif // MU_CLOUD_CLOUDCONFIGURATION_H
