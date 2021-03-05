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

EditMenuController::EditMenuController()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

mu::async::Channel<std::vector<mu::actions::ActionCode> > EditMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
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
