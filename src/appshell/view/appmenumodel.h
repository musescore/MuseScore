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
#include "actions/iactionsregister.h"
#include "actions/iactionsdispatcher.h"
#include "shortcuts/ishortcutsregister.h"
#include "uicomponents/uicomponentstypes.h"
#include "context/iglobalcontext.h"
#include "workspace/iworkspacemanager.h"
#include "iappshellconfiguration.h"

namespace mu::appshell {
class AppMenuModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, actions::IActionsRegister, actionsRegister)
    INJECT(appshell, actions::IActionsDispatcher, actionsDispatcher)
    INJECT(appshell, shortcuts::IShortcutsRegister, shortcutsRegister)
    INJECT(appshell, context::IGlobalContext, globalContext)
    INJECT(appshell, workspace::IWorkspaceManager, workspacesManager)
    INJECT(appshell, IAppShellConfiguration, configuration)

    Q_PROPERTY(QVariantList items READ items NOTIFY itemsChanged)

public:
    explicit AppMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void handleAction(const QString& actionCode);

    QVariantList items();

signals:
    void itemsChanged();

private:
    notation::IMasterNotationPtr currentMasterNotation() const;
    notation::INotationPtr currentNotation() const;

    uicomponents::MenuItem fileItem();
    uicomponents::MenuItem editItem();
    uicomponents::MenuItem viewItem();
    uicomponents::MenuItem addItem();
    uicomponents::MenuItem formatItem();
    uicomponents::MenuItem toolsItem();
    uicomponents::MenuItem helpItem();

    uicomponents::MenuItemList notesItems() const;
    uicomponents::MenuItemList intervalsItems() const;
    uicomponents::MenuItemList tupletsItems() const;
    uicomponents::MenuItemList measuresItems() const;
    uicomponents::MenuItemList framesItems() const;
    uicomponents::MenuItemList textItems() const;
    uicomponents::MenuItemList linesItems() const;
    uicomponents::MenuItemList workspacesItems() const;

    uicomponents::MenuItem makeMenu(const std::string& title, const uicomponents::MenuItemList& actions, bool enabled = true);
    uicomponents::MenuItem makeAction(const actions::ActionCode& actionCode, bool enabled = true, bool checked = false,
                                      const std::string& section = "") const;
    uicomponents::MenuItem makeSeparator() const;

    bool needSaveScore() const;
    bool scoreOpened() const;
    bool canUndo() const;
    bool canRedo() const;
    bool selectedElementOnScore() const;
    bool isNoteInputMode() const;

    uicomponents::MenuItemList m_items;
};
}

#endif // MU_APPSHELL_APPMENUMODEL_H
