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

class DockPanel::DockPanelMenuModel : public AbstractMenuModel
{
public:
    explicit DockPanelMenuModel(QObject* parent = nullptr)
        : AbstractMenuModel(parent)
    {
    }

    void load() override
    {
        MenuItemList items;

        if (m_customMenuModel && m_customMenuModel->rowCount() > 0) {
            items << m_customMenuModel->items();
            items << makeSeparator();
        }

        //! TODO: temporary solution for testing
        MenuItem close;
        close.code = "close";
        close.title = "Close tab";
        close.state.enabled = true;
        items << close;

        MenuItem undock;
        undock.code = "undock";
        undock.title = "Undock";
        undock.state.enabled = true;
        items << undock;

        MenuItem move;
        move.code = "move";
        move.title = "Move panel to right side";
        move.state.enabled = true;
        items << move;

        setItems(items);
    }

    AbstractMenuModel* customMenuModel() const
    {
        return m_customMenuModel;
    }

    void setCustomMenuModel(AbstractMenuModel* model)
    {
        m_customMenuModel = model;

        connect(model, &AbstractMenuModel::itemsChanged, this, [this]() {
            load();
        });
    }

private:
    AbstractMenuModel* m_customMenuModel = nullptr;
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

    w->setProperty(DOCK_PANEL_PROPERY, QVariant::fromValue(this));

    m_menuModel->load();

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
