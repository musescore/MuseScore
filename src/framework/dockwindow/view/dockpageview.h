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

#ifndef MUSE_DOCK_DOCKPAGEVIEW_H
#define MUSE_DOCK_DOCKPAGEVIEW_H

#include "framework/uicomponents/view/qmllistproperty.h"

#include <QQuickItem>

#include "modularity/ioc.h"
#include "ui/inavigationcontroller.h"

#include "internal/dockbase.h"
#include "docktypes.h"

Q_MOC_INCLUDE("dockwindow/view/dockstatusbarview.h")

namespace mu::ui {
class NavigationControl;
}

namespace muse::dock {
class DockToolBarView;
class DockPanelView;
class DockCentralView;
class DockStatusBarView;
class DockingHolderView;
class DockPageView : public QQuickItem, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(QQmlListProperty<muse::dock::DockToolBarView> mainToolBars READ mainToolBarsProperty CONSTANT)
    Q_PROPERTY(QQmlListProperty<muse::dock::DockToolBarView> toolBars READ toolBarsProperty CONSTANT)
    Q_PROPERTY(QQmlListProperty<muse::dock::DockingHolderView> toolBarsDockingHolders READ toolBarsDockingHoldersProperty CONSTANT)
    Q_PROPERTY(QQmlListProperty<muse::dock::DockPanelView> panels READ panelsProperty CONSTANT)
    Q_PROPERTY(QQmlListProperty<muse::dock::DockingHolderView> panelsDockingHolders READ panelsDockingHoldersProperty)
    Q_PROPERTY(muse::dock::DockCentralView * centralDock READ centralDock WRITE setCentralDock NOTIFY centralDockChanged)
    Q_PROPERTY(muse::dock::DockStatusBarView * statusBar READ statusBar WRITE setStatusBar NOTIFY statusBarChanged)

    muse::Inject<ui::INavigationController> navigationController = { this };

public:
    explicit DockPageView(QQuickItem* parent = nullptr);

    void init();
    void deinit();

    QString uri() const;

    void setParams(const QVariantMap& params);

    QQmlListProperty<DockToolBarView> mainToolBarsProperty();
    QQmlListProperty<DockToolBarView> toolBarsProperty();
    QQmlListProperty<DockingHolderView> toolBarsDockingHoldersProperty();
    QQmlListProperty<DockPanelView> panelsProperty();
    QQmlListProperty<DockingHolderView> panelsDockingHoldersProperty();

    QList<DockToolBarView*> mainToolBars() const;
    QList<DockToolBarView*> toolBars() const;
    QList<DockingHolderView*> toolBarsHolders() const;
    DockCentralView* centralDock() const;
    DockStatusBarView* statusBar() const;
    QList<DockPanelView*> panels() const;
    QList<DockingHolderView*> panelsHolders() const;
    QList<DockBase*> allDocks() const;

    DockBase* dockByName(const QString& dockName) const;
    DockingHolderView* holder(DockType type, Location location) const;
    QList<DockPanelView*> possiblePanelsForTab(const DockPanelView* tab) const;

    bool isDockOpen(const QString& dockName) const;
    void toggleDock(const QString& dockName);
    void setDockOpen(const QString& dockName, bool open);

    bool isDockFloating(const QString& dockName) const;
    void toggleDockFloating(const QString& dockName);

    Q_INVOKABLE void setDefaultNavigationControl(muse::ui::NavigationControl* control);

public slots:
    void setUri(const QString& uri);
    void setCentralDock(DockCentralView* central);
    void setStatusBar(DockStatusBarView* statusBar);

signals:
    void inited();
    void setParamsRequested(const QVariantMap& params);
    void uriChanged(const QString& uri);
    void centralDockChanged(DockCentralView* central);
    void statusBarChanged(DockStatusBarView* statusBar);

private:
    void componentComplete() override;

    DockPanelView* findPanelForTab(const DockPanelView* tab) const;

    void reorderSections();
    void doReorderSections();
    void reorderDocksNavigationSections(QList<DockBase*>& docks);
    void reorderNavigationSectionPanels(QList<DockBase*>& sectionDocks);

    QString m_uri;
    uicomponents::QmlListProperty<DockToolBarView> m_mainToolBars;
    uicomponents::QmlListProperty<DockToolBarView> m_toolBars;
    uicomponents::QmlListProperty<DockingHolderView> m_toolBarsDockingHolders;
    uicomponents::QmlListProperty<DockPanelView> m_panels;
    uicomponents::QmlListProperty<DockingHolderView> m_panelsDockingHolders;
    DockCentralView* m_central = nullptr;
    DockStatusBarView* m_statusBar = nullptr;
};
}

#endif // MUSE_DOCK_DOCKPAGEVIEW_H
