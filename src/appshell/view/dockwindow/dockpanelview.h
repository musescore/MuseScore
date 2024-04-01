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

#ifndef MU_DOCK_DOCKPANELVIEW_H
#define MU_DOCK_DOCKPANELVIEW_H

#include "internal/dockbase.h"

#include "uicomponents/view/abstractmenumodel.h"

class QQmlComponent;

namespace mu::uicomponents {
class AbstractMenuModel;
}

namespace mu::dock {
class DockPanelView : public DockBase
{
    Q_OBJECT

    Q_PROPERTY(QString groupName READ groupName WRITE setGroupName NOTIFY groupNameChanged)
    Q_PROPERTY(
        mu::uicomponents::AbstractMenuModel
        * contextMenuModel READ contextMenuModel WRITE setContextMenuModel NOTIFY contextMenuModelChanged)
    Q_PROPERTY(QQmlComponent * toolbarComponent READ toolbarComponent WRITE setToolbarComponent NOTIFY toolbarComponentChanged)

public:
    explicit DockPanelView(QQuickItem* parent = nullptr);
    ~DockPanelView() override;

    QString groupName() const;
    uicomponents::AbstractMenuModel* contextMenuModel() const;
    QQmlComponent* toolbarComponent() const;

    bool isTabAllowed(const DockPanelView* tab) const;
    void addPanelAsTab(DockPanelView* tab);
    void setCurrentTabIndex(int index);

public slots:
    void setGroupName(const QString& name);
    void setContextMenuModel(uicomponents::AbstractMenuModel* model);
    void setToolbarComponent(QQmlComponent* component);

signals:
    void groupNameChanged();
    void contextMenuModelChanged();
    void toolbarComponentChanged();

private:
    void componentComplete() override;

    QString m_groupName;

    class DockPanelMenuModel;
    DockPanelMenuModel* m_menuModel = nullptr;

    QQmlComponent* m_toolbarComponent = nullptr;
};
}

#endif // MU_DOCK_DOCKPANELVIEW_H
