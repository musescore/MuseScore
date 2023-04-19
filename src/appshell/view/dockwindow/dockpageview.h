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

#ifndef MU_DOCK_DOCKPAGEVIEW_H
#define MU_DOCK_DOCKPAGEVIEW_H

#include "framework/uicomponents/view/qmllistproperty.h"

#include <QQuickItem>

#include "modularity/ioc.h"
#include "ui/inavigationcontroller.h"

#include "internal/dockbase.h"
#include "docktypes.h"

namespace mu::ui {
class NavigationControl;
}

namespace mu::dock {
class DockToolBarView;
class DockPanelView;
class DockCentralView;
class DockStatusBarView;
class DockingHolderView;
class DockPageView : public QQuickItem
{
    Q_OBJECT

    INJECT(dock, ui::INavigationController, navigationController)

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBarView> mainToolBars READ mainToolBarsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBarView> toolBars READ toolBarsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockingHolderView> toolBarsDockingHolders READ toolBarsDockingHoldersProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockPanelView> panels READ panelsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockingHolderView> panelsDockingHolders READ panelsDockingHoldersProperty)
    Q_PROPERTY(mu::dock::DockCentralView * centralDock READ centralDock WRITE setCentralDock NOTIFY centralDockChanged)
    Q_PROPERTY(mu::dock::DockStatusBarView * statusBar READ statusBar WRITE setStatusBar NOTIFY statusBarChanged)

public:
    explicit DockPageView(QQuickItem* parent = nullptr);

    void init();

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

    Q_INVOKABLE void setDefaultNavigationControl(mu::ui::NavigationControl* control);

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

    DockPanelView* findFirstPanelForTab(const DockPanelView* tab) const;
    DockPanelView* findCurrentPanelForTab(const DockPanelView* tab) const;

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

#endif // MU_DOCK_DOCKPAGEVIEW_H
