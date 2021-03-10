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
#include "editmenucontroller.h"

#include "log.h"

using namespace mu::appshell;
using namespace mu::notation;
using namespace mu::actions;

void EditMenuController::init()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool EditMenuController::contains(const mu::actions::ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();
    return actions.find(actionCode) != actions.end();
}

bool EditMenuController::actionAvailable(const mu::actions::ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();

    if (actions.find(actionCode) == actions.end()) {
        return false;
    }

    return actions[actionCode]();
}

mu::async::Channel<mu::actions::ActionCodeList> EditMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

std::map<mu::actions::ActionCode, EditMenuController::AvailableCallback> EditMenuController::actions() const
{
    static std::map<ActionCode, AvailableCallback> _actions = {
        { "undo", std::bind(&EditMenuController::isUndoAvailable, this) },
        { "redo", std::bind(&EditMenuController::isRedoAvailable, this) },
        { "cut", std::bind(&EditMenuController::isCutAvailable, this) },
        { "copy", std::bind(&EditMenuController::isCopyAvailable, this) },
        { "paste", std::bind(&EditMenuController::isPasteAvailable, this, PastingType::Default) },
        { "paste-half", std::bind(&EditMenuController::isPasteAvailable, this, PastingType::Half) },
        { "paste-double", std::bind(&EditMenuController::isPasteAvailable, this, PastingType::Double) },
        { "paste-special", std::bind(&EditMenuController::isPasteAvailable, this, PastingType::Special) },
        { "swap", std::bind(&EditMenuController::isSwapAvailable, this) },
        { "delete", std::bind(&EditMenuController::isDeleteAvailable, this) },
        { "select-all", std::bind(&EditMenuController::isSelectAllAvailable, this) },
        { "select-similar", std::bind(&EditMenuController::isSelectSimilarAvailable, this) },
        { "find", std::bind(&EditMenuController::isFindAvailable, this) },
        { "preference-dialog", std::bind(&EditMenuController::isPreferenceDialogAvailable, this) }
    };

    return _actions;
}

bool EditMenuController::isUndoAvailable() const
{
    return controller()->actionAvailable("undo");
}

bool EditMenuController::isRedoAvailable() const
{
    return controller()->actionAvailable("redo");
}

bool EditMenuController::isCutAvailable() const
{
    return controller()->actionAvailable("cut");
}

bool EditMenuController::isCopyAvailable() const
{
    return controller()->actionAvailable("copy");
}

bool EditMenuController::isPasteAvailable(mu::notation::PastingType pastingType) const
{
    return controller()->actionAvailable(pastingActionCode(pastingType));
}

bool EditMenuController::isSwapAvailable() const
{
    return controller()->actionAvailable("swap");
}

bool EditMenuController::isDeleteAvailable() const
{
    return controller()->actionAvailable("delete");
}

bool EditMenuController::isSelectSimilarAvailable() const
{
    return controller()->actionAvailable("select-similar");
}

bool EditMenuController::isSelectAllAvailable() const
{
    return controller()->actionAvailable("select-all");
}

bool EditMenuController::isFindAvailable() const
{
    return controller()->actionAvailable("find");
}

bool EditMenuController::isPreferenceDialogAvailable() const
{
    NOT_IMPLEMENTED;
    return false;
}

std::string EditMenuController::pastingActionCode(mu::notation::PastingType type) const
{
    std::map<PastingType, std::string> pastingTypeStrings {
        { PastingType::Default, "paste" },
        { PastingType::Half, "paste-half" },
        { PastingType::Double, "paste-double" },
        { PastingType::Special, "paste-special" },
    };

    return pastingTypeStrings[type];
}
