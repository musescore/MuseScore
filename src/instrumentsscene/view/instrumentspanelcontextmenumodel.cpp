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

InstrumentsPanelContextMenuModel::InstrumentsPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void InstrumentsPanelContextMenuModel::load()
{
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        INotationPtr notation = globalContext()->currentNotation();
        m_masterNotation = globalContext()->currentMasterNotation();

        if (!m_masterNotation || !notation || m_masterNotation->notation() != notation) {
            clear();
        } else {
            loadItems();
        }
    });
}

void InstrumentsPanelContextMenuModel::loadItems()
{
    TRACEFUNC;

    m_orders.clear();

    RetValCh<InstrumentsMeta> meta = instrumentsRepository()->instrumentsMeta();
    if (!meta.ret) {
        LOGE() << meta.ret.toString();
        return;
    }

    m_orders = meta.val.scoreOrders;
    MenuItemList orderItems;

    dispatcher()->unReg(this);

    ScoreOrder currentOrder = m_masterNotation->notation()->scoreOrder();

    for (const ScoreOrder* order : m_orders) {
        MenuItem orderItem;

        orderItem.id = order->id;
        orderItem.title = order->name;
        orderItem.code = codeFromQString("set-order-" + order->id);
        orderItem.checkable = Checkable::Yes;
        orderItem.state.enabled = true;
        orderItem.state.checked = currentOrder.id == order->id;

        orderItems << orderItem;

        dispatcher()->reg(this, orderItem.code, [this, order]() {
            m_masterNotation->parts()->setScoreOrder(*order);
        });
    }

    MenuItemList items {
        makeMenu(qtrc("instruments", "Instrument ordering"), orderItems)
    };

    setItems(items);
}
