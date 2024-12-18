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
#include <QLocale>

#include "global/concurrency/concurrent.h"
#include "dataformatter.h"
#include "learnerrors.h"
#include "log.h"

using namespace muse;
using namespace muse::learn;
using namespace muse::network;

static const QString DEFAULT_PLAYLIST_TAG("default");

static QString preferedPlaylistTag()
{
    return QLocale().name();
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

        {
            std::lock_guard lock(m_startedPlaylistMutex);
            m_startedPlaylist = result.val;
        }

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

        {
            std::lock_guard lock(m_advancedPlaylistMutex);
            m_advancedPlaylist = result.val;
        }
        m_advancedPlaylistChannel.send(m_advancedPlaylist);
    };

    Concurrent::run(this, &LearnService::th_requestPlaylist, configuration()->startedPlaylistUrl(), startedPlaylistCallBack);
    Concurrent::run(this, &LearnService::th_requestPlaylist, configuration()->advancedPlaylistUrl(), advancedPlaylistCallBack);
}

Playlist LearnService::startedPlaylist() const
{
    std::lock_guard lock(m_startedPlaylistMutex);
    return m_startedPlaylist;
}

async::Channel<Playlist> LearnService::startedPlaylistChanged() const
{
    return m_startedPlaylistChannel;
}

Playlist LearnService::advancedPlaylist() const
{
    std::lock_guard lock(m_advancedPlaylistMutex);
    return m_advancedPlaylist;
}

async::Channel<Playlist> LearnService::advancedPlaylistChanged() const
{
    return m_advancedPlaylistChannel;
}

void LearnService::th_requestPlaylist(const QUrl& playlistUrl, std::function<void(RetVal<Playlist>)> callBack) const
{
    TRACEFUNC;

    network::INetworkManagerPtr networkManager = networkManagerCreator()->makeNetworkManager();
    RequestHeaders headers = configuration()->headers();

    QBuffer playlistInfoData;
    Ret playlistItemsRet = networkManager->get(playlistUrl, &playlistInfoData, headers);
    if (!playlistItemsRet) {
        callBack(playlistItemsRet);
        return;
    }

    QJsonDocument playlistInfoDoc = QJsonDocument::fromJson(playlistInfoData.data());

    RetVal<Playlist> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = parsePlaylist(playlistInfoDoc);

    callBack(result);
}

Playlist LearnService::parsePlaylist(const QJsonDocument& playlistDoc) const
{
    Playlist result;

    QJsonObject obj = playlistDoc.object();
    QJsonArray items = obj.value(preferedPlaylistTag()).toArray();
    if (items.isEmpty()) {
        items = obj.value(DEFAULT_PLAYLIST_TAG).toArray();
    }

    for (const QJsonValue itemVal : items) {
        QJsonObject itemObj = itemVal.toObject();

        PlaylistItem item;
        item.title = itemObj.value("title").toString();
        item.author = itemObj.value("author").toString();
        item.url = itemObj.value("url").toString();
        item.thumbnailUrl = itemObj.value("thumbnailUrl").toString();
        item.durationSecs = itemObj.value("durationSecs").toInt();

        result << item;
    }

    return result;
}
