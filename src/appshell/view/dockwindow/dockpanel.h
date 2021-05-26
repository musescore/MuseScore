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

#ifndef MU_DOCK_DOCKPANEL_H
#define MU_DOCK_DOCKPANEL_H

#include "internal/dockbase.h"

#include "framework/uicomponents/view/qmllistproperty.h"

namespace mu::dock {
class DockPanel : public DockBase
{
    Q_OBJECT

    Q_PROPERTY(DockPanel * tabifyPanel READ tabifyPanel WRITE setTabifyPanel NOTIFY tabifyPanelChanged)

public:
    explicit DockPanel(QQuickItem* parent = nullptr);

    DockPanel* tabifyPanel() const;

public slots:
    void setTabifyPanel(DockPanel* panel);

signals:
    void tabifyPanelChanged(DockPanel* panel);

private:
    DockType type() const override;

    DockPanel* m_tabifyPanel = nullptr;
};
}

#endif // MU_DOCK_DOCKPANEL_H
