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

#include "learnservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>
#include <QLocale>

#include "dataformatter.h"
#include "learnerrors.h"
#include "log.h"

using namespace muse;
using namespace muse::learn;
using namespace muse::network;

static Playlist parsePlaylist(const QJsonDocument& playlistDoc)
{
    Playlist result;

    QJsonObject obj = playlistDoc.object();
    QString preferredPlaylistTag = QLocale().name();
    QJsonArray items = obj.value(preferredPlaylistTag).toArray();
    if (items.isEmpty()) {
        items = obj.value("default").toArray();
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

void LearnService::refreshPlaylists()
{
    auto startedPlaylistCallBack = [this](const RetVal<Playlist>& result) {
        if (!result.ret) {
            LOGE() << "Unable to get started playlist: " << result.ret.toString();
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
            LOGE() << "Unable to get advanced playlist: " << result.ret.toString();
            return;
        }

        if (m_advancedPlaylist == result.val) {
            return;
        }

        m_advancedPlaylist = result.val;
        m_advancedPlaylistChannel.send(m_advancedPlaylist);
    };

    downloadPlaylist(configuration()->startedPlaylistUrl(), startedPlaylistCallBack);
    downloadPlaylist(configuration()->advancedPlaylistUrl(), advancedPlaylistCallBack);
}

Playlist LearnService::startedPlaylist() const
{
    return m_startedPlaylist;
}

async::Channel<Playlist> LearnService::startedPlaylistChanged() const
{
    return m_startedPlaylistChannel;
}

Playlist LearnService::advancedPlaylist() const
{
    return m_advancedPlaylist;
}

async::Channel<Playlist> LearnService::advancedPlaylistChanged() const
{
    return m_advancedPlaylistChannel;
}

void LearnService::downloadPlaylist(const QUrl& playlistUrl, std::function<void(RetVal<Playlist>)> callBack)
{
    TRACEFUNC;

    if (!m_networkManager) {
        m_networkManager = networkManagerCreator()->makeNetworkManager();
    }

    RequestHeaders headers = configuration()->headers();
    auto playlistInfoData = std::make_shared<QBuffer>();

    RetVal<Progress> progress = m_networkManager->get(playlistUrl, playlistInfoData, headers);
    if (!progress.ret) {
        callBack(progress.ret);
        return;
    }

    progress.val.finished().onReceive(this, [callBack, playlistInfoData](const muse::ProgressResult& res) {
        if (!res.ret) {
            callBack(res.ret);
            return;
        }

        QJsonDocument playlistInfoDoc = QJsonDocument::fromJson(playlistInfoData->data());

        RetVal<Playlist> result;
        result.ret = make_ret(Ret::Code::Ok);
        result.val = parsePlaylist(playlistInfoDoc);

        callBack(result);
    });
}
