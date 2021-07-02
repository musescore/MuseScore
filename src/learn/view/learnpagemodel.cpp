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
#include "learnpagemodel.h"

#include <QVariant>
#include <QDateTime>

using namespace mu::learn;

LearnPageModel::LearnPageModel(QObject* parent)
    : QObject(parent)
{
}

QVariantList LearnPageModel::startedPlaylist() const
{
    Playlist filteredPlaylist = filterPlaylistBySearch(m_startedPlaylist);
    return playlistToVariantList(filteredPlaylist);
}

QVariantList LearnPageModel::advancedPlaylist() const
{
    Playlist filteredPlaylist = filterPlaylistBySearch(m_advancedPlaylist);
    return playlistToVariantList(filteredPlaylist);
}

void LearnPageModel::load()
{
    setStartedPlaylist(learnService()->startedPlaylist());
    learnService()->startedPlaylistChanged().onReceive(this, [this](const Playlist& playlist) {
        setStartedPlaylist(playlist);
    });

    setAdvancedPlaylist(learnService()->advancedPlaylist());
    learnService()->advancedPlaylistChanged().onReceive(this, [this](const Playlist& playlist) {
        setAdvancedPlaylist(playlist);
    });
}

void LearnPageModel::openVideo(const QString& videoId) const
{
    learnService()->openVideo(videoId.toStdString());
}

void LearnPageModel::setSearchText(const QString& text)
{
    if (m_searchText == text) {
        return;
    }

    m_searchText = text;

    emit startedPlaylistChanged();
    emit advancedPlaylistChanged();
}

void LearnPageModel::setStartedPlaylist(Playlist startedPlaylist)
{
    if (m_startedPlaylist == startedPlaylist) {
        return;
    }

    m_startedPlaylist = startedPlaylist;
    emit startedPlaylistChanged();
}

void LearnPageModel::setAdvancedPlaylist(Playlist advancedPlaylist)
{
    if (m_advancedPlaylist == advancedPlaylist) {
        return;
    }

    m_advancedPlaylist = advancedPlaylist;
    emit advancedPlaylistChanged();
}

QVariantList LearnPageModel::playlistToVariantList(const Playlist& playlist) const
{
    QVariantList result;

    for (const PlaylistItem& item : playlist) {
        QVariantMap itemObj;
        itemObj["videoId"] = QString::fromStdString(item.videoId);
        itemObj["title"] = QString::fromStdString(item.title);
        itemObj["author"] =QString::fromStdString(item.author);
        itemObj["thumbnailUrl"] = QString::fromStdString(item.thumbnailUrl);
        itemObj["duration"] = QDateTime::fromSecsSinceEpoch(item.durationSec).toString("hh:mm::ss");

        result << itemObj;
    }

    return result;
}

Playlist LearnPageModel::filterPlaylistBySearch(const Playlist& playlist) const
{
    Playlist result;

    for (const PlaylistItem& playlistItem : playlist) {
        QString title = QString::fromStdString(playlistItem.title);
        QString author = QString::fromStdString(playlistItem.author);
        if (title.contains(m_searchText, Qt::CaseInsensitive)
            || author.contains(m_searchText, Qt::CaseInsensitive)) {
            result.push_back(playlistItem);
        }
    }

    return result;
}
