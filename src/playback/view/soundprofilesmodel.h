/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MU_PLAYBACK_SOUNDPROFILESMODEL_H
#define MU_PLAYBACK_SOUNDPROFILESMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "internal/isoundprofilesrepository.h"
#include "iplaybackconfiguration.h"
#include "iplaybackcontroller.h"
#include "playbacktypes.h"

namespace mu::playback {
class SoundProfilesModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT_STATIC(ISoundProfilesRepository, profilesRepo)
    INJECT_STATIC(context::IGlobalContext, context)
    INJECT_STATIC(IPlaybackConfiguration, config)
    INJECT_STATIC(IPlaybackController, controller)

    Q_PROPERTY(QString activeProfile READ activeProfile WRITE setActiveProfile NOTIFY activeProfileChanged)
    Q_PROPERTY(
        QString defaultProjectsProfile READ defaultProjectsProfile WRITE setDefaultProjectsProfile NOTIFY defaultProjectsProfileChanged)
    Q_PROPERTY(
        QString currentlySelectedProfile READ currentlySelectedProfile WRITE setCurrentlySelectedProfile NOTIFY currentlySelectedProfileChanged)
public:
    explicit SoundProfilesModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    const QString& activeProfile() const;
    void setActiveProfile(const QString& newActiveProfile);

    const QString& defaultProjectsProfile() const;
    void setDefaultProjectsProfile(const QString& newDefaultProjectsProfile);

    const QString& currentlySelectedProfile() const;
    void setCurrentlySelectedProfile(const QString& newCurrentlySelectedProfile);

signals:
    void activeProfileChanged();
    void defaultProjectsProfileChanged();
    void currentlySelectedProfileChanged();

private:
    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleEnabled
    };

    std::vector<SoundProfile> m_profiles;

    QString m_activeProfile;
    QString m_defaultProjectsProfile;
    QString m_currentlySelectedProfile;
};
}

#endif // MU_PLAYBACK_SOUNDPROFILESMODEL_H
