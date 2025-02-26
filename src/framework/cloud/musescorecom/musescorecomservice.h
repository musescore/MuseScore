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
#ifndef MUSE_CLOUD_MUSESCORECOMSERVICE_H
#define MUSE_CLOUD_MUSESCORECOMSERVICE_H

#include <memory>

#include "modularity/ioc.h"
#include "icloudconfiguration.h"
#include "network/inetworkmanagercreator.h"
#include "global/iapplication.h"

#include "internal/abstractcloudservice.h"

#include "musescorecom/imusescorecomservice.h"

namespace muse::cloud {
class MuseScoreComService : public IMuseScoreComService, public AbstractCloudService,
    public std::enable_shared_from_this<MuseScoreComService>
{
    Inject<ICloudConfiguration> configuration = { this };
    Inject<network::INetworkManagerCreator> networkManagerCreator = { this };
    Inject<IApplication> application = { this };

public:
    explicit MuseScoreComService(const modularity::ContextPtr& iocCtx, QObject* parent = nullptr);

    IAuthorizationServicePtr authorization() override;

    CloudInfo cloudInfo() const override;

    QUrl scoreManagerUrl() const override;

    ProgressPtr uploadScore(QIODevice& scoreData, const QString& title, Visibility visibility = Visibility::Private,
                            const QUrl& sourceUrl = QUrl(), int revisionId = 0) override;
    ProgressPtr uploadAudio(QIODevice& audioData, const QString& audioFormat, const QUrl& sourceUrl) override;

    RetVal<ScoreInfo> downloadScoreInfo(const QUrl& sourceUrl) override;
    RetVal<ScoreInfo> downloadScoreInfo(int scoreId) override;

    async::Promise<ScoresList> downloadScoresList(int scoresPerBatch, int batchNumber) override;

    ProgressPtr downloadScore(int scoreId, QIODevice& scoreData, const QString& hash = QString(),
                              const QString& secret = QString()) override;

private:
    ServerConfig serverConfig() const override;

    Ret downloadAccountInfo() override;

    bool doUpdateTokens() override;

    network::RequestHeaders headers() const;

    Ret doDownloadScore(network::INetworkManagerPtr downloadManager, int scoreId, QIODevice& scoreData,
                        const QString& hash = QString(), const QString& secret = QString());

    RetVal<ValMap> doUploadScore(network::INetworkManagerPtr uploadManager, QIODevice& scoreData, const QString& title,
                                 Visibility visibility, const QUrl& sourceUrl = QUrl(), int revisionId = 0);

    Ret doUploadAudio(network::INetworkManagerPtr uploadManager, QIODevice& audioData, const QString& audioFormat, const QUrl& sourceUrl);
};
}

#endif // MUSE_CLOUD_MUSESCORECOMSERVICE_H
