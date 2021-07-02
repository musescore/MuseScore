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

#ifndef MU_DOCK_DOCKPAGE_H
#define MU_DOCK_DOCKPAGE_H

#include "framework/uicomponents/view/qmllistproperty.h"

#include <QQuickItem>

#include "internal/dockbase.h"
#include "docktypes.h"

namespace mu::dock {
class DockToolBar;
class DockPanel;
class DockCentral;
class DockStatusBar;
class DockToolBarHolder;
class DockPanelHolder;
class DockPage : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBar> mainToolBars READ mainToolBarsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBar> toolBars READ toolBarsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBarHolder> toolBarsDockingHolders READ toolBarsDockingHoldersProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockPanel> panels READ panelsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockPanelHolder> panelsDockingHolders READ panelsDockingHoldersProperty)
    Q_PROPERTY(mu::dock::DockCentral* centralDock READ centralDock WRITE setCentralDock NOTIFY centralDockChanged)
    Q_PROPERTY(mu::dock::DockStatusBar* statusBar READ statusBar WRITE setStatusBar NOTIFY statusBarChanged)

public:
    explicit DockPage(QQuickItem* parent = nullptr);

    void init();
    void close();

    QString uri() const;

    QQmlListProperty<DockToolBar> mainToolBarsProperty();
    QQmlListProperty<DockToolBar> toolBarsProperty();
    QQmlListProperty<DockToolBarHolder> toolBarsDockingHoldersProperty();
    QQmlListProperty<DockPanel> panelsProperty();
    QQmlListProperty<DockPanelHolder> panelsDockingHoldersProperty();

    QList<DockToolBar*> mainToolBars() const;
    QList<DockToolBar*> toolBars() const;
    QList<DockToolBarHolder*> toolBarsHolders() const;
    DockCentral* centralDock() const;
    DockStatusBar* statusBar() const;
    QList<DockPanel*> panels() const;
    QList<DockPanelHolder*> panelsHolders() const;
    QList<DockBase*> allDocks() const;

    DockBase* dockByName(const QString& dockName) const;
    DockToolBarHolder* toolBarHolderByLocation(DockBase::DockLocation location) const;
    DockPanelHolder* panelHolderByLocation(DockBase::DockLocation location) const;

    bool isDockShown(const QString& dockName) const;
    void toggleDockVisibility(const QString& dockName);

public slots:
    void setUri(const QString& uri);
    void setCentralDock(DockCentral* central);
    void setStatusBar(DockStatusBar* statusBar);

signals:
    void inited();
    void uriChanged(const QString& uri);
    void centralDockChanged(DockCentral* central);
    void statusBarChanged(DockStatusBar* statusBar);

private:
    void componentComplete() override;

    QString m_uri;
    uicomponents::QmlListProperty<DockToolBar> m_mainToolBars;
    uicomponents::QmlListProperty<DockToolBar> m_toolBars;
    uicomponents::QmlListProperty<DockToolBarHolder> m_toolBarsDockingHolders;
    uicomponents::QmlListProperty<DockPanel> m_panels;
    uicomponents::QmlListProperty<DockPanelHolder> m_panelsDockingHolders;
    DockCentral* m_central = nullptr;
    DockStatusBar* m_statusBar = nullptr;
};
}

#endif // MU_DOCK_DOCKPAGE_H
