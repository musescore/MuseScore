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

#include "learnservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>

#include "log.h"

using namespace mu::learn;
using namespace mu::network;

LearnService::LearnService(QObject* parent)
    : QObject(parent)
{
}

void LearnService::init()
{
    TRACEFUNC;

    m_networkManager = networkManagerCreator()->makeNetworkManager();
}

Playlist LearnService::startedPlaylist() const
{
    TRACEFUNC;

    QUrl playlistUrl = configuration()->startedPlaylistUrl();
    if (playlistUrl.isEmpty()) {
        return {};
    }

    return requestPlaylist(playlistUrl);
}

Playlist LearnService::advancedPlaylist() const
{
    TRACEFUNC;

    QUrl playlistUrl = configuration()->advancedPlaylistUrl();
    if (playlistUrl.isEmpty()) {
        return {};
    }

    return requestPlaylist(playlistUrl);
}

void LearnService::openVideo(const std::string& videoId) const
{
    QUrl videoUrl = configuration()->videoUrl(videoId);
    interactive()->openUrl(videoUrl.toString().toStdString());
}

Playlist LearnService::requestPlaylist(const QUrl& playlistUrl) const
{
    QBuffer receivedData;
    RequestHeaders headers = configuration()->headers();

    Ret getPlaylist = m_networkManager->get(playlistUrl, &receivedData, headers);
    if (!getPlaylist) {
        LOGE() << getPlaylist.toString();
        return {};
    }

    QVariantMap playlistInfo = QJsonDocument::fromJson(receivedData.data()).toVariant().toMap();
    return parsePlaylist(playlistInfo);
}

void LearnService::openUrl(const QUrl& url)
{
    Ret ret = interactive()->openUrl(url.toString().toStdString());
    if (!ret) {
        LOGE() << ret.toString();
    }
}

Playlist LearnService::parsePlaylist(const QVariantMap& playlistMap) const
{
    Playlist result;

    QVariantList items = playlistMap["items"].toList();
    for (const QVariant& itemVar : items) {
        QVariantMap snippet = itemVar.toMap()["snippet"].toMap();

        PlaylistItem item;

        QVariantMap resourceId = snippet["resourceId"].toMap();
        item.videoId = resourceId["videoId"].toString().toStdString();

        item.title = snippet["title"].toString().toStdString();
        item.author = snippet["videoOwnerChannelTitle"].toString().toStdString();

        QVariantMap thumbnails = snippet["thumbnails"].toMap();
        QVariantMap thumbnailsMedium = thumbnails["medium"].toMap();
        item.thumbnailUrl = thumbnailsMedium["url"].toString().toStdString();
        // todo duration

        result.push_back(item);
    }

    return result;
}
