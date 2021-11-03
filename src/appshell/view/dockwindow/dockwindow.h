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
#include "internal/dockbase.h"

namespace KDDockWidgets {
class MainWindowBase;
class LayoutSaver;
}

namespace mu::dock {
class DockToolBarView;
class DockingHolderView;
class DockPageView;
class DockPanelView;
class DockWindow : public QQuickItem, public IDockWindow, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString currentPageUri READ currentPageUri NOTIFY currentPageUriChanged)

    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBarView> toolBars READ toolBarsProperty)
    Q_PROPERTY(
        mu::dock::DockingHolderView
        * mainToolBarDockingHolder READ mainToolBarDockingHolder WRITE setMainToolBarDockingHolder NOTIFY mainToolBarDockingHolderChanged)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockPageView> pages READ pagesProperty)

    INJECT(dock, IDockWindowProvider, dockWindowProvider)
    INJECT(dock, ui::IUiConfiguration, uiConfiguration)
    INJECT(dock, appshell::IStartupScenario, startupScenario)

public:
    explicit DockWindow(QQuickItem* parent = nullptr);
    ~DockWindow() override;

    QString currentPageUri() const;

    QQmlListProperty<mu::dock::DockToolBarView> toolBarsProperty();
    QQmlListProperty<mu::dock::DockPageView> pagesProperty();
    DockingHolderView* mainToolBarDockingHolder() const;

    Q_INVOKABLE void loadPage(const QString& uri);

    //! IDockWindow
    bool isDockOpen(const QString& dockName) const override;
    void toggleDock(const QString& dockName) override;
    void setDockOpen(const QString& dockName, bool open) override;

    async::Channel<QStringList> docksOpenStatusChanged() const override;

    bool isDockFloating(const QString& dockName) const override;
    void toggleDockFloating(const QString& dockName) override;

    DropDestination hover(const QString& draggedDockName, const QPoint& globalPos) override;
    void endHover() override;

public slots:
    void setMainToolBarDockingHolder(DockingHolderView* mainToolBarDockingHolder);

signals:
    void currentPageUriChanged(const QString& uri);

    void mainToolBarDockingHolderChanged(DockingHolderView* mainToolBarDockingHolder);

private slots:
    void onQuit();

private:
    DockPageView* pageByUri(const QString& uri) const;
    DockPageView* currentPage() const;

    void componentComplete() override;
    void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry) override;

    void loadPageContent(const DockPageView* page);
    void unitePanelsToTabs(const DockPageView* page);
    void loadPageToolbars(const DockPageView* page);
    void loadPagePanels(const DockPageView* page);
    void alignToolBars(const DockPageView* page);

    void addDock(DockBase* dock, KDDockWidgets::Location location, const DockBase* relativeTo = nullptr);

    void saveGeometry();
    void restoreGeometry();

    QByteArray windowState() const;

    void savePageState(const QString& pageName);
    void restorePageState(const QString& pageName);

    void resetWindowState();
    bool restoreLayout(const QByteArray& layout, KDDockWidgets::RestoreOptions options = KDDockWidgets::RestoreOptions());

    void initDocks(DockPageView* page);

    QList<DockToolBarView*> topLevelToolBars(const DockPageView* page) const;

    bool isMouseOverDock(const QPoint& mouseLocalPos, const DockBase* dock) const;
    void updateToolBarOrientation(DockToolBarView* draggedToolBar, const DropDestination& dropDestination = DropDestination());
    void setCurrentDropDestination(const DockBase* draggedDock, const DropDestination& dropDestination);

    DropDestination resolveDropDestination(const DockBase* draggedDock, const QPoint& localPos) const;
    DockingHolderView* resolveDockingHolder(DockType draggedDockType, const QPoint& localPos) const;
    DockingHolderView* resolveToolbarDockingHolder(const QPoint& localPos) const;
    DockingHolderView* resolvePanelDockingHolder(const QPoint& localPos) const;

    DockPanelView* findTabifyPanel(const DockPanelView* panel, const QPoint& localPos) const;
    DockPanelView* findRootPanel(const DockPanelView* panel) const;

    DockBase* dockByName(const QString& dockName) const;

    KDDockWidgets::MainWindowBase* m_mainWindow = nullptr;
    QString m_currentPageUri;
    uicomponents::QmlListProperty<DockToolBarView> m_toolBars;
    DockingHolderView* m_mainToolBarDockingHolder = nullptr;
    uicomponents::QmlListProperty<DockPageView> m_pages;
    DropDestination m_currentDropDestination;
    async::Channel<QStringList> m_docksOpenStatusChanged;

    bool m_quiting = false;
};
}

#endif // MU_DOCK_DOCKWINDOW_H
