//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_WORKSPACE_CURRENTWORKSPACEMODEL_H
#define MU_WORKSPACE_CURRENTWORKSPACEMODEL_H

#include <QObject>

#include "iworkspaceconfiguration.h"
#include "iinteractive.h"
#include "async/asyncable.h"
#include "modularity/ioc.h"

namespace mu::workspace {
class CurrentWorkspaceModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(workspace, IWorkspaceConfiguration, configuration)
    INJECT(workspace, framework::IInteractive, interactive)

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
