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

#ifndef MU_DOCK_DOCKWINDOW_H
#define MU_DOCK_DOCKWINDOW_H

#include <QQuickItem>

#include "framework/uicomponents/view/qmllistproperty.h"

#include "thirdparty/KDDockWidgets/src/KDDockWidgets.h"

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "ui/iuiconfiguration.h"
#include "async/asyncable.h"
#include "internal/istartupscenario.h"
#include "idockwindow.h"
#include "idockwindowprovider.h"

namespace KDDockWidgets {
class MainWindowBase;
class LayoutSaver;
}

namespace mu::dock {
class DockToolBar;
class DockToolBarHolder;
class DockPanelHolder;
class DockPage;
class DockBase;
class DockWindow : public QQuickItem, public IDockWindow, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString currentPageUri READ currentPageUri NOTIFY currentPageUriChanged)

    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBar> toolBars READ toolBarsProperty)
    Q_PROPERTY(
        mu::dock::DockToolBarHolder
        * mainToolBarDockingHolder READ mainToolBarDockingHolder WRITE setMainToolBarDockingHolder NOTIFY mainToolBarDockingHolderChanged)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockPage> pages READ pagesProperty)

    INJECT(dock, IDockWindowProvider, dockWindowProvider)
    INJECT(dock, ui::IUiConfiguration, uiConfiguration)
    INJECT(dock, appshell::IStartupScenario, startupScenario)

public:
    explicit DockWindow(QQuickItem* parent = nullptr);
    ~DockWindow() override;

    QString currentPageUri() const;

    QQmlListProperty<mu::dock::DockToolBar> toolBarsProperty();
    QQmlListProperty<mu::dock::DockPage> pagesProperty();
    DockToolBarHolder* mainToolBarDockingHolder() const;

    Q_INVOKABLE void loadPage(const QString& uri);

    //! IDockWindow
    void setToolBarOrientation(const QString& toolBarName, framework::Orientation orientation) override;
    void showDockingHolder(const QPoint& globalPos, DockingHolderType type) override;
    void hideAllDockingHolders() override;

    bool isDockOpen(const QString& dockName) const override;
    void toggleDock(const QString& dockName) override;
    void setDockOpen(const QString& dockName, bool open) override;

    async::Channel<QStringList> docksOpenStatusChanged() const override;

    bool isDockFloating(const QString& dockName) const override;
    void toggleDockFloating(const QString& dockName) override;

public slots:
    void setMainToolBarDockingHolder(DockToolBarHolder* mainToolBarDockingHolder);

signals:
    void currentPageUriChanged(const QString& uri);

    void mainToolBarDockingHolderChanged(DockToolBarHolder* mainToolBarDockingHolder);

private slots:
    void onQuit();

private:
    DockPage* pageByUri(const QString& uri) const;
    DockPage* currentPage() const;

    void componentComplete() override;

    void loadPageContent(const DockPage* page);
    void unitePanelsToTabs(const DockPage* page);
    void loadPageToolbars(const DockPage* page);
    void loadPagePanels(const DockPage* page);

    void addDock(DockBase* dock, KDDockWidgets::Location location, const DockBase* relativeTo = nullptr);

    void saveGeometry();
    void restoreGeometry();

    QByteArray windowState() const;

    void savePageState(const QString& pageName);
    void restorePageState(const QString& pageName);

    void resetWindowState();
    bool restoreLayout(const QByteArray& layout, KDDockWidgets::RestoreOptions options = KDDockWidgets::RestoreOptions());

    void initDocks(DockPage* page);

    DockToolBarHolder* resolveToolbarDockingHolder(const QPoint& localPos) const;
    void showToolBarDockingHolder(const QPoint& globalPos);
    void hideCurrentToolBarDockingHolder();
    bool isMouseOverCurrentToolBarDockingHolder(const QPoint& mouseLocalPos) const;

    DockPanelHolder* resolvePanelDockingHolder(const QPoint& localPos) const;
    void showPanelDockingHolder(const QPoint& globalPos);
    void hideCurrentPanelDockingHolder();
    bool isMouseOverCurrentPanelDockingHolder(const QPoint& mouseLocalPos) const;

    KDDockWidgets::MainWindowBase* m_mainWindow = nullptr;
    QString m_currentPageUri;
    uicomponents::QmlListProperty<DockToolBar> m_toolBars;
    DockToolBarHolder* m_mainToolBarDockingHolder = nullptr;
    uicomponents::QmlListProperty<DockPage> m_pages;
    DockToolBarHolder* m_currentToolBarDockingHolder = nullptr;
    DockPanelHolder* m_currentPanelDockingHolder = nullptr;
    async::Channel<QStringList> m_docksOpenStatusChanged;

    bool m_quiting = false;
};
}

#endif // MU_DOCK_DOCKWINDOW_H
