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
#ifndef MU_PROJECT_RECENTSCORESMODEL_H
#define MU_PROJECT_RECENTSCORESMODEL_H

#include <QAbstractListModel>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "iprojectconfiguration.h"
#include "irecentfilescontroller.h"
#include "iinteractive.h"
#include "cloud/musescorecom/imusescorecomservice.h"

namespace mu::project {
class RecentScoresModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(actions::IActionsDispatcher, dispatcher)
    INJECT(IProjectConfiguration, configuration)
    INJECT(IRecentFilesController, recentFilesController)
    INJECT(framework::IInteractive, interactive)
    INJECT(cloud::IMuseScoreComService, museScoreComService)

public:
    RecentScoresModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addNewScore();
    Q_INVOKABLE void openScore();
    Q_INVOKABLE void openRecentScore(const QString& scorePath);
    Q_INVOKABLE void openScoreManager();

private:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ScoreRole
    };

    void updateRecentScores();
    void setRecentScores(const QVariantList& recentScores);

    QVariantList m_recentScores;
};
}

#endif // MU_PROJECT_RECENTSCORESMODEL_H
