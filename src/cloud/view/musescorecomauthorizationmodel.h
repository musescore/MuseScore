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
#ifndef MU_CLOUD_MUSESCORECOMAUTHORIZATIONMODEL_H
#define MU_CLOUD_MUSESCORECOMAUTHORIZATIONMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "cloud/musescorecom/imusescorecomservice.h"

namespace mu::cloud {
class MuseScoreComAuthorizationModel : public QObject, async::Asyncable
{
    Q_OBJECT

    INJECT(IMuseScoreComService, museScoreComService)

    Q_PROPERTY(bool userAuthorized READ userAuthorized NOTIFY userAuthorizedChanged)

public:
    explicit MuseScoreComAuthorizationModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    bool userAuthorized() const;

    Q_INVOKABLE void createAccount();
    Q_INVOKABLE void signIn();
    Q_INVOKABLE void signOut();

signals:
    void userAuthorizedChanged();
};
}

#endif // MU_CLOUD_MUSESCORECOMAUTHORIZATIONMODEL_H
