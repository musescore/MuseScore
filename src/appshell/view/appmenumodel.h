//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_APPSHELL_APPMENUMODEL_H
#define MU_APPSHELL_APPMENUMODEL_H

#include <QObject>

#include "modularity/ioc.h"

#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "workspace/iworkspacemanager.h"
#include "iappshellconfiguration.h"
#include "userscores/iuserscoresservice.h"

#include "ui/view/abstractmenumodel.h"

namespace mu::appshell {
class AppMenuModel : public QObject, public ui::AbstractMenuModel
{
    Q_OBJECT

    INJECT(appshell, actions::IActionsDispatcher, actionsDispatcher)
    INJECT(appshell, workspace::IWorkspaceManager, workspacesManager)
    INJECT(appshell, IAppShellConfiguration, configuration)
    INJECT(appshell, userscores::IUserScoresService, userScoresService)

    Q_PROPERTY(QVariantList items READ items NOTIFY itemsChanged)

public:
    explicit AppMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void handleAction(const QString& actionCodeStr, int actionIndex);

signals:
    void itemsChanged();

private:
    void setupConnections();
    void onActionsStateChanges(const actions::ActionCodeList& codes) override;

    ui::MenuItem fileItem() const;
    ui::MenuItem editItem() const;
    ui::MenuItem viewItem() const;
    ui::MenuItem addItem() const;
    ui::MenuItem formatItem() const;
    ui::MenuItem toolsItem() const;
    ui::MenuItem helpItem() const;

    ui::MenuItemList recentScores() const;
    ui::MenuItemList notesItems() const;
    ui::MenuItemList intervalsItems() const;
    ui::MenuItemList tupletsItems() const;
    ui::MenuItemList measuresItems() const;
    ui::MenuItemList framesItems() const;
    ui::MenuItemList textItems() const;
    ui::MenuItemList linesItems() const;
    ui::MenuItemList toolbarsItems() const;
    ui::MenuItemList workspacesItems() const;
};
}

#endif // MU_APPSHELL_APPMENUMODEL_H
