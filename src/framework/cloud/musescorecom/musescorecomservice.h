/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
    GlobalInject<ICloudConfiguration> configuration;
    GlobalInject<network::INetworkManagerCreator> networkManagerCreator;
    GlobalInject<IApplication> application;

public:
    explicit MuseScoreComService(const modularity::ContextPtr& iocCtx, QObject* parent = nullptr);

    IAuthorizationServicePtr authorization() override;

    CloudInfo cloudInfo() const override;

    QUrl scoreManagerUrl() const override;

    ProgressPtr uploadScore(DevicePtr scoreData, const QString& title, Visibility visibility = Visibility::Private,
                            const QUrl& sourceUrl = QUrl(), int revisionId = 0) override;
    ProgressPtr uploadAudio(DevicePtr audioData, const QString& audioFormat, const QUrl& sourceUrl) override;

    RetVal<ScoreInfo> downloadScoreInfo(const QUrl& sourceUrl) override;
    RetVal<ScoreInfo> downloadScoreInfo(int scoreId) override;

    async::Promise<ScoresList> downloadScoresList(int scoresPerBatch, int batchNumber) override;

    ProgressPtr downloadScore(int scoreId, DevicePtr scoreData, const QString& hash = QString(),
                              const QString& secret = QString()) override;

private:
    ServerConfig serverConfig() const override;

    async::Promise<Ret> downloadAccountInfo() override;
    async::Promise<Ret> updateTokens() override;

    network::RequestHeaders headers() const;

    void doDownloadScoreInfo(int scoreId, std::function<void(const RetVal<ScoreInfo>& res)> finished);

    async::Promise<Ret> doDownloadScore(int scoreId, DevicePtr scoreData, const QString& hash, const QString& secret, ProgressPtr progress);

    async::Promise<RetVal<bool> > checkScoreAlreadyUploaded(const ID& scoreId);

    async::Promise<Ret> doUploadScore(DevicePtr scoreData, const QString& title, Visibility visibility, const QUrl& sourceUrl,
                                      int revisionId, ProgressPtr progress);

    async::Promise<Ret> doUploadAudio(DevicePtr audioData, const QString& audioFormat, const QUrl& sourceUrl, ProgressPtr progress);
};
}
