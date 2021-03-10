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
#include "helpmenucontroller.h"

#include "log.h"

using namespace mu::appshell;
using namespace mu::actions;

void HelpMenuController::init()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool HelpMenuController::contains(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();
    return actions.find(actionCode) != actions.end();
}

bool HelpMenuController::actionAvailable(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();

    if (actions.find(actionCode) == actions.end()) {
        return false;
    }

    return actions[actionCode]();
}

mu::async::Channel<mu::actions::ActionCodeList> HelpMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

std::map<ActionCode, HelpMenuController::AvailableCallback> HelpMenuController::actions() const
{
    static std::map<ActionCode, AvailableCallback> _actions = {
        { "show-tours", std::bind(&HelpMenuController::isShowToursAvailable, this) },
        { "reset-tours", std::bind(&HelpMenuController::isResetToursAvailable, this) },
        { "online-handbook", std::bind(&HelpMenuController::isOnlineHandbookAvailable, this) },
        { "about", std::bind(&HelpMenuController::isAboutAvailable, this) },
        { "about-qt", std::bind(&HelpMenuController::isAboutQtAvailable, this) },
        { "about-musicxml", std::bind(&HelpMenuController::isAboutMusicXMLAVailable, this) },
        { "check-update", std::bind(&HelpMenuController::isCheckUpdateAvailable, this) },
        { "ask-help", std::bind(&HelpMenuController::isAskForHelpAvailable, this) },
        { "report-bug", std::bind(&HelpMenuController::isBugReportAvailable, this) },
        { "leave-feedback", std::bind(&HelpMenuController::isLeaveFeedbackAvailable, this) },
        { "revert-factory", std::bind(&HelpMenuController::isRevertToFactorySettingsAvailable, this) }
    };

    return _actions;
}

bool HelpMenuController::isShowToursAvailable() const
{
    return controller()->actionAvailable("show-tours");
}

bool HelpMenuController::isResetToursAvailable() const
{
    return controller()->actionAvailable("reset-tours");
}

bool HelpMenuController::isOnlineHandbookAvailable() const
{
    return controller()->actionAvailable("online-handbook");
}

bool HelpMenuController::isAboutAvailable() const
{
    return controller()->actionAvailable("about");
}

bool HelpMenuController::isAboutQtAvailable() const
{
    return controller()->actionAvailable("about-qt");
}

bool HelpMenuController::isAboutMusicXMLAVailable() const
{
    return controller()->actionAvailable("about-musicxml");
}

bool HelpMenuController::isCheckUpdateAvailable() const
{
    return controller()->actionAvailable("check-update");
}

bool HelpMenuController::isAskForHelpAvailable() const
{
    return controller()->actionAvailable("ask-help");
}

bool HelpMenuController::isBugReportAvailable() const
{
    return controller()->actionAvailable("report-bug");
}

bool HelpMenuController::isLeaveFeedbackAvailable() const
{
    return controller()->actionAvailable("leave-feedback");
}

bool HelpMenuController::isRevertToFactorySettingsAvailable() const
{
    return controller()->actionAvailable("revert-factory");
}
