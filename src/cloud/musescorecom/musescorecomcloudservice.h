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
#ifndef MU_CLOUD_MUSESCORECOM_CLOUDSERVICE_H
#define MU_CLOUD_MUSESCORECOM_CLOUDSERVICE_H

#include "modularity/ioc.h"
#include "icloudconfiguration.h"
#include "network/inetworkmanagercreator.h"
#include "multiinstances/imultiinstancesprovider.h"

#include "internal/abstractcloudservice.h"

#include "musescorecom/imusescorecomcloudservice.h"

namespace mu::cloud {
class MuseScoreComCloudService : public AbstractCloudService, public IMuseScoreComCloudService
{
    Q_OBJECT

    INJECT(cloud, ICloudConfiguration, configuration)
    INJECT(cloud, network::INetworkManagerCreator, networkManagerCreator)

public:
    MuseScoreComCloudService(QObject* parent = nullptr);

    CloudInfo cloudInfo() const override;

    QUrl scoreManagerUrl() const override;

    framework::ProgressPtr uploadScore(QIODevice& scoreData, const QString& title, Visibility visibility = Visibility::Private,
                                       const QUrl& sourceUrl = QUrl()) override;
    framework::ProgressPtr uploadAudio(QIODevice& audioData, const QString& audioFormat, const QUrl& sourceUrl) override;

    RetVal<ScoreInfo> downloadScoreInfo(const QUrl& sourceUrl) override;

private:
    ServerConfig serverConfig() override;

    Ret downloadAccountInfo() override;

    network::RequestHeaders headers() const;

    RetVal<ScoreInfo> downloadScoreInfo(int scoreId);

    mu::RetVal<mu::ValMap> doUploadScore(network::INetworkManagerPtr uploadManager, QIODevice& scoreData, const QString& title,
                                         Visibility visibility, const QUrl& sourceUrl = QUrl());
    Ret doUploadAudio(network::INetworkManagerPtr uploadManager, QIODevice& audioData, const QString& audioFormat, const QUrl& sourceUrl);
};
}

#endif // MU_CLOUD_MUSESCORECOM_CLOUDSERVICE_H
