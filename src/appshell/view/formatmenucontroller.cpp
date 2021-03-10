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
using namespace mu::actions;

void FormatMenuController::init()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool FormatMenuController::contains(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();
    return actions.find(actionCode) != actions.end();
}

bool FormatMenuController::actionAvailable(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();

    if (actions.find(actionCode) == actions.end()) {
        return false;
    }

    return actions[actionCode]();
}

mu::async::Channel<ActionCodeList> FormatMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

std::map<mu::actions::ActionCode, FormatMenuController::AvailableCallback> FormatMenuController::actions() const
{
    static std::map<ActionCode, AvailableCallback> _actions = {
        { "stretch+", std::bind(&FormatMenuController::isStretchIncreaseAvailable, this) },
        { "stretch-", std::bind(&FormatMenuController::isStretchDecreaseAvailable, this) },
        { "reset-stretch", std::bind(&FormatMenuController::isResetAvailable, this, ResettableValueType::Stretch) },
        { "edit-style", std::bind(&FormatMenuController::isEditStyleAvailable, this) },
        { "page-settings", std::bind(&FormatMenuController::isPageSettingsAvailable, this) },
        { "reset-text-style-overrides",
          std::bind(&FormatMenuController::isResetAvailable, this, ResettableValueType::TextStyleOverriders) },
        { "reset-beammode", std::bind(&FormatMenuController::isResetAvailable, this, ResettableValueType::BeamMode) },
        { "reset", std::bind(&FormatMenuController::isResetAvailable, this, ResettableValueType::ShapesAndPosition) },
        { "load-style", std::bind(&FormatMenuController::isLoadStyleAvailable, this) },
        { "save-style", std::bind(&FormatMenuController::isSaveStyleAvailable, this) }
    };

    return _actions;
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
