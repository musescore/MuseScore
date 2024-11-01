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

#pragma once

#include "uicomponents/view/abstractmenumodel.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/iinstrumentsrepository.h"
#include "actions/iactionsdispatcher.h"

namespace mu::instrumentsscene {
class LayoutPanelContextMenuModel : public muse::uicomponents::AbstractMenuModel, public muse::actions::Actionable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)
    INJECT(notation::IInstrumentsRepository, instrumentsRepository)
    INJECT(muse::actions::IActionsDispatcher, dispatcher)

public:
    explicit LayoutPanelContextMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;

signals:
    void expandCollapseAllRequested(bool expand);

private:
    void updateMenu();
    void loadInstrumentOrders();
    void buildMenu(bool includeInstrumentsOrdering);
    void setInstrumentsOrder(const muse::actions::ActionData& args);
    void updateOrderingMenu(const QString& newOrderId);

    muse::uicomponents::MenuItem* createInstrumentsOrderingItem();
    muse::uicomponents::MenuItem* createExpandCollapseAllItem(bool expand);

    notation::IMasterNotationPtr m_masterNotation;
    notation::ScoreOrderList m_orders;
};
}
