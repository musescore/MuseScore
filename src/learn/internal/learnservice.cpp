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
    QUrl videoUrl = configuration()->videoOpenUrl(videoId);
    interactive()->openUrl(videoUrl.toString().toStdString());
}

Playlist LearnService::requestPlaylist(const QUrl& playlistUrl) const
{
    RequestHeaders headers = configuration()->headers();

    QBuffer playlistItemsData;
    Ret playlistItems = m_networkManager->get(playlistUrl, &playlistItemsData, headers);
    if (!playlistItems) {
        LOGE() << playlistItems.toString();
        return {};
    }

    QVariantMap playlistInfo = QJsonDocument::fromJson(playlistItemsData.data()).toVariant().toMap();

    std::vector<std::string> playlistItemsIds = parsePlaylistItemsIds(playlistInfo);
    if (playlistItemsIds.empty()) {
        LOGW() << "Empty list of playlist items";
        return {};
    }

    QUrl playlistVideosInfoUrl = configuration()->videosInfoUrl(playlistItemsIds);
    QBuffer videosInfoData;
    Ret videos = m_networkManager->get(playlistVideosInfoUrl, &videosInfoData, headers);
    if (!videos) {
        LOGE() << videos.toString();
        return {};
    }

    QVariantMap videosInfo = QJsonDocument::fromJson(videosInfoData.data()).toVariant().toMap();
    return parsePlaylist(videosInfo);
}

void LearnService::openUrl(const QUrl& url)
{
    Ret ret = interactive()->openUrl(url.toString().toStdString());
    if (!ret) {
        LOGE() << ret.toString();
    }
}

std::vector<std::string> LearnService::parsePlaylistItemsIds(const QVariantMap& playlistMap) const
{
    std::vector<std::string> result;

    QVariantList items = playlistMap["items"].toList();
    for (const QVariant& itemVar : items) {
        QVariantMap snippet = itemVar.toMap()["snippet"].toMap();
        QVariantMap resourceId = snippet["resourceId"].toMap();
        std::string videoId = resourceId["videoId"].toString().toStdString();

        result.push_back(videoId);
    }

    return result;
}

Playlist LearnService::parsePlaylist(const QVariantMap& playlistMap) const
{
    Playlist result;

    QVariantList items = playlistMap["items"].toList();
    for (const QVariant& itemVar : items) {
        QVariantMap itemMap = itemVar.toMap();

        PlaylistItem item;
        item.videoId = itemMap["id"].toString().toStdString();

        QVariantMap snippet = itemMap["snippet"].toMap();
        item.title = snippet["title"].toString().toStdString();
        item.author = snippet["channelTitle"].toString().toStdString();

        QVariantMap thumbnails = snippet["thumbnails"].toMap();
        QVariantMap thumbnailsMedium = thumbnails["medium"].toMap();
        item.thumbnailUrl = thumbnailsMedium["url"].toString().toStdString();

        QVariantMap contentDetails = itemMap["contentDetails"].toMap();
//        QString duration = contentDetails["duration"].toString();
//        QDateTime time = QDateTime::fromString(duration, "'P'D'T'hh'H'mm'M'ss'S'");
//        item.durationSec = time.toSecsSinceEpoch();

        result.push_back(item);
    }

    return result;
}
