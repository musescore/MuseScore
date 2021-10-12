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

#include "internal/dockbase.h"
#include "docktypes.h"

namespace mu::dock {
class DockToolBarView;
class DockPanelView;
class DockCentralView;
class DockStatusBarView;
class DockToolBarHolder;
class DockPanelHolder;
class DockPageView : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBarView> mainToolBars READ mainToolBarsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBarView> toolBars READ toolBarsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBarHolder> toolBarsDockingHolders READ toolBarsDockingHoldersProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockPanelView> panels READ panelsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockPanelHolder> panelsDockingHolders READ panelsDockingHoldersProperty)
    Q_PROPERTY(mu::dock::DockCentralView * centralDock READ centralDock WRITE setCentralDock NOTIFY centralDockChanged)
    Q_PROPERTY(mu::dock::DockStatusBarView * statusBar READ statusBar WRITE setStatusBar NOTIFY statusBarChanged)

public:
    explicit DockPageView(QQuickItem* parent = nullptr);

    void init();
    void close();

    QString uri() const;

    QQmlListProperty<DockToolBarView> mainToolBarsProperty();
    QQmlListProperty<DockToolBarView> toolBarsProperty();
    QQmlListProperty<DockToolBarHolder> toolBarsDockingHoldersProperty();
    QQmlListProperty<DockPanelView> panelsProperty();
    QQmlListProperty<DockPanelHolder> panelsDockingHoldersProperty();

    QList<DockToolBarView*> mainToolBars() const;
    QList<DockToolBarView*> toolBars() const;
    QList<DockToolBarHolder*> toolBarsHolders() const;
    DockCentralView* centralDock() const;
    DockStatusBarView* statusBar() const;
    QList<DockPanelView*> panels() const;
    QList<DockPanelHolder*> panelsHolders() const;
    QList<DockBase*> allDocks() const;

    DockBase* dockByName(const QString& dockName) const;
    DockToolBarHolder* toolBarHolderByLocation(DockBase::DockLocation location) const;
    DockPanelHolder* panelHolderByLocation(DockBase::DockLocation location) const;

    bool isDockOpen(const QString& dockName) const;
    void toggleDock(const QString& dockName);
    void setDockOpen(const QString& dockName, bool open);

    bool isDockFloating(const QString& dockName) const;
    void toggleDockFloating(const QString& dockName);

public slots:
    void setUri(const QString& uri);
    void setCentralDock(DockCentralView* central);
    void setStatusBar(DockStatusBarView* statusBar);

signals:
    void inited();
    void uriChanged(const QString& uri);
    void centralDockChanged(DockCentralView* central);
    void statusBarChanged(DockStatusBarView* statusBar);

private:
    void componentComplete() override;

    DockPanelView* findPanelForTab(const DockPanelView* tab) const;

    QString m_uri;
    uicomponents::QmlListProperty<DockToolBarView> m_mainToolBars;
    uicomponents::QmlListProperty<DockToolBarView> m_toolBars;
    uicomponents::QmlListProperty<DockToolBarHolder> m_toolBarsDockingHolders;
    uicomponents::QmlListProperty<DockPanelView> m_panels;
    uicomponents::QmlListProperty<DockPanelHolder> m_panelsDockingHolders;
    DockCentralView* m_central = nullptr;
    DockStatusBarView* m_statusBar = nullptr;
};
}

#endif // MU_DOCK_DOCKPAGEVIEW_H
