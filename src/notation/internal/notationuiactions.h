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
#ifndef MU_NOTATION_NOTATIONUIACTIONS_H
#define MU_NOTATION_NOTATIONUIACTIONS_H

#include "ui/iuiactionsmodule.h"
#include "notationtypes.h"
#include "notationactioncontroller.h"
#include "modularity/ioc.h"
#include "context/iuicontextresolver.h"
#include "engraving/iengravingconfiguration.h"
#include "async/asyncable.h"
#include "ui/uitypes.h"

namespace mu::notation {
class NotationUiActions : public ui::IUiActionsModule, public async::Asyncable
{
    INJECT(context::IUiContextResolver, uicontextResolver)
    INJECT(engraving::IEngravingConfiguration, engravingConfiguration)

public:

    NotationUiActions(std::shared_ptr<NotationActionController> controller);

    void init();

    const ui::UiActionList& actionsList() const override;

    bool actionEnabled(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionCheckedChanged() const override;

    static DurationType actionDurationType(const actions::ActionCode& actionCode);
    static AccidentalType actionAccidentalType(const actions::ActionCode& actionCode);
    static int actionDotCount(const actions::ActionCode& actionCode);
    static int actionVoice(const actions::ActionCode& actionCode);
    static SymbolId actionArticulationSymbolId(const actions::ActionCode& actionCode);

    static const ui::ToolConfig& defaultNoteInputBarConfig();

private:
    static const ui::UiActionList m_actions;
    static const ui::UiActionList m_scoreConfigActions;
    static const ui::UiActionList m_engravingDebuggingActions;

    bool isScoreConfigAction(const actions::ActionCode& code) const;
    bool isScoreConfigChecked(const actions::ActionCode& code, const ScoreConfig& cfg) const;

    std::shared_ptr<NotationActionController> m_controller;
    async::Channel<actions::ActionCodeList> m_actionEnabledChanged;
    async::Channel<actions::ActionCodeList> m_actionCheckedChanged;
};
}

#endif // MU_NOTATION_NOTATIONUIACTIONS_H
