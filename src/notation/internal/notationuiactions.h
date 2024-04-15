/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
class NotationUiActions : public muse::ui::IUiActionsModule, public muse::async::Asyncable
{
    INJECT(context::IUiContextResolver, uicontextResolver)
    INJECT(engraving::IEngravingConfiguration, engravingConfiguration)

public:

    NotationUiActions(std::shared_ptr<NotationActionController> controller);

    void init();

    const muse::ui::UiActionList& actionsList() const override;

    bool actionEnabled(const muse::ui::UiAction& act) const override;
    muse::async::Channel<muse::actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const muse::ui::UiAction& act) const override;
    muse::async::Channel<muse::actions::ActionCodeList> actionCheckedChanged() const override;

    static DurationType actionDurationType(const muse::actions::ActionCode& actionCode);
    static AccidentalType actionAccidentalType(const muse::actions::ActionCode& actionCode);
    static int actionDotCount(const muse::actions::ActionCode& actionCode);
    static int actionVoice(const muse::actions::ActionCode& actionCode);
    static SymbolId actionArticulationSymbolId(const muse::actions::ActionCode& actionCode);

    static const muse::ui::ToolConfig& defaultNoteInputBarConfig();

private:
    static const muse::ui::UiActionList m_actions;
    static const muse::ui::UiActionList m_scoreConfigActions;
    static const muse::ui::UiActionList m_engravingDebuggingActions;

    bool isScoreConfigAction(const muse::actions::ActionCode& code) const;
    bool isScoreConfigChecked(const muse::actions::ActionCode& code, const ScoreConfig& cfg) const;

    std::shared_ptr<NotationActionController> m_controller;
    muse::async::Channel<muse::actions::ActionCodeList> m_actionEnabledChanged;
    muse::async::Channel<muse::actions::ActionCodeList> m_actionCheckedChanged;
};
}

#endif // MU_NOTATION_NOTATIONUIACTIONS_H
