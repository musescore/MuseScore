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

#ifndef MU_WORKSPACE_CURRENTWORKSPACEMODEL_H
#define MU_WORKSPACE_CURRENTWORKSPACEMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "iinteractive.h"
#include "iworkspaceconfiguration.h"

namespace mu::workspace {
class CurrentWorkspaceModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(workspace, IWorkspaceConfiguration, configuration)
    INJECT(workspace, framework::IInteractive, interactive)
    INJECT(workspace, actions::IActionsDispatcher, dispatcher)

    Q_PROPERTY(QString currentWorkspaceName READ currentWorkspaceName NOTIFY currentWorkspaceNameChanged)

public:
    explicit CurrentWorkspaceModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void selectWorkspace();

    QString currentWorkspaceName() const;

signals:
    void currentWorkspaceNameChanged();
};
}

#endif // MU_WORKSPACE_CURRENTWORKSPACEMODEL_H
