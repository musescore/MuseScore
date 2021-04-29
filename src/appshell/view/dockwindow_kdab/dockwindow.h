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

namespace KDDockWidgets {
class MainWindowBase;
}

namespace mu::dock {
class DockToolBar;
class DockPage;
class DockBase;
class DockWindow : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString currentPageUri READ currentPageUri NOTIFY currentPageUriChanged)

    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBar> toolBars READ toolBarsProperty)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockPage> pages READ pagesProperty)

public:
    explicit DockWindow(QQuickItem* parent = nullptr);

    QString currentPageUri() const;

    QQmlListProperty<mu::dock::DockToolBar> toolBarsProperty();
    QQmlListProperty<mu::dock::DockPage> pagesProperty();

    Q_INVOKABLE void loadPage(const QString& uri);

signals:
    void currentPageUriChanged(const QString& uri);

private:
    DockPage* pageByUri(const QString& uri) const;

    void componentComplete() override;

    void loadPageContent(const DockPage* page);
    void unitePanelsToTabs(const DockPage* page);

    void addDock(DockBase* dock, KDDockWidgets::Location location, const DockBase* relativeTo = nullptr);

    KDDockWidgets::MainWindowBase* m_mainWindow = nullptr;
    QString m_currentPageUri;
    uicomponents::QmlListProperty<DockToolBar> m_toolBars;
    uicomponents::QmlListProperty<DockPage> m_pages;
};
}

#endif // MU_DOCK_DOCKWINDOW_H
