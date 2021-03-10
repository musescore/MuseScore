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
#include "context/iglobalcontext.h"
#include "workspace/iworkspacemanager.h"
#include "palette/ipaletteactionscontroller.h"
#include "iappshellconfiguration.h"
#include "userscores/iuserscoresservice.h"
#include "iapplicationactioncontroller.h"
#include "inotationpagestate.h"

#include "ifilemenucontroller.h"
#include "ieditmenucontroller.h"
#include "iviewmenucontroller.h"
#include "notation/iaddmenucontroller.h"
#include "iformatmenucontroller.h"
#include "itoolsmenucontroller.h"
#include "ihelpmenucontroller.h"

#include "uicomponents/view/abstractmenumodel.h"

namespace mu::appshell {
class AppMenuModel : public uicomponents::AbstractMenuModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, actions::IActionsDispatcher, actionsDispatcher)
    INJECT(appshell, context::IGlobalContext, globalContext)
    INJECT(appshell, workspace::IWorkspaceManager, workspacesManager)
    INJECT(appshell, palette::IPaletteActionsController, paletteController)
    INJECT(appshell, IAppShellConfiguration, configuration)
    INJECT(appshell, userscores::IUserScoresService, userScoresService)
    INJECT(appshell, IApplicationActionController, controller)
    INJECT(appshell, INotationPageState, notationPageState)

    INJECT(appshell, IFileMenuController, fileMenuController)
    INJECT(appshell, IEditMenuController, editMenuController)
    INJECT(appshell, IViewMenuController, viewMenuController)
    INJECT(appshell, notation::IAddMenuController, addMenuController)
    INJECT(appshell, IFormatMenuController, formatMenuController)
    INJECT(appshell, IToolsMenuController, toolsMenuController)
    INJECT(appshell, IHelpMenuController, helpMenuController)

public:
    explicit AppMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void handleAction(const QString& actionCodeStr, int actionIndex);

    bool actionEnabled(const actions::ActionCode& actionCode) const override;

private:
    notation::IMasterNotationPtr currentMasterNotation() const;
    notation::INotationPtr currentNotation() const;
    notation::INotationInteractionPtr notationInteraction() const;
    notation::INotationNoteInputPtr notationNoteInput() const;

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

    bool isPanelVisible(PanelType type) const;
    bool isNoteInputMode() const;

    notation::ScoreConfig scoreConfig() const;
};
}

#endif // MU_APPSHELL_APPMENUMODEL_H
