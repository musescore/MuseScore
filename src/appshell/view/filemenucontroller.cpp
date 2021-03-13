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
#include "filemenucontroller.h"

using namespace mu::appshell;
using namespace mu::actions;
using namespace mu::uicomponents;

void FileMenuController::init()
{
    fileController()->actionsAvailableChanged().onReceive(this, [this](const ActionCodeList& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });

    notationController()->actionsAvailableChanged().onReceive(this, [this](const ActionCodeList& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool FileMenuController::contains(const ActionCode& actionCode) const
{
    return fileControllerContains(actionCode) || notationControllerContains(actionCode);
}

ActionState FileMenuController::actionState(const ActionCode& actionCode) const
{
    ActionState state;
    state.enabled = actionEnabled(actionCode);

    return state;
}

mu::async::Channel<ActionCodeList> FileMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

ActionCodeList FileMenuController::fileControllerActions() const
{
    static ActionCodeList actions = {
        "file-new",
        "file-open",
        "clear-recent",
        "file-close",
        "file-save",
        "file-save-as",
        "file-save-a-copy",
        "file-save-selection",
        "file-save-online",
        "file-import-pdf",
        "file-export",
        "parts",
        "print",
        "quit"
    };

    return actions;
}

ActionCodeList FileMenuController::notationControllerActions() const
{
    static ActionCodeList actions = {
        "edit-info"
    };

    return actions;
}

bool FileMenuController::fileControllerContains(const ActionCode& actionCode) const
{
    ActionCodeList fileActions = fileControllerActions();
    return std::find(fileActions.begin(), fileActions.end(), actionCode) != fileActions.end();
}

bool FileMenuController::notationControllerContains(const ActionCode& actionCode) const
{
    ActionCodeList notationActions = notationControllerActions();
    return std::find(notationActions.begin(), notationActions.end(), actionCode) != notationActions.end();
}

bool FileMenuController::actionEnabled(const ActionCode& actionCode) const
{
    if (fileControllerContains(actionCode)) {
        return fileController()->actionAvailable(actionCode);
    }

    if (notationControllerContains(actionCode)) {
        return notationController()->actionAvailable(actionCode);
    }

    return false;
}
