//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_NOTATION_NOTATIONUIACTIONS_H
#define MU_NOTATION_NOTATIONUIACTIONS_H

#include "ui/iuiactionsmodule.h"
#include "notationtypes.h"
#include "notationactioncontroller.h"
#include "modularity/ioc.h"
#include "context/iuicontextresolver.h"
#include "async/asyncable.h"

namespace mu::notation {
class NotationUiActions : public ui::IUiActionsModule, public async::Asyncable
{
    INJECT(notation, context::IUiContextResolver, uicontextResolver)
public:

    NotationUiActions(std::shared_ptr<NotationActionController> controller);

    void init();

    const ui::UiActionList& actionsList() const override;

    bool actionEnabled(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionCheckedChanged() const override;

    static ui::UiActionList defaultNoteInputActions();

    static DurationType actionDurationType(const actions::ActionCode& actionCode);
    static AccidentalType actionAccidentalType(const actions::ActionCode& actionCode);
    static int actionDotCount(const actions::ActionCode& actionCode);
    static int actionVoice(const actions::ActionCode& actionCode);
    static SymbolId actionArticulationSymbolId(const actions::ActionCode& actionCode);

private:
    static const ui::UiActionList m_actions;
    static const ui::UiActionList m_noteInputActions;
    static const ui::UiActionList m_scoreConfigActions;

    bool isScoreConfigAction(const actions::ActionCode& code) const;
    bool isScoreConfigChecked(const actions::ActionCode& code, const ScoreConfig& cfg) const;

    std::shared_ptr<NotationActionController> m_controller;
    async::Channel<actions::ActionCodeList> m_actionEnabledChanged;
    async::Channel<actions::ActionCodeList> m_actionCheckedChanged;
};
}

#endif // MU_NOTATION_NOTATIONUIACTIONS_H
