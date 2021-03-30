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
#ifndef MU_APPSHELL_APPLICATIONUIACTIONS_H
#define MU_APPSHELL_APPLICATIONUIACTIONS_H

#include "ui/iuiactionsmodule.h"
#include "applicationactioncontroller.h"
#include "modularity/ioc.h"
#include "context/iuicontextresolver.h"
#include "inotationpagestate.h"
#include "async/asyncable.h"

namespace mu::appshell {
class ApplicationUiActions : public ui::IUiActionsModule, public async::Asyncable
{
    INJECT(appshell, INotationPageState, notationPageState)
public:

    ApplicationUiActions(std::shared_ptr<ApplicationActionController> controller);

    void init();

    const ui::UiActionList& actionsList() const override;

    bool actionEnabled(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionCheckedChanged() const override;

private:

    PanelType panelType(const actions::ActionCode& code) const;
    actions::ActionCode panelTypeToAction(const PanelType& type) const;

    static const ui::UiActionList m_actions;
    static const std::vector<std::pair<actions::ActionCode, PanelType> > m_panels;

    std::shared_ptr<ApplicationActionController> m_controller;
    async::Channel<actions::ActionCodeList> m_actionEnabledChanged;
    async::Channel<actions::ActionCodeList> m_actionCheckedChanged;
};
}

#endif // MU_APPSHELL_APPLICATIONUIACTIONS_H
