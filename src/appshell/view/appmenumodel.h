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
#include "uicomponents/uicomponentstypes.h"
#include "workspace/iworkspacemanager.h"
#include "iappshellconfiguration.h"
#include "userscores/iuserscoresservice.h"

#include "uicomponents/view/abstractmenumodel.h"
#include "uicomponents/imenucontrollersregister.h"

namespace mu::appshell {
class AppMenuModel : public QObject, public uicomponents::AbstractMenuModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, actions::IActionsDispatcher, actionsDispatcher)
    INJECT(appshell, workspace::IWorkspaceManager, workspacesManager)
    INJECT(appshell, IAppShellConfiguration, configuration)
    INJECT(appshell, userscores::IUserScoresService, userScoresService)

    INJECT(appshell, uicomponents::IMenuControllersRegister, menuControllersRegister)

    Q_PROPERTY(QVariantList items READ items NOTIFY itemsChanged)

public:
    explicit AppMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void handleAction(const QString& actionCodeStr, int actionIndex);

    uicomponents::ActionState actionState(const actions::ActionCode& actionCode) const override;

signals:
    void itemsChanged();

private:
    void setupConnections();

    uicomponents::MenuItem fileItem() const;
    uicomponents::MenuItem editItem() const;
    uicomponents::MenuItem viewItem() const;
    uicomponents::MenuItem addItem() const;
    uicomponents::MenuItem formatItem() const;
    uicomponents::MenuItem toolsItem() const;
    uicomponents::MenuItem helpItem() const;

    uicomponents::MenuItemList recentScores() const;
    uicomponents::MenuItemList notesItems() const;
    uicomponents::MenuItemList intervalsItems() const;
    uicomponents::MenuItemList tupletsItems() const;
    uicomponents::MenuItemList measuresItems() const;
    uicomponents::MenuItemList framesItems() const;
    uicomponents::MenuItemList textItems() const;
    uicomponents::MenuItemList linesItems() const;
    uicomponents::MenuItemList toolbarsItems() const;
    uicomponents::MenuItemList workspacesItems() const;
};
}

#endif // MU_APPSHELL_APPMENUMODEL_H
