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

#include "docktypes.h"

namespace mu::dock {
class DockToolBar;
class DockPanel;
class DockCentral;
class DockStatusBar;
class DockBase;
class DockPage : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBar> mainToolBars READ mainToolBarsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBar> toolBars READ toolBarsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBar> toolBarsDockingHelpers READ toolBarsDockingHelpersProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockPanel> panels READ panelsProperty)
    Q_PROPERTY(mu::dock::DockCentral* centralDock READ centralDock WRITE setCentralDock NOTIFY centralDockChanged)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockStatusBar> statusBars READ statusBarsProperty)

public:
    explicit DockPage(QQuickItem* parent = nullptr);

    void init();
    void close();

    QString uri() const;

    QQmlListProperty<DockToolBar> mainToolBarsProperty();
    QQmlListProperty<DockToolBar> toolBarsProperty();
    QQmlListProperty<DockPanel> panelsProperty();
    QQmlListProperty<DockStatusBar> statusBarsProperty();
    QQmlListProperty<DockToolBar> toolBarsDockingHelpersProperty();

    QList<DockToolBar*> mainToolBars() const;
    QList<DockToolBar*> toolBars() const;
    QList<DockToolBar*> toolBarsHelpers() const;
    DockCentral* centralDock() const;
    QList<DockPanel*> panels() const;
    QList<DockStatusBar*> statusBars() const;
    QList<DockBase*> allDocks() const;
    DockBase* dockByName(const QString& dockName) const;

    void close();

public slots:
    void setUri(const QString& uri);
    void setCentralDock(DockCentral* central);

signals:
    void uriChanged(const QString& uri);
    void centralDockChanged(DockCentral* central);

private:
    void componentComplete() override;

    QString m_uri;
    uicomponents::QmlListProperty<DockToolBar> m_mainToolBars;
    uicomponents::QmlListProperty<DockToolBar> m_toolBars;
    uicomponents::QmlListProperty<DockToolBar> m_toolBarsDockingHelpers;
    uicomponents::QmlListProperty<DockPanel> m_panels;
    DockCentral* m_central = nullptr;
    uicomponents::QmlListProperty<DockStatusBar> m_statusBars;
};
}

#endif // MU_DOCK_DOCKPAGE_H
