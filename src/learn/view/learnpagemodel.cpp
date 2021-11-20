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

#include "translation.h"

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
    learnService()->openVideo(videoId);
}

void LearnPageModel::openUrl(const QString& url) const
{
    interactive()->openUrl(QUrl(url));
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

QVariantMap LearnPageModel::classesAuthor() const
{
    QVariantMap author;
    author["name"] = qtrc("learn", "Marc Sabatella");
    author["role"] = qtrc("learn", "Instructor");
    author["position"] = qtrc("learn", "Founder, Director of Mastering MuseScore School");
    author["description"] = qtrc("learn", "My name is Marc Sabatella, and I am the founder and director of the Mastering MuseScore School. "
                                          "I am one of the developers and chief ambassadors for MuseScore, "
                                          "the world's most popular music notation software. "
                                          "I have been teaching music online since the dawn of the World Wide Web, "
                                          "and I have been teaching in person for even longer. "
                                          "From the publication of my groundbreaking Jazz Improvisation Primer back in the 1990’s, "
                                          "to my years on the faculty at major music schools, "
                                          "and culminating in this Mastering MuseScore School, "
                                          "I have dedicated most of my life to helping as many musicians as I can.");
    author["avatarUrl"] = "qrc:/qml/MuseScore/Learn/resources/marc_sabatella.JPG";
    author["organizationName"] = qtrc("learn", "Mastering MuseScore School");
    author["organizationUrl"] = "https://school.masteringmusescore.com/";

    return author;
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

    auto formatDuration = [](int durationSecs) {
        QTime duration = QDateTime::fromSecsSinceEpoch(durationSecs).time();
        int hour = duration.hour();
        int minute = duration.minute();
        int second = duration.second();

        return (hour > 0 ? QString::number(hour) + ":" : "") + QString::number(minute) + ":" + QString::number(second);
    };

    for (const PlaylistItem& item : playlist) {
        QVariantMap itemObj;
        itemObj["videoId"] = item.videoId;
        itemObj["title"] = item.title;
        itemObj["author"] = item.author;
        itemObj["thumbnailUrl"] = item.thumbnailUrl;
        itemObj["duration"] = formatDuration(item.durationSecs);

        result << itemObj;
    }

    return result;
}

Playlist LearnPageModel::filterPlaylistBySearch(const Playlist& playlist) const
{
    Playlist result;

    for (const PlaylistItem& playlistItem : playlist) {
        if (playlistItem.title.contains(m_searchText, Qt::CaseInsensitive)
            || playlistItem.author.contains(m_searchText, Qt::CaseInsensitive)) {
            result.push_back(playlistItem);
        }
    }

    return result;
}
