/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <QQuickItem>

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"

#include "docktabsmodel.h"

namespace KDDockWidgets {
class Frame;
class DockWidgetBase;
}

namespace muse::dock {
class DockFrameModel : public QObject, public muse::LazyInjectable
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem * frame READ frame WRITE setFrame NOTIFY frameChanged)
    Q_PROPERTY(DockTabsModel * tabsModel READ tabsModel CONSTANT)

    Q_PROPERTY(QQmlComponent * titleBar READ titleBar NOTIFY titleBarChanged)
    Q_PROPERTY(bool titleBarAllowed READ titleBarAllowed NOTIFY titleBarAllowedChanged)
    Q_PROPERTY(bool isHorizontalPanel READ isHorizontalPanel NOTIFY isHorizontalPanelChanged)
    Q_PROPERTY(QObject * navigationSection READ navigationSection NOTIFY navigationSectionChanged)
    Q_PROPERTY(QString currentDockUniqueName READ currentDockUniqueName NOTIFY currentDockChanged)

    Q_PROPERTY(bool highlightingVisible READ highlightingVisible NOTIFY highlightingVisibleChanged)
    Q_PROPERTY(QRect highlightingRect READ highlightingRect NOTIFY highlightingVisibleChanged)

    muse::LazyInject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    explicit DockFrameModel(QObject* parent = nullptr);

    QQuickItem* frame() const;
    DockTabsModel* tabsModel() const { return m_tabsModel; }

    QQmlComponent* titleBar() const;
    bool titleBarAllowed() const;
    bool isHorizontalPanel() const;
    QObject* navigationSection() const;
    QString currentDockUniqueName() const;

    bool highlightingVisible() const;
    QRect highlightingRect() const;

    Q_INVOKABLE void handleMenuItem(const QString& itemId) const;

public slots:
    void setFrame(QQuickItem* item);

signals:
    void frameChanged(QQuickItem* frame);
    void titleBarChanged();
    void titleBarAllowedChanged(bool visible);
    void isHorizontalPanelChanged();
    void navigationSectionChanged();
    void currentDockChanged();
    void highlightingVisibleChanged();

private:
    bool eventFilter(QObject* watched, QEvent* event);

    QVariant currentDockContextMenuModel() const;
    QVariant currentDockToolbarComponent() const;

    void onContextMenuChanged(QObject* obj);
    void onToolBarComponentChanged(QObject* obj);

    void listenChangesInFrame();
    void setTitleBarAllowed(bool allowed);
    void setIsHorizontalPanel(bool is);

    KDDockWidgets::DockWidgetBase* currentDockWidget() const;
    QVariant currentDockProperty(const char* propertyName) const;

    QObject* currentNavigationSection() const;
    void updateNavigationSection();

    QQmlComponent* currentTitleBar() const;
    void updateTitleBar();

    KDDockWidgets::Frame* m_frame = nullptr;
    QQmlComponent* m_titleBar = nullptr;
    bool m_titleBarAllowed = false;
    bool m_isHorizontalPanel = false;
    QObject* m_navigationSection = nullptr;

    DockTabsModel* m_tabsModel = nullptr;
};
}
