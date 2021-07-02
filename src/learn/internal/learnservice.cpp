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
#include <QtConcurrent>

#include "log.h"
#include "learnerrors.h"

using namespace mu::learn;
using namespace mu::network;

void LearnService::init()
{
    refreshPlaylists();
}

Playlist LearnService::startedPlaylist() const
{
    return m_startedPlaylist;
}

Playlist LearnService::advancedPlaylist() const
{
    return m_advancedPlaylist;
}

void LearnService::openVideo(const std::string& videoId) const
{
    QUrl videoUrl = configuration()->videoOpenUrl(videoId);
    interactive()->openUrl(videoUrl.toString().toStdString());
}

void LearnService::refreshPlaylists()
{
    async::Channel<RetVal<Playlist> >* startedPlaylistFinishChannel = new async::Channel<RetVal<Playlist> >();
    startedPlaylistFinishChannel->onReceive(this, [this](const RetVal<Playlist>& result) {
        if (!result.ret) {
            LOGW() << result.ret.toString();
            return;
        }

        m_startedPlaylist = result.val;
    });

    async::Channel<RetVal<Playlist> >* advancedPlaylistFinishChannel = new async::Channel<RetVal<Playlist> >();
    advancedPlaylistFinishChannel->onReceive(this, [this](const RetVal<Playlist>& result) {
        if (!result.ret) {
            LOGW() << result.ret.toString();
            return;
        }

        m_advancedPlaylist = result.val;
    });

    QtConcurrent::run(this, &LearnService::th_requestPlaylist, configuration()->startedPlaylistUrl(), startedPlaylistFinishChannel);
    QtConcurrent::run(this, &LearnService::th_requestPlaylist, configuration()->advancedPlaylistUrl(), advancedPlaylistFinishChannel);
}

void LearnService::th_requestPlaylist(const QUrl& playlistUrl, async::Channel<RetVal<Playlist> >* finishChannel) const
{
    TRACEFUNC;

    network::INetworkManagerPtr networkManager = networkManagerCreator()->makeNetworkManager();
    RequestHeaders headers = configuration()->headers();

    QBuffer playlistItemsData;
    Ret playlistItemsRet = networkManager->get(playlistUrl, &playlistItemsData, headers);
    if (!playlistItemsRet) {
        finishChannel->send(playlistItemsRet);
        return;
    }

    QVariantMap playlistInfo = QJsonDocument::fromJson(playlistItemsData.data()).toVariant().toMap();

    std::vector<std::string> playlistItemsIds = parsePlaylistItemsIds(playlistInfo);
    if (playlistItemsIds.empty()) {
        finishChannel->send(make_ret(Err::PlaylistIsEmpty));
        return;
    }

    QUrl playlistVideosInfoUrl = configuration()->videosInfoUrl(playlistItemsIds);
    QBuffer videosInfoData;
    Ret videosRet = networkManager->get(playlistVideosInfoUrl, &videosInfoData, headers);
    if (!videosRet) {
        finishChannel->send(videosRet);
        return;
    }

    QVariantMap videosInfo = QJsonDocument::fromJson(videosInfoData.data()).toVariant().toMap();

    RetVal<Playlist> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = parsePlaylist(videosInfo);

    finishChannel->send(result);
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
