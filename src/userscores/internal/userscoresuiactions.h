//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_USERSCORES_USERSCORESUIACTIONS_H
#define MU_USERSCORES_USERSCORESUIACTIONS_H

#include "ui/iuiactionsmodule.h"
#include "filescorecontroller.h"
#include "modularity/ioc.h"
#include "context/iuicontextresolver.h"

namespace mu::userscores {
class UserScoresUiActions : public ui::IUiActionsModule
{
    INJECT(userscores, context::IUiContextResolver, uicontextResolver)
public:

    UserScoresUiActions(std::shared_ptr<FileScoreController> controller);

    const ui::UiActionList& actionsList() const override;

    bool actionEnabled(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionCheckedChanged() const override;

private:
    static const ui::UiActionList m_actions;
    std::shared_ptr<FileScoreController> m_controller;
    async::Channel<actions::ActionCodeList> m_actionEnabledChanged;
    async::Channel<actions::ActionCodeList> m_actionCheckedChanged;
};
}

#endif // MU_USERSCORES_USERSCORESUIACTIONS_H
