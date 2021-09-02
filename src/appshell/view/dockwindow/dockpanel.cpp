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

static const QString SET_DOCK_OPEN_ACTION_CODE = "set-dock-open";
static const QString TOGGLE_FLOATING_ACTION_CODE = "toggle-floating";

class DockPanel::DockPanelMenuModel : public AbstractMenuModel
{
public:
    DockPanelMenuModel(DockPanel* panel)
        : AbstractMenuModel(panel), m_panel(panel)
    {
        listenFloatingChanged();
    }

    void load() override
    {
        TRACEFUNC;

        MenuItemList items;

        if (m_customMenuModel && m_customMenuModel->rowCount() > 0) {
            items << m_customMenuModel->items();
            items << makeSeparator();
        }

        MenuItem closeDockItem = buildMenuItem(SET_DOCK_OPEN_ACTION_CODE, mu::qtrc("dock", "Close"));
        closeDockItem.args = ActionData::make_arg2<QString, bool>(m_panel->objectName(), false);
        items << closeDockItem;

        MenuItem toggleFloatingItem = buildMenuItem(TOGGLE_FLOATING_ACTION_CODE, toggleFloatingActionTitle());
        toggleFloatingItem.args = ActionData::make_arg1<QString>(m_panel->objectName());
        items << toggleFloatingItem;

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

        connect(model, &AbstractMenuModel::itemChanged, this, [this](const MenuItem& item) {
            updateItem(item);
        });
    }

private:
    MenuItem buildMenuItem(const QString& actionCode, const QString& title) const
    {
        MenuItem item;
        item.id = actionCode;
        item.code = codeFromQString(actionCode);
        item.title = title;
        item.state.enabled = true;

        return item;
    }

    QString toggleFloatingActionTitle() const
    {
        return m_panel->floating() ? mu::qtrc("dock", "Dock") : mu::qtrc("dock", "Undock");
    }

    void listenFloatingChanged()
    {
        connect(m_panel, &DockPanel::floatingChanged, this, [this]() {
            int index = itemIndex(TOGGLE_FLOATING_ACTION_CODE);

            if (index == INVALID_ITEM_INDEX) {
                return;
            }

            MenuItem& item = this->item(index);
            item.title = toggleFloatingActionTitle();

            QModelIndex modelIndex = this->index(index);
            emit dataChanged(modelIndex, modelIndex, { TitleRole });
        });
    }

    void updateItem(const MenuItem& newItem)
    {
        int index = itemIndex(newItem.id);

        if (index == INVALID_ITEM_INDEX) {
            return;
        }

        item(index) = newItem;

        QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex);
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
