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

#include "instrumentspanelcontextmenumodel.h"

#include "log.h"
#include "translation.h"

#include "actions/actiontypes.h"

using namespace mu::context;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::ui;
using namespace mu::actions;

static const ActionCode SET_INSTRUMENTS_ORDER_CODE("set-instruments-order");

static const QString ORDERING_MENU_ID("ordering-menu");

InstrumentsPanelContextMenuModel::InstrumentsPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void InstrumentsPanelContextMenuModel::load()
{
    dispatcher()->reg(this, SET_INSTRUMENTS_ORDER_CODE, this, &InstrumentsPanelContextMenuModel::setInstrumentsOrder);

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        INotationPtr notation = globalContext()->currentNotation();
        m_masterNotation = globalContext()->currentMasterNotation();

        if (!m_masterNotation || !notation || m_masterNotation->notation() != notation) {
            clear();
        } else {
            loadItems();
        }

        if (!m_masterNotation) {
            return;
        }

        m_masterNotation->parts()->scoreOrderChanged().onNotify(this, [this] {
            updateOrderingMenu(m_masterNotation->parts()->scoreOrder().id);
        });
    });
}

void InstrumentsPanelContextMenuModel::loadItems()
{
    TRACEFUNC;

    ScoreOrder currentOrder = m_masterNotation->parts()->scoreOrder();
    m_orders = instrumentsRepository()->orders();

    if (!m_orders.contains(currentOrder)) {
        m_orders.append(currentOrder);
    }

    MenuItemList orderItems;

    for (const ScoreOrder& order : m_orders) {
        MenuItem orderItem;

        orderItem.id = order.id;
        orderItem.title = order.name;
        orderItem.code = SET_INSTRUMENTS_ORDER_CODE;
        orderItem.checkable = Checkable::Yes;
        orderItem.state.enabled = true;
        orderItem.state.checked = currentOrder.id == order.id;
        orderItem.args = ActionData::make_arg1<QString>(order.id);

        orderItems << orderItem;
    }

    MenuItemList items {
        makeMenu(qtrc("instruments", "Instrument ordering"), orderItems, ORDERING_MENU_ID)
    };

    setItems(items);
}

void InstrumentsPanelContextMenuModel::setInstrumentsOrder(const actions::ActionData& args)
{
    if (args.empty()) {
        return;
    }

    QString newOrderId = args.arg<QString>(0);

    for (const ScoreOrder& order : m_orders) {
        if (order.id == newOrderId) {
            m_masterNotation->parts()->setScoreOrder(order);
            break;
        }
    }

    updateOrderingMenu(newOrderId);
}

void InstrumentsPanelContextMenuModel::updateOrderingMenu(const QString& newOrderId)
{
    MenuItem& orderingMenu = findMenu(ORDERING_MENU_ID);

    for (MenuItem& item : orderingMenu.subitems) {
        item.state.checked = item.id == newOrderId;
    }

    emit itemChanged(orderingMenu);
}
