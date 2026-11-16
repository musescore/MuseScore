/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include "layoutpanelcontextmenumodel.h"

#include "thirdparty/kors_logger/src/log_base.h"
#include "types/translatablestring.h"

#include "notation/imasternotation.h"
#include "notation/inotationparts.h" // IWYU pragma: keep

#include "log.h"

using namespace mu::context;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace muse::uicomponents;

static const QString EXPAND_ALL_ID("expand-all-instruments");
static const QString COLLAPSE_ALL_ID("collapse-all-instruments");

static const QString ORDERING_MENU_ID("ordering-menu");

LayoutPanelContextMenuModel::LayoutPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void LayoutPanelContextMenuModel::load()
{
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        updateMenu();
    });

    updateMenu();
}

void LayoutPanelContextMenuModel::handleMenuItem(const QString& itemId)
{
    if (itemId == EXPAND_ALL_ID) {
        emit expandCollapseAllRequested(true);
    } else if (itemId == COLLAPSE_ALL_ID) {
        emit expandCollapseAllRequested(false);
    } else {
        setInstrumentsOrder(itemId);
    }
}

void LayoutPanelContextMenuModel::updateMenu()
{
    INotationPtr notation = globalContext()->currentNotation();
    m_masterNotation = globalContext()->currentMasterNotation();

    if (!m_masterNotation || !notation) {
        clear();
    } else if (m_masterNotation->notation() == notation) {
        buildMenu(true);
    } else {
        buildMenu(false);
    }

    if (!m_masterNotation) {
        return;
    }

    m_masterNotation->parts()->scoreOrderChanged().onNotify(this, [this] {
        updateOrderingMenu(m_masterNotation->parts()->scoreOrder().id);
    }, Mode::SetReplace);
}

void LayoutPanelContextMenuModel::loadInstrumentOrders()
{
    TRACEFUNC;

    ScoreOrder currentOrder = m_masterNotation->parts()->scoreOrder();
    m_orders = instrumentsRepository()->orders();

    const ScoreOrder& custom = customOrder();
    if (m_orders.empty() || !muse::contains(m_orders, custom)) {
        m_orders.push_back(custom);
    }

    if (!muse::contains(m_orders, currentOrder)) {
        currentOrder.customized = false;
        m_orders.push_back(currentOrder);
    }
}

void LayoutPanelContextMenuModel::buildMenu(bool includeInstrumentsOrdering)
{
    MenuItemList items;
    if (includeInstrumentsOrdering) {
        loadInstrumentOrders();
        items.append({
            createInstrumentsOrderingItem(),
            makeSeparator()
        });
    }

    items.append({
        createExpandCollapseAllItem(false),
        createExpandCollapseAllItem(true)
    });

    setItems(items);
}

void LayoutPanelContextMenuModel::setInstrumentsOrder(const QString& newOrderId)
{
    IF_ASSERT_FAILED(!newOrderId.isEmpty()) {
        return;
    }

    for (const ScoreOrder& order : m_orders) {
        if (order.id == newOrderId) {
            m_masterNotation->parts()->setScoreOrder(order);
            break;
        }
    }

    updateOrderingMenu(newOrderId);
}

void LayoutPanelContextMenuModel::updateOrderingMenu(const QString& newOrderId)
{
    MenuItem& orderingMenu = findMenu(ORDERING_MENU_ID);

    for (MenuItem* item : orderingMenu.subitems()) {
        item->setChecked(item->id() == newOrderId);
    }
}

MenuItem* LayoutPanelContextMenuModel::createInstrumentsOrderingItem()
{
    ScoreOrder currentOrder = m_masterNotation->parts()->scoreOrder();

    MenuItemList orderItems;

    auto createNewItem = [currentOrder, this](const ScoreOrder& order, bool customized) {
        MenuItem* orderItem = new MenuItem(this);
        orderItem->setId(order.id);
        orderItem->setTitle(order.getName());
        orderItem->setCheckable(true);
        orderItem->setChecked(!customized && currentOrder.id == order.id);
        return orderItem;
    };

    for (const ScoreOrder& order : m_orders) {
        orderItems << createNewItem(order, currentOrder.customized);

        if (currentOrder.customized && (currentOrder.id == order.id)) {
            orderItems << createNewItem(currentOrder, false);
        }
    }

    return makeMenu(TranslatableString("layoutpanel", "Instrument ordering"), orderItems, ORDERING_MENU_ID);
}

MenuItem* LayoutPanelContextMenuModel::createExpandCollapseAllItem(bool expand)
{
    MenuItem* item = new MenuItem(this);
    item->setId(expand ? EXPAND_ALL_ID : COLLAPSE_ALL_ID);
    item->setTitle(expand
                   ? TranslatableString("layoutpanel", "Expand all instruments")
                   : TranslatableString("layoutpanel", "Collapse all instruments"));

    return item;
}
