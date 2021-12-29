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
#ifndef MU_APPSHELL_APPMENUMODEL_H
#define MU_APPSHELL_APPMENUMODEL_H

#include <memory>

#include "ui/view/abstractmenumodel.h"

#include "actions/actionable.h"
#include "modularity/ioc.h"
#include "ui/imainwindow.h"
#include "ui/iuiactionsregister.h"
#include "ui/inavigationcontroller.h"
#include "actions/iactionsdispatcher.h"
#include "workspace/iworkspacemanager.h"
#include "iappshellconfiguration.h"
#include "project/irecentprojectsprovider.h"

namespace mu::appshell {
class AppMenuModel : public ui::AbstractMenuModel
{
    Q_OBJECT

    INJECT(appshell, ui::IMainWindow, mainWindow)
    INJECT(appshell, ui::IUiActionsRegister, uiActionsRegister)
    INJECT(appshell, ui::INavigationController, navigationController)
    INJECT(appshell, actions::IActionsDispatcher, actionsDispatcher)
    INJECT(appshell, workspace::IWorkspaceManager, workspacesManager)
    INJECT(appshell, IAppShellConfiguration, configuration)
    INJECT(appshell, project::IRecentProjectsProvider, recentProjectsProvider)

    Q_PROPERTY(QString highlightedMenuId READ highlightedMenuId NOTIFY highlightedMenuIdChanged)

    Q_PROPERTY(QWindow * appWindow READ appWindow WRITE setAppWindow)

public:
    explicit AppMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;

    QWindow* appWindow() const;

public slots:
    void setHighlightedMenuId(QString highlightedMenuId);
    void setAppWindow(QWindow* appWindow);

signals:
    void openMenu(const QString& menuId);
    void highlightedMenuIdChanged(QString highlightedMenuId);

private:
    void setupConnections();
    void onActionsStateChanges(const actions::ActionCodeList& codes) override;

    using ui::AbstractMenuModel::makeMenuItem;
    ui::MenuItem* makeMenuItem(const actions::ActionCode& actionCode, ui::MenuItemRole role) const;

    ui::MenuItem* fileItem() const;
    ui::MenuItem* editItem() const;
    ui::MenuItem* viewItem() const;
    ui::MenuItem* addItem() const;
    ui::MenuItem* formatItem() const;
    ui::MenuItem* toolsItem() const;
    ui::MenuItem* helpItem() const;
    ui::MenuItem* diagnosticItem() const;

    ui::MenuItemList recentScores() const;
    ui::MenuItemList appendClearRecentSection(const ui::MenuItemList& recentScores) const;

    ui::MenuItemList notesItems() const;
    ui::MenuItemList intervalsItems() const;
    ui::MenuItemList tupletsItems() const;
    ui::MenuItemList measuresItems() const;
    ui::MenuItemList framesItems() const;
    ui::MenuItemList textItems() const;
    ui::MenuItemList linesItems() const;
    ui::MenuItemList toolbarsItems() const;
    ui::MenuItemList workspacesItems() const;

    // Custom navigation
    bool eventFilter(QObject* watched, QEvent* event) override;

    bool isNavigationStarted() const;
    bool isNavigateKey(int key) const;
    void navigate(int key);

    bool hasItemByActivateKey(const QString& keySymbol);
    void navigate(const QString& keySymbol);

    void resetNavigation();
    void navigateToFirstMenu();

    void saveMUNavigationSystemState();
    void restoreMUNavigationSystemState();

    void activateHighlightedMenu();

    QString highlightedMenuId() const;

    QString menuIdByActivateSymbol(const QString& symbol);

    QString m_highlightedMenuId;
    QWindow* m_appWindow = nullptr;

    bool m_needActivateHighlight = true;
    ui::INavigationControl* m_lastActiveNavigationControl = nullptr;
};
}

#endif // MU_APPSHELL_APPMENUMODEL_H
