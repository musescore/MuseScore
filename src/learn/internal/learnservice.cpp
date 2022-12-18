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
#include <QLocale>

#include "dataformatter.h"
#include "learnerrors.h"
#include "log.h"

using namespace mu::learn;
using namespace mu::network;

static const Playlist s_bilibiliPlaylist = {
    { "BV1fv4y197Vc", "MuseScore 4视频公告", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/announcing.jpg", 241 },
    { "BV1d14y1K7zf", "MuseScore 4与3相比有哪些变化", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/differences_v3.jpg", 341 },
    { "BV18V4y1P78J", "怎样安装交响乐插件Muse Sounds", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/installing.jpg", 63 },
    { "BV1Ye4y1T7t3", "雕排改进以及会如何影响旧乐谱", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/engraving.jpg", 144 },
    { "BV1Ae4y1M7gU?p=1", "视频导引：新建乐谱", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/setting_up.jpg", 92 },
    { "BV1Ae4y1M7gU?p=2", "视频导引：乐谱写作基础", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/basics_writing.jpg", 262 },
    { "BV1Ae4y1M7gU?p=3", "视频导引：力度记号、奏法记号、速度记号与文字", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/dynamics.jpg", 200 },
    { "BV1Ae4y1M7gU?p=4", "视频导引：吉他与打击乐", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/guitar.jpg", 167 },
    { "BV1Ae4y1M7gU?p=5", "视频导引：布局与分谱", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/layouts.jpg", 297 },
    { "BV1Ae4y1M7gU?p=6", "视频导引：文字、歌词与和弦记号", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/text.jpg", 165 },
    { "BV1Ae4y1M7gU?p=7", "视频导引：反复与跳跃", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/repeats.jpg", 161 },
    { "BV1Ae4y1M7gU?p=8", "视频导引：MIDI键盘的使用", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/midi.jpg", 148 },
    { "BV1Ae4y1M7gU?p=9", "视频导引：在云端保存与发布", "Musescore",
      "https://s3.amazonaws.com/s.musescore.org/video_thumbs/publishing.jpg", 131 }
};

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
    return QLocale().name() == "zh_CN" ? s_bilibiliPlaylist : m_startedPlaylist;
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
    openUrl(configuration()->videoOpenUrl(videoId, QLocale().name() == "zh_CN"));
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
