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
using namespace mu::notation;

FormatMenuController::FormatMenuController()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

mu::async::Channel<std::vector<mu::actions::ActionCode> > FormatMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

bool FormatMenuController::isEditStyleAvailable() const
{
    return controller()->actionAvailable("edit-style");
}

bool FormatMenuController::isPageSettingsAvailable() const
{
    return controller()->actionAvailable("page-settings");
}

bool FormatMenuController::isStretchIncreaseAvailable() const
{
    return controller()->actionAvailable("stretch+");
}

bool FormatMenuController::isStretchDecreaseAvailable() const
{
    return controller()->actionAvailable("stretch+");
}

bool FormatMenuController::isResetAvailable(ResettableValueType resettableValueType) const
{
    return controller()->actionAvailable(resetableValueTypeActionCode(resettableValueType));
}

bool FormatMenuController::isLoadStyleAvailable() const
{
    return controller()->actionAvailable("load-style");
}

bool FormatMenuController::isSaveStyleAvailable() const
{
    return controller()->actionAvailable("save-style");
}

std::string FormatMenuController::resetableValueTypeActionCode(ResettableValueType valueType) const
{
    std::map<ResettableValueType, std::string> ressetableValueTypeStrings {
        { ResettableValueType::Stretch, "reset-stretch" },
        { ResettableValueType::BeamMode,"reset-beammode" },
        { ResettableValueType::ShapesAndPosition,"reset" },
        { ResettableValueType::TextStyleOverriders,"reset-text-style-overrides" }
    };

    return ressetableValueTypeStrings[valueType];
}
