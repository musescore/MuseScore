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
#include "formatmenucontroller.h"

#include "log.h"

using namespace mu::appshell;
using namespace mu::actions;
using namespace mu::uicomponents;

void FormatMenuController::init()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const actions::ActionCodeList& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool FormatMenuController::contains(const ActionCode& actionCode) const
{
    ActionCodeList actions = this->actions();
    return std::find(actions.begin(), actions.end(), actionCode) != actions.end();
}

ActionState FormatMenuController::actionState(const ActionCode& actionCode) const
{
    ActionState state;
    state.enabled = actionEnabled(actionCode);

    return state;
}

mu::async::Channel<ActionCodeList> FormatMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

ActionCodeList FormatMenuController::actions() const
{
    static ActionCodeList actions = {
        "stretch+",
        "stretch-",
        "reset-stretch",
        "edit-style",
        "page-settings",
        "reset-text-style-overrides",
        "reset-beammode",
        "reset",
        "load-style",
        "save-style",
        "add-remove-breaks"
    };

    return actions;
}

bool FormatMenuController::actionEnabled(const ActionCode& actionCode) const
{
    return controller()->actionAvailable(actionCode);
}
