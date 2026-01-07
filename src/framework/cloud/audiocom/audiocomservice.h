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

#include <QJsonObject>

#include "modularity/ioc.h"
#include "icloudconfiguration.h"
#include "network/inetworkmanagercreator.h"

#include "internal/abstractcloudservice.h"

#include "audiocom/iaudiocomservice.h"

namespace muse::cloud {
class AudioComService : public IAudioComService, public AbstractCloudService, public std::enable_shared_from_this<AudioComService>
{
    muse::GlobalInject<ICloudConfiguration> configuration;
    muse::GlobalInject<network::INetworkManagerCreator> networkManagerCreator;

public:
    explicit AudioComService(const modularity::ContextPtr& iocCtx, QObject* parent = nullptr);

    IAuthorizationServicePtr authorization() override;

    QUrl projectManagerUrl() const override;

    CloudInfo cloudInfo() const override;

    ProgressPtr uploadAudio(DevicePtr audioData, const QString& audioFormat, const QString& title, const QUrl& url,
                            Visibility visibility = Visibility::Private, bool replaceExisting = false) override;

private:
    ServerConfig serverConfig() const override;

    async::Promise<Ret> downloadAccountInfo() override;
    async::Promise<Ret> updateTokens() override;

    network::RequestHeaders headers(const QString& token = QString()) const;

    async::Promise<Ret> uploadNewAudio(DevicePtr audioData, const QString& audioFormat, const QString& title, const QUrl& url,
                                       Visibility visibility, ProgressPtr progress);

    async::Promise<Ret> replaceExistingAudio(DevicePtr audioData, const QString& audioFormat, const QString& title, const QUrl& url,
                                             Visibility visibility, ProgressPtr progress);

    async::Promise<Ret> doUploadAudio(DevicePtr audioData, const QString& audioFormat, ProgressPtr progress);
    async::Promise<Ret> doCreateAudio(const QString& title, int size, const QString& audioFormat, const QUrl& existingUrl,
                                      Visibility visibility, bool replaceExisting);

    async::Promise<Ret> doUpdateVisibility(const QUrl& url, Visibility visibility);

    void notifyServerAboutFailUpload(const QUrl& failUrl, const QString& token);
    void notifyServerAboutSuccessUpload(const QUrl& successUrl, const QString& token);

    QString m_currentUploadingAudioSlug;
    QString m_currentUploadingAudioId;
    QJsonObject m_currentUploadingAudioInfo;
};
}
