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
#include "iinteractive.h"
#include "inotationpagestate.h"

#include "uicomponents/view/abstractmenumodel.h"

namespace mu::notation {
class AddMenuController;
}

namespace mu::appshell {
class FileMenuController;
class EditMenuController;
class ViewMenuController;
class FormatMenuController;
class ToolsMenuController;
class HelpMenuController;
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
    INJECT(appshell, framework::IInteractive, interactive)
    INJECT(appshell, INotationPageState, notationPageState)

public:
    explicit AppMenuModel(QObject* parent = nullptr);
    ~AppMenuModel();

    Q_INVOKABLE void load();
    Q_INVOKABLE void handleAction(const QString& actionCodeStr, int actionIndex);

private:
    notation::IMasterNotationPtr currentMasterNotation() const;
    notation::INotationPtr currentNotation() const;
    notation::INotationInteractionPtr notationInteraction() const;
    notation::INotationSelectionPtr notationSelection() const;

    void setupConnections();

    uicomponents::MenuItem fileItem();
    uicomponents::MenuItem editItem();
    uicomponents::MenuItem viewItem();
    uicomponents::MenuItem addItem();
    uicomponents::MenuItem formatItem();
    uicomponents::MenuItem toolsItem();
    uicomponents::MenuItem helpItem();

    uicomponents::MenuItemList recentScores();
    uicomponents::MenuItemList notesItems();
    uicomponents::MenuItemList intervalsItems();
    uicomponents::MenuItemList tupletsItems();
    uicomponents::MenuItemList measuresItems();
    uicomponents::MenuItemList framesItems();
    uicomponents::MenuItemList textItems();
    uicomponents::MenuItemList linesItems();
    uicomponents::MenuItemList toolbarsItems();
    uicomponents::MenuItemList workspacesItems();

    bool isPanelVisible(PanelType type) const;

    bool needSaveScore() const;
    bool scoreOpened() const;
    bool canUndo() const;
    bool canRedo() const;
    bool selectedElementOnScore() const;
    bool selectedRangeOnScore() const;
    bool hasSelectionOnScore() const;
    bool isNoteInputMode() const;
    bool isNotationPage() const;

    notation::ScoreConfig scoreConfig() const;

    notation::AddMenuController* m_addMenuController = nullptr;
    FileMenuController* m_fileMenuController = nullptr;
    EditMenuController* m_editMenuController = nullptr;
    ViewMenuController* m_viewMenuController = nullptr;
    FormatMenuController* m_formatMenuController = nullptr;
    ToolsMenuController* m_toolsMenuController = nullptr;
    HelpMenuController* m_helpMenuController = nullptr;
};
}

#endif // MU_APPSHELL_APPMENUMODEL_H
