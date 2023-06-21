/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_PROJECT_SCORESPAGEMODEL_H
#define MU_PROJECT_SCORESPAGEMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "iprojectconfiguration.h"
#include "actions/iactionsdispatcher.h"
#include "iinteractive.h"
#include "cloud/musescorecom/imusescorecomservice.h"

namespace mu::project {
class ScoresPageModel : public QObject
{
    Q_OBJECT

    INJECT(IProjectConfiguration, configuration)
    INJECT(actions::IActionsDispatcher, dispatcher)
    INJECT(framework::IInteractive, interactive)
    INJECT(cloud::IMuseScoreComService, museScoreComService)

    Q_PROPERTY(int tabIndex READ tabIndex WRITE setTabIndex NOTIFY tabIndexChanged)

public:
    explicit ScoresPageModel(QObject* parent = nullptr);

    Q_INVOKABLE void createNewScore();
    Q_INVOKABLE void openOther();
    Q_INVOKABLE void openScore(const QString& scorePath, const QString& displayNameOverride);
    Q_INVOKABLE void openScoreManager();

    int tabIndex() const;
    void setTabIndex(int index);

signals:
    void tabIndexChanged();
};
}

#endif // MU_PROJECT_SCORESPAGEMODEL_H
