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
#ifndef MU_CLOUD_MUSESCORECOMSERVICE_H
#define MU_CLOUD_MUSESCORECOMSERVICE_H

#include <memory>

#include "modularity/ioc.h"
#include "icloudconfiguration.h"
#include "network/inetworkmanagercreator.h"

#include "internal/abstractcloudservice.h"

#include "musescorecom/imusescorecomservice.h"

namespace mu::cloud {
class MuseScoreComService : public IMuseScoreComService, public AbstractCloudService,
    public std::enable_shared_from_this<MuseScoreComService>
{
    INJECT(ICloudConfiguration, configuration)
    INJECT(network::INetworkManagerCreator, networkManagerCreator)

public:
    explicit MuseScoreComService(QObject* parent = nullptr);

    IAuthorizationServicePtr authorization() override;

    CloudInfo cloudInfo() const override;

    QUrl scoreManagerUrl() const override;

    framework::ProgressPtr uploadScore(QIODevice& scoreData, const QString& title, Visibility visibility = Visibility::Private,
                                       const QUrl& sourceUrl = QUrl(), int revisionId = 0) override;
    framework::ProgressPtr uploadAudio(QIODevice& audioData, const QString& audioFormat, const QUrl& sourceUrl) override;

    RetVal<ScoreInfo> downloadScoreInfo(const QUrl& sourceUrl) override;

    async::Promise<ScoresList> downloadScoresList(int scoresPerBatch, int batchNumber) override;

    framework::ProgressPtr downloadScore(int scoreId, QIODevice& scoreData) override;

private:
    ServerConfig serverConfig() const override;

    Ret downloadAccountInfo() override;

    bool doUpdateTokens() override;

    network::RequestHeaders headers() const;

    RetVal<ScoreInfo> downloadScoreInfo(int scoreId);

    Ret doDownloadScore(network::INetworkManagerPtr downloadManager, int scoreId, QIODevice& scoreData);

    mu::RetVal<mu::ValMap> doUploadScore(network::INetworkManagerPtr uploadManager, QIODevice& scoreData, const QString& title,
                                         Visibility visibility, const QUrl& sourceUrl = QUrl(), int revisionId = 0);
    Ret doUploadAudio(network::INetworkManagerPtr uploadManager, QIODevice& audioData, const QString& audioFormat, const QUrl& sourceUrl);
};
}

#endif // MU_CLOUD_MUSESCORECOMSERVICE_H
