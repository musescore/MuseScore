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
#ifndef MU_INSTRUMENTSSCENE_INSTRUMENTPANELCONTEXTMENUMODEL_H
#define MU_INSTRUMENTSSCENE_INSTRUMENTPANELCONTEXTMENUMODEL_H

#include "uicomponents/view/abstractmenumodel.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/iinstrumentsrepository.h"
#include "actions/iactionsdispatcher.h"

namespace mu::instrumentsscene {
class InstrumentsPanelContextMenuModel : public uicomponents::AbstractMenuModel, public actions::Actionable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)
    INJECT(notation::IInstrumentsRepository, instrumentsRepository)
    INJECT(actions::IActionsDispatcher, dispatcher)

public:
    explicit InstrumentsPanelContextMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;

private:
    void loadItems();
    void buildMenu();
    void setInstrumentsOrder(const actions::ActionData& args);
    void updateOrderingMenu(const QString& newOrderId);

    notation::IMasterNotationPtr m_masterNotation;
    notation::ScoreOrderList m_orders;
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTPANELCONTEXTMENUMODEL_H
