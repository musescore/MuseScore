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

using namespace muse::learn;

LearnPageModel::LearnPageModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
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
    learnService()->refreshPlaylists();

    setStartedPlaylist(learnService()->startedPlaylist());
    learnService()->startedPlaylistChanged().onReceive(this, [this](const Playlist& playlist) {
        setStartedPlaylist(playlist);
    });

    setAdvancedPlaylist(learnService()->advancedPlaylist());
    learnService()->advancedPlaylistChanged().onReceive(this, [this](const Playlist& playlist) {
        setAdvancedPlaylist(playlist);
    });
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
    author["name"] = muse::qtrc("learn", "Marc Sabatella");
    author["role"] = muse::qtrc("learn", "Instructor");

    // Rename to "MuseScore Studio" in this description?
    author["position"] = muse::qtrc("learn", "Creator, Mastering MuseScore");
    author["description"] = muse::qtrc("learn", "Welcome to Mastering MuseScore – the most comprehensive resource "
                                                "for learning the world’s most popular music notation software! "
                                                "My name is Marc Sabatella, and I have been helping develop, support, "
                                                "and promote MuseScore since its initial release over ten years ago.\n\n"
                                                "Whether you are just getting started with music notation software, "
                                                "or are a power user eager to explore advanced engraving and playback techniques, "
                                                "my flagship online course Mastering MuseScore "
                                                "covers everything you need to know to get the most out of MuseScore.\n\n"
                                                "In addition, Mastering MuseScore features a supportive community of musicians, "
                                                "with discussion spaces, live streams, "
                                                "and other related courses and services to help you create your best music. "
                                                "Take advantage of this opportunity to learn MuseScore from one of its most recognized experts!\n\n"
                                                "(Note: Mastering MuseScore is available in English only)");
    author["avatarUrl"] = "qrc:/qml/Muse/Learn/resources/marc_sabatella.JPG";
    author["organizationName"] = muse::qtrc("learn", "Mastering MuseScore");
    author["organizationUrl"] = "https://www.masteringmusescore.com/musescore4";

    return author;
}

bool LearnPageModel::classesEnabled()
{
    return learnConfiguration()->classesEnabled();
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

    // h:mm:ss for anything over an hour
    //    m:ss for anything under an hour
    //    0:ss for anything under a minute
    auto formatDuration = [](int durationSecs) {
        int seconds = durationSecs;
        int minutes = seconds / 60;
        seconds -= minutes * 60;
        int hours = minutes / 60;
        minutes -= hours * 60;

        return ((hours > 0)
                ? (QString::number(hours) + ":" + QString::number(minutes).rightJustified(2, '0'))
                : QString::number(minutes)
                ) + ":"
               + QString::number(seconds).rightJustified(2, '0');
    };

    for (const PlaylistItem& item : playlist) {
        QVariantMap itemObj;
        itemObj["title"] = item.title;
        itemObj["author"] = item.author;
        itemObj["url"] = item.url;
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
