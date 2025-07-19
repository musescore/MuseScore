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
#ifndef MUSE_CLOUD_AUDIOCOMSERVICE_H
#define MUSE_CLOUD_AUDIOCOMSERVICE_H

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
    muse::Inject<ICloudConfiguration> configuration = { this };
    muse::Inject<network::INetworkManagerCreator> networkManagerCreator = { this };

public:
    explicit AudioComService(const modularity::ContextPtr& iocCtx, QObject* parent = nullptr);

    IAuthorizationServicePtr authorization() override;

    QUrl projectManagerUrl() const override;

    CloudInfo cloudInfo() const override;

    ProgressPtr uploadAudio(QIODevice& audioData, const QString& audioFormat, const QString& title, const QUrl& url,
                            Visibility visibility = Visibility::Private, bool replaceExisting = false) override;

private:
    ServerConfig serverConfig() const override;

    Ret downloadAccountInfo() override;

    bool doUpdateTokens() override;

    network::RequestHeaders headers(const QString& token = QString()) const;

    Ret doUploadAudio(network::INetworkManagerPtr uploadManager, QIODevice& audioData, const QString& audioFormat);
    Ret doCreateAudio(network::INetworkManagerPtr manager, const QString& title, int size, const QString& audioFormat,
                      const QUrl& existingUrl, Visibility visibility, bool replaceExisting);

    Ret doUpdateVisibility(network::INetworkManagerPtr manager, const QUrl& url, Visibility visibility);

    void notifyServerAboutFailUpload(const QUrl& failUrl, const QString& token);
    void notifyServerAboutSuccessUpload(const QUrl& successUrl, const QString& token);

    QString m_currentUploadingAudioSlug;
    QString m_currentUploadingAudioId;
    QJsonObject m_currentUploadingAudioInfo;
};
}

#endif // MUSE_CLOUD_AUDIOCOMSERVICE_H
