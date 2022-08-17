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

#include "dataformatter.h"
#include "learnerrors.h"
#include "log.h"

using namespace mu::learn;
using namespace mu::network;

static int videoDurationSecs(const QString& durationInIsoFormat)
{
    // NOTE Available ISO8601 duration format: P#Y#M#DT#H#M#S

    QRegularExpression regexp(QString("("
                                      "P"
                                      "((?<years>[0-9]+)Y)?"
                                      "((?<months>[0-9]+)M)?"
                                      "((?<days>[0-9]+)D)?"
                                      "T"
                                      "((?<hours>[0-9]+)H)?"
                                      "((?<minutes>[0-9]+)M)?"
                                      "((?<seconds>[0-9]+)S)?"
                                      ")"));

    QRegularExpressionMatch match = regexp.match(durationInIsoFormat);

    if (!match.hasMatch()) {
        return 0;
    }

    int hours = match.captured("hours").toInt();
    int minutes = match.captured("minutes").toInt();
    int seconds = match.captured("seconds").toInt();

    return seconds + minutes * 60 + hours * 60 * 60;
}

void LearnService::refreshPlaylists()
{
    auto startedPlaylistCallBack = [this](const RetVal<Playlist>& result) {
        if (!result.ret) {
            LOGW() << result.ret.toString();
            return;
        }

        if (m_startedPlaylist == result.val) {
            return;
        }

        m_startedPlaylist = result.val;
        m_startedPlaylistChannel.send(m_startedPlaylist);
    };

    auto advancedPlaylistCallBack = [this](const RetVal<Playlist>& result) {
        if (!result.ret) {
            LOGW() << result.ret.toString();
            return;
        }

        if (m_advancedPlaylist == result.val) {
            return;
        }

        m_advancedPlaylist = result.val;
        m_advancedPlaylistChannel.send(m_advancedPlaylist);
    };

    QtConcurrent::run(this, &LearnService::th_requestPlaylist, configuration()->startedPlaylistUrl(), startedPlaylistCallBack);
    QtConcurrent::run(this, &LearnService::th_requestPlaylist, configuration()->advancedPlaylistUrl(), advancedPlaylistCallBack);
}

Playlist LearnService::startedPlaylist() const
{
    return m_startedPlaylist;
}

mu::async::Channel<Playlist> LearnService::startedPlaylistChanged() const
{
    return m_startedPlaylistChannel;
}

Playlist LearnService::advancedPlaylist() const
{
    return m_advancedPlaylist;
}

mu::async::Channel<Playlist> LearnService::advancedPlaylistChanged() const
{
    return m_advancedPlaylistChannel;
}

void LearnService::openVideo(const QString& videoId) const
{
    openUrl(configuration()->videoOpenUrl(videoId));
}

void LearnService::th_requestPlaylist(const QUrl& playlistUrl, std::function<void(RetVal<Playlist>)> callBack) const
{
    TRACEFUNC;

    network::INetworkManagerPtr networkManager = networkManagerCreator()->makeNetworkManager();
    RequestHeaders headers = configuration()->headers();

    QBuffer playlistItemsData;
    Ret playlistItemsRet = networkManager->get(playlistUrl, &playlistItemsData, headers);
    if (!playlistItemsRet) {
        callBack(playlistItemsRet);
        return;
    }

    QJsonDocument playlistInfoDoc = QJsonDocument::fromJson(playlistItemsData.data());

    QStringList playlistItemsIds = parsePlaylistItemsIds(playlistInfoDoc);
    if (playlistItemsIds.isEmpty()) {
        callBack(make_ret(Err::PlaylistIsEmpty));
        return;
    }

    QUrl playlistVideosInfoUrl = configuration()->videosInfoUrl(playlistItemsIds);
    QBuffer videosInfoData;
    Ret videosRet = networkManager->get(playlistVideosInfoUrl, &videosInfoData, headers);
    if (!videosRet) {
        callBack(videosRet);
        return;
    }

    QJsonDocument videosInfoDoc = QJsonDocument::fromJson(videosInfoData.data());

    RetVal<Playlist> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = parsePlaylist(videosInfoDoc);

    callBack(result);
}

void LearnService::openUrl(const QUrl& url) const
{
    Ret ret = interactive()->openUrl(url);
    if (!ret) {
        LOGE() << ret.toString();
    }
}

QStringList LearnService::parsePlaylistItemsIds(const QJsonDocument& playlistDoc) const
{
    QStringList result;

    QJsonObject obj = playlistDoc.object();
    QJsonArray items = obj.value("items").toArray();

    for (const QJsonValue& itemVal : items) {
        QJsonObject itemObj = itemVal.toObject();
        QJsonObject snippetObj = itemObj.value("snippet").toObject();
        QJsonObject resourceIdObj = snippetObj.value("resourceId").toObject();
        QString videoId = resourceIdObj.value("videoId").toString();

        result << videoId;
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
        item.videoId = itemObj.value("id").toString();

        item.title = snippetObj.value("title").toString();
        item.author = snippetObj.value("channelTitle").toString();

        QJsonObject thumbnailsObj = snippetObj.value("thumbnails").toObject();
        QJsonObject thumbnailsMediumObj = thumbnailsObj.value("medium").toObject();
        item.thumbnailUrl = thumbnailsMediumObj.value("url").toString();

        QJsonObject contentDetails = itemObj.value("contentDetails").toObject();
        QString durationInIsoFormat = contentDetails["duration"].toString();
        item.durationSecs = videoDurationSecs(durationInIsoFormat);

        result << item;
    }

    return result;
}
