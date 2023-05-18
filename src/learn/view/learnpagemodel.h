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
#ifndef MU_LEARN_LEARNPAGEMODEL_H
#define MU_LEARN_LEARNPAGEMODEL_H

#include <QObject>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ilearnservice.h"

namespace mu::learn {
class LearnPageModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QVariantList startedPlaylist READ startedPlaylist NOTIFY startedPlaylistChanged)
    Q_PROPERTY(QVariantList advancedPlaylist READ advancedPlaylist NOTIFY advancedPlaylistChanged)

    INJECT(ILearnService, learnService)

public:
    explicit LearnPageModel(QObject* parent = nullptr);

    QVariantList startedPlaylist() const;
    QVariantList advancedPlaylist() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void setSearchText(const QString& text);
    Q_INVOKABLE QVariantMap classesAuthor() const;

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

#endif // MU_LEARN_LEARNPAGEMODEL_H
