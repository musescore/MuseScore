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
#ifndef MU_USERSCORES_RECENTSCORESMODEL_H
#define MU_USERSCORES_RECENTSCORESMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "iuserscoresconfiguration.h"
#include "iuserscoresservice.h"

namespace mu::userscores {
class RecentScoresModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(userscores, actions::IActionsDispatcher, dispatcher)
    INJECT(userscores, IUserScoresConfiguration, configuration)
    INJECT(userscores, IUserScoresService, userScoresService)

public:
    RecentScoresModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addNewScore();
    Q_INVOKABLE void openScore();
    Q_INVOKABLE void openRecentScore(const QString& scorePath);

private:
    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleScore
    };

    void updateRecentScores(const notation::MetaList& recentScoresList);
    void setRecentScores(const QVariantList& recentScores);

    QVariantList m_recentScores;
    QHash<int, QByteArray> m_roles;
};
}

#endif // MU_USERSCORES_RECENTSCORESMODEL_H
