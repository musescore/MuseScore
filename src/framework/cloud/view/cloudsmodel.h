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
#ifndef MUSE_CLOUD_CLOUDSMODEL_H
#define MUSE_CLOUD_CLOUDSMODEL_H

#include <QAbstractListModel>
#include <QVariant>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "cloud/musescorecom/imusescorecomservice.h"
#include "cloud/audiocom/iaudiocomservice.h"

namespace muse::cloud {
class CloudsModel : public QAbstractListModel, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool userAuthorized READ userAuthorized NOTIFY userAuthorizedChanged)

#ifdef MUSE_MODULE_CLOUD_MUSESCORECOM
    Inject<IMuseScoreComService> museScoreComService = { this };
#endif
    Inject<IAudioComService> audioComService = { this };

public:
    explicit CloudsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool userAuthorized() const;
    Q_INVOKABLE QVariant firstAuthorizedCloudInfo() const;
    Q_INVOKABLE QVariant cloudInfo(const QString& cloudCode) const;
    Q_INVOKABLE QVariantList visibilityModel(const QString& cloudCode) const;
    Q_INVOKABLE QVariant dialogText(const QString& cloudCode, const QString& existingScoreOrAudioUrl) const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void createAccount(const QString& code);
    Q_INVOKABLE void signIn(const QString& code);
    Q_INVOKABLE void signOut(const QString& code);

signals:
    void userAuthorizedChanged();

private:
    enum Roles {
        rCloudCode = Qt::UserRole + 1,
        rCloudTitle,
        rUserIsAuthorized,
        rUserName,
        rUserProfileUrl,
        rUserAvatarUrl,
        rUserCollectionUrl
    };

    int indexByCode(const QString& code) const;

    int indexOfFirstAuthorizedCloud() const;

    std::vector<IAuthorizationServicePtr> m_clouds;
};
}

#endif // MUSE_CLOUD_CLOUDSMODEL_H
