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

#include "dockpanel.h"

#include "thirdparty/KDDockWidgets/src/DockWidgetQuick.h"

#include "log.h"
#include "translation.h"

#include "ui/uitypes.h"
#include "ui/view/abstractmenumodel.h"

using namespace mu::dock;
using namespace mu::uicomponents;
using namespace mu::ui;
using namespace mu::actions;

class DockPanel::DockPanelMenuModel : public AbstractMenuModel
{
public:
    DockPanelMenuModel(DockPanel* panel)
        : AbstractMenuModel(panel), m_panel(panel)
    {
        connect(m_panel, &DockPanel::floatingChanged, this, [this]() {
            load();
        });
    }

    void load() override
    {
        TRACEFUNC;

        MenuItemList items;

        if (m_customMenuModel && m_customMenuModel->rowCount() > 0) {
            items << m_customMenuModel->items();
            items << makeSeparator();
        }

        QString toggleFloatingTitle = m_panel->floating() ? mu::qtrc("dock", "Dock")
                                                          : mu::qtrc("dock", "Undock");

        items << makeDockPanelItem("close-dock", mu::qtrc("dock", "Close"));
        items << makeDockPanelItem("toggle-floating", toggleFloatingTitle);

        setItems(items);
    }

    AbstractMenuModel* customMenuModel() const
    {
        return m_customMenuModel;
    }

    void setCustomMenuModel(AbstractMenuModel* model)
    {
        m_customMenuModel = model;

        if (!model) {
            return;
        }

        connect(model, &AbstractMenuModel::itemsChanged, this, [this]() {
            load();
        });
    }

private:
    MenuItem makeDockPanelItem(const QString& code, const QString& title) const
    {
        MenuItem item;
        item.id = code;
        item.code = codeFromQString(code);
        item.title = title;
        item.state.enabled = true;
        item.args = ActionData::make_arg1<QString>(m_panel->objectName());

        return item;
    }

    AbstractMenuModel* m_customMenuModel = nullptr;
    DockPanel* m_panel = nullptr;
};

DockPanel::DockPanel(QQuickItem* parent)
    : DockBase(parent), m_menuModel(new DockPanelMenuModel(this))
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
}

DockPanel::~DockPanel()
{
    KDDockWidgets::DockWidgetQuick* w = dockWidget();
    IF_ASSERT_FAILED(w) {
        return;
    }

    w->setProperty(DOCK_PANEL_PROPERY, QVariant::fromValue(nullptr));
    w->setProperty(CONTEXT_MENU_MODEL_PROPERTY, QVariant::fromValue(nullptr));
}

DockPanel* DockPanel::tabifyPanel() const
{
    return m_tabifyPanel;
}

void DockPanel::setTabifyPanel(DockPanel* panel)
{
    if (panel == m_tabifyPanel) {
        return;
    }

    m_tabifyPanel = panel;
    emit tabifyPanelChanged(panel);
}

DockType DockPanel::type() const
{
    return DockType::Panel;
}

void DockPanel::componentComplete()
{
    DockBase::componentComplete();

    KDDockWidgets::DockWidgetQuick* w = dockWidget();
    IF_ASSERT_FAILED(w) {
        return;
    }

    m_menuModel->load();

    w->setProperty(DOCK_PANEL_PROPERY, QVariant::fromValue(this));
    w->setProperty(CONTEXT_MENU_MODEL_PROPERTY, QVariant::fromValue(m_menuModel));

    connect(m_menuModel, &AbstractMenuModel::itemsChanged, [w, this]() {
        if (w) {
            w->setProperty(CONTEXT_MENU_MODEL_PROPERTY, QVariant::fromValue(m_menuModel));
        }
    });
}

QObject* DockPanel::navigationSection() const
{
    return m_navigationSection;
}

void DockPanel::setNavigationSection(QObject* newNavigation)
{
    if (m_navigationSection == newNavigation) {
        return;
    }

    m_navigationSection = newNavigation;
    emit navigationSectionChanged();
}

AbstractMenuModel* DockPanel::contextMenuModel() const
{
    return m_menuModel->customMenuModel();
}

void DockPanel::setContextMenuModel(AbstractMenuModel* model)
{
    if (m_menuModel->customMenuModel() == model) {
        return;
    }

    m_menuModel->setCustomMenuModel(model);
    emit contextMenuModelChanged();
}
