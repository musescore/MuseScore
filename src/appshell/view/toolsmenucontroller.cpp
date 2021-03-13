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
#include "toolsmenucontroller.h"

using namespace mu::appshell;
using namespace mu::actions;
using namespace mu::uicomponents;

void ToolsMenuController::init()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const ActionCodeList& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool ToolsMenuController::contains(const ActionCode& actionCode) const
{
    ActionCodeList actions = this->actions();
    return std::find(actions.begin(), actions.end(), actionCode) != actions.end();
}

ActionState ToolsMenuController::actionState(const ActionCode& actionCode) const
{
    ActionState state;
    state.enabled = actionEnabled(actionCode);

    return state;
}

mu::async::Channel<ActionCodeList> ToolsMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

ActionCodeList ToolsMenuController::actions() const
{
    static ActionCodeList actions = {
        "voice-x12",
        "voice-x13",
        "voice-x14",
        "voice-x23",
        "voice-x24",
        "voice-x34",
        "split-measure",
        "join-measures",
        "transpose",
        "explode",
        "implode",
        "realize-chord-symbols",
        "time-delete",
        "slash-fill",
        "slash-rhythm",
        "pitch-spell",
        "reset-groupings",
        "resequence-rehearsal-marks",
        "unroll-repeats",
        "copy-lyrics-to-clipboard",
        "fotomode",
        "del-empty-measures"
    };

    return actions;
}

bool ToolsMenuController::actionEnabled(const ActionCode& actionCode) const
{
    return controller()->actionAvailable(actionCode);
}
