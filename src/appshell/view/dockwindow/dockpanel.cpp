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

class DockPanel::DockPanelMenuModel : public ui::AbstractMenuModel
{
public:
    explicit DockPanelMenuModel(QObject* parent = nullptr)
        : AbstractMenuModel(parent)
    {
    }

    void load() override
    {
        //! TODO: temporary solution for testing
        MenuItem close;
        close.code = "close";
        close.title = "Close tab";
        close.state.enabled = true;

        MenuItem undock;
        undock.code = "undock";
        undock.title = "Undock";
        undock.state.enabled = true;

        MenuItem move;
        move.code = "move";
        move.title = "Move panel to right side";
        move.state.enabled = true;

        MenuItemList standardItems {
            close,
            undock,
            move
        };

        setItems(standardItems);
    }

    QVariant customMenuModel() const
    {
        return m_customMenuModel;
    }

    void setCustomMenuModel(const QVariant& model)
    {
        m_customMenuModel = model;
    }

    QVariant toVariant() const
    {
        QVariantList result = m_customMenuModel.toList();
        if (!result.isEmpty()) {
            result << makeSeparator().toMap();
        }

        result << itemsProperty();

        return result;
    }

private:
    QVariant m_customMenuModel;
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
    w->setProperty(CONTEXT_MENU_MODEL_PROPERTY, m_menuModel->toVariant());

    connect(this, &DockPanel::contextMenuModelChanged, [this, w]() {
        if (w) {
            w->setProperty(CONTEXT_MENU_MODEL_PROPERTY, m_menuModel->toVariant());
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

QVariant DockPanel::contextMenuModel() const
{
    return m_menuModel->customMenuModel();
}

void DockPanel::setContextMenuModel(const QVariant& model)
{
    if (m_menuModel->customMenuModel() == model) {
        return;
    }

    m_menuModel->setCustomMenuModel(model);
    emit contextMenuModelChanged();
}
