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

    QJsonDocument playlistInfoDoc = QJsonDocument::fromJson(playlistItemsData.data());

    std::vector<std::string> playlistItemsIds = parsePlaylistItemsIds(playlistInfoDoc);
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

    QJsonDocument videosInfoDoc = QJsonDocument::fromJson(videosInfoData.data());

    RetVal<Playlist> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = parsePlaylist(videosInfoDoc);

    finishChannel->send(result);
}

void LearnService::openUrl(const QUrl& url)
{
    Ret ret = interactive()->openUrl(url.toString().toStdString());
    if (!ret) {
        LOGE() << ret.toString();
    }
}

std::vector<std::string> LearnService::parsePlaylistItemsIds(const QJsonDocument& playlistDoc) const
{
    std::vector<std::string> result;

    QJsonObject obj = playlistDoc.object();
    QJsonArray items = obj.value("items").toArray();

    for (const QJsonValue& itemVal : items) {
        QJsonObject itemObj = itemVal.toObject();
        QJsonObject snippetObj = itemObj.value("snippet").toObject();
        QJsonObject resourceIdObj = snippetObj.value("resourceId").toObject();
        std::string videoId = resourceIdObj.value("videoId").toString().toStdString();

        result.push_back(videoId);
    }

    return result;
}

Playlist LearnService::parsePlaylist(const QJsonDocument& playlistDoc) const
{
    Playlist result;

    QJsonObject obj = playlistDoc.object();
    QJsonArray items = obj.value("items").toArray();

    for (const QJsonValue& itemVal : items) {
        QJsonObject itemObj = itemVal.toObject();
        QJsonObject snippetObj = itemObj.value("snippet").toObject();

        PlaylistItem item;
        item.videoId = itemObj.value("id").toString().toStdString();

        item.title = snippetObj.value("title").toString().toStdString();
        item.author = snippetObj.value("channelTitle").toString().toStdString();

        QJsonObject thumbnailsObj = snippetObj.value("thumbnails").toObject();
        QJsonObject thumbnailsMediumObj = thumbnailsObj.value("medium").toObject();
        item.thumbnailUrl = thumbnailsMediumObj.value("url").toString().toStdString();

        QJsonObject contentDetails = itemObj.value("contentDetails").toObject();
//        QString duration = contentDetails["duration"].toString();
//        QDateTime time = QDateTime::fromString(duration, "'P'D'T'hh'H'mm'M'ss'S'");
//        item.durationSec = time.toSecsSinceEpoch();

        result.push_back(item);
    }

    return result;
}
