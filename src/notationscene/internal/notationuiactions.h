/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "async/asyncable.h"
#include "context/iuicontextresolver.h"
#include "engraving/iengravingconfiguration.h"
#include "modularity/ioc.h"
#include "notation/notationtypes.h"
#include "ui/iuiactionsmodule.h"

#include "notationactioncontroller.h"

namespace mu::notation {
class NotationUiActions : public muse::ui::IUiActionsModule, public muse::async::Asyncable, public muse::Injectable
{
    muse::GlobalInject<engraving::IEngravingConfiguration> engravingConfiguration;
    muse::Inject<context::IUiContextResolver> uicontextResolver = { this };

public:

    NotationUiActions(std::shared_ptr<NotationActionController> controller, const muse::modularity::ContextPtr& iocCtx);

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
    static const muse::ui::UiActionList s_actions;
    static const muse::ui::UiActionList s_undoRedoActions;
    static const muse::ui::UiActionList s_scoreConfigActions;
    static const muse::ui::UiActionList s_engravingDebuggingActions;

    void updateActionsEnabled(const muse::ui::UiActionList& actions);

    bool isScoreConfigAction(const muse::actions::ActionCode& code) const;
    bool isScoreConfigChecked(const muse::actions::ActionCode& code, const ScoreConfig& cfg) const;

    std::shared_ptr<NotationActionController> m_controller;
    muse::async::Channel<muse::actions::ActionCodeList> m_actionEnabledChanged;
    muse::async::Channel<muse::actions::ActionCodeList> m_actionCheckedChanged;

    std::unordered_map<muse::actions::ActionCode, bool> m_actionEnabledMap;
};
}
