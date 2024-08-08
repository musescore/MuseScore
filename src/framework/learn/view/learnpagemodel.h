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
#ifndef MUSE_LEARN_LEARNPAGEMODEL_H
#define MUSE_LEARN_LEARNPAGEMODEL_H

#include <QObject>
#include <QVariant>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ilearnservice.h"
#include "ilearnconfiguration.h"

namespace muse::learn {
class LearnPageModel : public QObject, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QVariantList startedPlaylist READ startedPlaylist NOTIFY startedPlaylistChanged)
    Q_PROPERTY(QVariantList advancedPlaylist READ advancedPlaylist NOTIFY advancedPlaylistChanged)

    Inject<ILearnService> learnService = { this };
    Inject<ILearnConfiguration> learnConfiguration = { this };

public:
    explicit LearnPageModel(QObject* parent = nullptr);

    QVariantList startedPlaylist() const;
    QVariantList advancedPlaylist() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void setSearchText(const QString& text);
    Q_INVOKABLE QVariantMap classesAuthor() const;
    Q_INVOKABLE bool classesEnabled();

private slots:
    void setStartedPlaylist(Playlist startedPlaylist);
    void setAdvancedPlaylist(Playlist advancedPlaylist);

signals:
    void startedPlaylistChanged();
    void advancedPlaylistChanged();

private:
    QVariantList playlistToVariantList(const Playlist& playlist) const;

    Playlist filterPlaylistBySearch(const Playlist& playlist) const;

    Playlist m_startedPlaylist;
    Playlist m_advancedPlaylist;

    QString m_searchText;
};
}

#endif // MUSE_LEARN_LEARNPAGEMODEL_H
