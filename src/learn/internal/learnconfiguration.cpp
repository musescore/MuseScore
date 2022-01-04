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
#include "learnconfiguration.h"

#include <QString>

#include "config.h"

using namespace mu::learn;
using namespace mu::network;

static const QString API_KEY(YOUTUBE_API_KEY);
static const QString GET_STARTED_PLAYLIST_ID("PLTYuWi2LmaPEZX1IDtL6Lx1Uyq7WGdGCL");
static const QString ADVANCED_PLAYLIST_ID("PL24C760637A625BB6");
static const int MAX_NUMBER_OF_RESULT_ITEMS(100);

RequestHeaders LearnConfiguration::headers() const
{
    RequestHeaders headers;
    headers.rawHeaders["Accept"] = "application/json";
    return headers;
}

QUrl mu::learn::LearnConfiguration::startedPlaylistUrl() const
{
    return QUrl(apiRootUrl() + "/playlistItems?" + playlistItemsParams(GET_STARTED_PLAYLIST_ID));
}

QUrl LearnConfiguration::advancedPlaylistUrl() const
{
    return QUrl(apiRootUrl() + "/playlistItems?" + playlistItemsParams(ADVANCED_PLAYLIST_ID));
}

QUrl LearnConfiguration::videosInfoUrl(const QStringList& videosIds) const
{
    return QUrl(apiRootUrl() + "/videos?" + videosParams(videosIds));
}

QUrl LearnConfiguration::videoOpenUrl(const QString& videoId) const
{
    return QUrl("https://www.youtube.com/watch?v=" + videoId);
}

QString LearnConfiguration::apiRootUrl() const
{
    return "https://youtube.googleapis.com/youtube/v3";
}

QString LearnConfiguration::playlistItemsParams(const QString& playlistId) const
{
    QStringList params {
        "part=snippet",
        "playlistId=" + playlistId,
        "key=" + API_KEY,
        "maxResults=" + QString::number(MAX_NUMBER_OF_RESULT_ITEMS)
    };

    return params.join('&');
}

QString LearnConfiguration::videosParams(const QStringList& videosIds) const
{
    QStringList params {
        "part=snippet,contentDetails",
        "key=" + API_KEY,
        "maxResults=" + QString::number(MAX_NUMBER_OF_RESULT_ITEMS)
    };

    params.reserve(params.size() + videosIds.size());

    for (const QString& videoId : videosIds) {
        params << "id=" + videoId;
    }

    return params.join('&');
}
