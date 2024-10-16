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

#ifndef MUSE_DOCK_DOCKPANELVIEW_H
#define MUSE_DOCK_DOCKPANELVIEW_H

#include "internal/dockbase.h"

#include "uicomponents/view/abstractmenumodel.h"

namespace muse::uicomponents {
class AbstractMenuModel;
}

namespace muse::dock {
class DockPanelView : public DockBase
{
    Q_OBJECT

    Q_PROPERTY(QString groupName READ groupName WRITE setGroupName NOTIFY groupNameChanged)
    Q_PROPERTY(QObject * navigationSection READ navigationSection WRITE setNavigationSection NOTIFY navigationSectionChanged)
    Q_PROPERTY(
        muse::uicomponents::AbstractMenuModel
        * contextMenuModel READ contextMenuModel WRITE setContextMenuModel NOTIFY contextMenuModelChanged)
    Q_PROPERTY(QQmlComponent * titleBar READ titleBar WRITE setTitleBar NOTIFY titleBarChanged)

public:
    explicit DockPanelView(QQuickItem* parent = nullptr);
    ~DockPanelView() override;

    QString groupName() const;
    QObject* navigationSection() const;
    uicomponents::AbstractMenuModel* contextMenuModel() const;
    QQmlComponent* titleBar() const;

    bool isTabAllowed(const DockPanelView* tab) const;
    void addPanelAsTab(DockPanelView* tab);
    void setCurrentTabIndex(int index);

public slots:
    void setGroupName(const QString& name);
    void setNavigationSection(QObject* newNavigation);
    void setContextMenuModel(uicomponents::AbstractMenuModel* model);
    void setTitleBar(QQmlComponent* titleBar);

signals:
    void groupNameChanged();
    void navigationSectionChanged();
    void contextMenuModelChanged();
    void titleBarChanged();

private:
    void componentComplete() override;

    QString m_groupName;
    QObject* m_navigationSection = nullptr;

    class DockPanelMenuModel;
    DockPanelMenuModel* m_menuModel = nullptr;
    QQmlComponent* m_titleBar = nullptr;
};
}

#endif // MUSE_DOCK_DOCKPANELVIEW_H
