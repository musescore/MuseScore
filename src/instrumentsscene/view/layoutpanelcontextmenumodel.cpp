/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "actions/actiontypes.h"
#include "types/translatablestring.h"

#include "log.h"

using namespace mu::context;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace muse::actions;

static const ActionCode SET_INSTRUMENTS_ORDER_CODE("set-instruments-order");
static const ActionCode EXPAND_ALL_CODE("expand-all-instruments");
static const ActionCode COLLAPSE_ALL_CODE("collapse-all-instruments");

static const QString ORDERING_MENU_ID("ordering-menu");

LayoutPanelContextMenuModel::LayoutPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void LayoutPanelContextMenuModel::load()
{
    dispatcher()->reg(this, SET_INSTRUMENTS_ORDER_CODE, this, &LayoutPanelContextMenuModel::setInstrumentsOrder);

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        updateMenu();
    });

    updateMenu();
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
    });
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

void LayoutPanelContextMenuModel::setInstrumentsOrder(const ActionData& args)
{
    if (args.empty()) {
        return;
    }

    String newOrderId = String::fromQString(args.arg<QString>(0));

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
        UiActionState state = item->state();
        state.checked = item->id() == newOrderId;
        item->setState(state);
    }
}

MenuItem* LayoutPanelContextMenuModel::createInstrumentsOrderingItem()
{
    ScoreOrder currentOrder = m_masterNotation->parts()->scoreOrder();

    MenuItemList orderItems;

    auto createNewItem = [currentOrder, this](const ScoreOrder& order, bool customized) {
        MenuItem* orderItem = new MenuItem(this);
        orderItem->setId(order.id);

        UiAction action;
        action.title = order.getName();
        action.code = SET_INSTRUMENTS_ORDER_CODE;
        action.checkable = Checkable::Yes;
        orderItem->setAction(action);

        UiActionState state;
        state.enabled = true;
        state.checked = !customized && currentOrder.id == order.id;
        orderItem->setState(state);

        orderItem->setArgs(ActionData::make_arg1<QString>(order.id));

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
    item->setId(QString::fromStdString(expand ? EXPAND_ALL_CODE : COLLAPSE_ALL_CODE));

    UiAction action;
    action.title = expand
                   ? TranslatableString("layoutpanel", "Expand all instruments")
                   : TranslatableString("layoutpanel", "Collapse all instruments");
    action.code = expand ? EXPAND_ALL_CODE : COLLAPSE_ALL_CODE;
    item->setAction(action);

    UiActionState state;
    state.enabled = true;
    item->setState(state);

    dispatcher()->reg(this, action.code, [this, expand]() {
        emit expandCollapseAllRequested(expand);
    });

    return item;
}
